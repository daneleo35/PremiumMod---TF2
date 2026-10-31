"""Integration tests use the schema and query text extracted from production SP.

SQLite is real, on disk, including concurrent connections and rollback. The
SourceMod Database.Format boundary is converted to Python's bound parameters.
This does not emulate MySQL or TF2 callbacks.
"""
from concurrent.futures import ThreadPoolExecutor
from contextlib import closing
from pathlib import Path
import re
import sqlite3
import tempfile
import threading
import unittest

ROOT = Path(__file__).resolve().parents[1]
SOURCE = (ROOT / 'scripting/premium/storage.inc').read_text(encoding='utf-8')


def query(prefix):
    matches = re.findall(r'"([^"\n]+)"', SOURCE)
    value = next(text for text in matches if text.startswith(prefix))
    return value.replace("'%s'", '?').replace('%d', '?').replace('%s', 'OR IGNORE')


CREATE_MEMBERS = query('CREATE TABLE IF NOT EXISTS premium_v3_members')
CREATE_TRIALS = query('CREATE TABLE IF NOT EXISTS premium_v3_trials')
INSERT_TRIAL = query('INSERT %s INTO premium_v3_trials')
INSERT_MEMBER = query('INSERT %s INTO premium_v3_members')
DELETE_MEMBER = query('DELETE FROM premium_v3_members')
REPLACE_TRIAL = query('REPLACE INTO premium_v3_trials')
START_TRIAL = query('UPDATE premium_v3_trials SET used')
LOAD = query('SELECT used, trial_until')
LEGACY = query('SELECT steamid FROM premium ORDER BY')
A, B = '[U:1:8307981]', '[U:1:123456789]'


class StorageIntegration(unittest.TestCase):
    def setUp(self):
        scratch = ROOT / 'tests/tmp'
        scratch.mkdir(exist_ok=True)
        self.directory = tempfile.TemporaryDirectory(prefix='premium-sql-', dir=scratch)
        assert Path(self.directory.name).resolve().is_relative_to(scratch.resolve())
        self.path = Path(self.directory.name) / 'premium.sqlite'
        self.db = sqlite3.connect(self.path, isolation_level=None)
        self.db.execute(CREATE_MEMBERS)
        self.db.execute(CREATE_TRIALS)

    def tearDown(self):
        self.db.close()
        self.directory.cleanup()

    def test_schema_is_idempotent(self):
        self.db.execute(CREATE_MEMBERS)
        self.db.execute(CREATE_TRIALS)

    def test_new_record_defaults_to_unused(self):
        self.db.execute(INSERT_TRIAL, (A,))
        self.assertEqual((0, 0, 0), self.db.execute(LOAD, (A, A)).fetchone())

    def test_loading_does_not_reset_trial(self):
        self.db.execute(REPLACE_TRIAL, (A, 1, 1700000900))
        self.db.execute(INSERT_TRIAL, (A,))
        self.assertEqual((1, 1700000900, 0), self.db.execute(LOAD, (A, A)).fetchone())

    def test_only_one_atomic_trial_start(self):
        self.db.execute(INSERT_TRIAL, (A,))
        first = self.db.execute(START_TRIAL, (1700000900, A)).rowcount
        second = self.db.execute(START_TRIAL, (1700001800, A)).rowcount
        self.assertEqual((1, 0), (first, second))
        self.assertEqual(1700000900, self.db.execute(LOAD, (A, A)).fetchone()[1])

    def test_two_servers_cannot_both_start_a_trial(self):
        self.db.execute(INSERT_TRIAL, (A,))
        barrier = threading.Barrier(2)
        def claim(until):
            with closing(sqlite3.connect(self.path, timeout=10, isolation_level=None)) as db:
                barrier.wait(timeout=5)
                return db.execute(START_TRIAL, (until, A)).rowcount
        with ThreadPoolExecutor(max_workers=2) as pool:
            self.assertEqual([0, 1], sorted(pool.map(claim, [1700000900, 1700001800])))

    def test_disconnect_restart_keeps_the_deadline(self):
        self.db.execute(REPLACE_TRIAL, (A, 1, 1700000900))
        self.db.close()
        self.db = sqlite3.connect(self.path, isolation_level=None)
        self.assertEqual((1, 1700000900, 0), self.db.execute(LOAD, (A, A)).fetchone())

    def test_players_do_not_share_countdown_state(self):
        self.db.execute(REPLACE_TRIAL, (A, 1, 1700000900))
        self.db.execute(REPLACE_TRIAL, (B, 1, 1700001800))
        self.assertEqual(1700000900, self.db.execute(LOAD, (A, A)).fetchone()[1])
        self.assertEqual(1700001800, self.db.execute(LOAD, (B, B)).fetchone()[1])

    def test_reset_revokes_trial_and_restores_eligibility(self):
        self.db.execute(REPLACE_TRIAL, (A, 1, 1700000900))
        self.db.execute(REPLACE_TRIAL, (A, 0, 0))
        self.assertEqual((0, 0, 0), self.db.execute(LOAD, (A, A)).fetchone())
        self.assertEqual(1, self.db.execute(START_TRIAL, (1700001900, A)).rowcount)

    def test_grant_uses_a_finite_persistent_deadline(self):
        self.db.execute(REPLACE_TRIAL, (A, 1, 1700000900))
        self.db.execute(REPLACE_TRIAL, (A, 1, 1700001900))
        self.assertEqual((1, 1700001900, 0), self.db.execute(LOAD, (A, A)).fetchone())

    def test_duplicate_membership_is_idempotent(self):
        self.db.execute(INSERT_MEMBER, (A,))
        self.db.execute(INSERT_MEMBER, (A,))
        self.assertEqual(1, self.db.execute('SELECT COUNT(*) FROM premium_v3_members').fetchone()[0])

    def test_remove_changes_only_the_selected_identity(self):
        for steam in [A, B]:
            self.db.execute(INSERT_MEMBER, (steam,))
            self.db.execute(INSERT_TRIAL, (steam,))
        self.assertEqual(1, self.db.execute(DELETE_MEMBER, (A,)).rowcount)
        self.assertEqual(0, self.db.execute(LOAD, (A, A)).fetchone()[2])
        self.assertEqual(1, self.db.execute(LOAD, (B, B)).fetchone()[2])

    def test_failed_import_transaction_rolls_back(self):
        self.db.execute('BEGIN')
        try:
            self.db.execute(INSERT_MEMBER, (A,))
            self.db.execute('INSERT INTO missing_table VALUES (1)')
        except sqlite3.OperationalError:
            self.db.execute('ROLLBACK')
        self.assertEqual(0, self.db.execute('SELECT COUNT(*) FROM premium_v3_members').fetchone()[0])

    def test_legacy_batches_are_ordered_and_bounded(self):
        self.db.execute('CREATE TABLE premium (steamid TEXT)')
        self.db.executemany('INSERT INTO premium VALUES (?)', [(f'[U:1:{i:04d}]',) for i in reversed(range(220))])
        batch = self.db.execute(LEGACY, (100,)).fetchall()
        self.assertEqual(100, len(batch))
        self.assertEqual('[U:1:0100]', batch[0][0])
        self.assertEqual('[U:1:0199]', batch[-1][0])

    def test_database_failure_does_not_create_partial_schema_rows(self):
        with self.assertRaises(sqlite3.OperationalError):
            self.db.execute('SELECT * FROM premium')
        self.assertEqual(0, self.db.execute('SELECT COUNT(*) FROM premium_v3_trials').fetchone()[0])


if __name__ == '__main__':
    unittest.main(verbosity=2)
