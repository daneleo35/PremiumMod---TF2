"""Run SQL emitted by compiled production SourcePawn against real SQLite.

Only Database.Format / Transaction.AddQuery are fake host boundaries. Purchase,
debit, receipt, reward and extension SQL comes from AddEconomyQueries itself.
MySQL query shape is checked, but no live MySQL or TF2 server is simulated.
"""
from concurrent.futures import ThreadPoolExecutor
from contextlib import closing
from pathlib import Path
import re
import sqlite3
import tempfile
import threading
import unittest

from run_policy import TestHost, UnsignedFormatter, atcprintf, native, SourcePawnPlugin
from smx.sourcemod.natives.base import MethodMap

ROOT = Path(__file__).resolve().parents[1]
CAPTURE = []

class SQLDatabase(MethodMap):
    @native('cell', 'writable_string', 'string', '...')
    def Format(self, handle, output, fmt, *arguments):
        value = atcprintf(self.amx, fmt, arguments, formatter=UnsignedFormatter())
        if len(value.encode()) >= output.max_length:
            raise AssertionError('SQL buffer truncated')
        return output.write(value, null_terminate=True)

class SQLTransaction(MethodMap):
    @native('cell', 'string')
    def AddQuery(self, handle, query):
        CAPTURE.append(query)

class SQLHost(TestHost):
    TestDatabase = SQLDatabase()
    TestTransaction = SQLTransaction()

def queries(kind, *, seconds=0, interval=600, reward=5, cap=1000000, price=10,
            duration=3600, renew=1, limit=1, days=365, now=100000, nonce=1,
            player=85, feature=8, mysql=0):
    if kind != 1: feature = -1
    CAPTURE.clear()
    with (ROOT/'tests/economy_queries.smx').open('rb') as stream:
        plugin = SourcePawnPlugin(stream, smsys_options={'natives_cls': SQLHost})
    plugin.runtime.call_function_by_name('Emit', kind, seconds, interval, reward, cap,
        price, duration, renew, limit, days, now, nonce, player, feature, mysql)
    return list(CAPTURE)

def execute(db, statements):
    db.execute('BEGIN IMMEDIATE')
    try:
        result = None
        for statement in statements:
            cursor = db.execute(statement)
            if statement.startswith('SELECT points,'):
                result = cursor.fetchone()
        db.execute('COMMIT')
        return result
    except Exception:
        db.execute('ROLLBACK')
        raise

class EconomyIntegration(unittest.TestCase):
    def setUp(self):
        scratch = ROOT/'tests/tmp'; scratch.mkdir(exist_ok=True)
        self.directory = tempfile.TemporaryDirectory(prefix='points-', dir=scratch)
        self.path = Path(self.directory.name)/'wallet.sqlite'
        self.db = sqlite3.connect(self.path, isolation_level=None)
        source = (ROOT/'scripting/premium/economy_state.inc').read_text()
        for key in ('SQL_ECON_WALLET', 'SQL_ECON_RENTALS', 'SQL_ECON_RECEIPTS'):
            self.db.execute(re.search(rf'#define {key} "([^"\n]+)"', source).group(1))
        self.db.execute("INSERT INTO premium_v3_wallets (steamid, points) VALUES ('[U:1:85]', 100)")

    def tearDown(self):
        self.db.close(); self.directory.cleanup()

    def test_reward_requires_complete_interval_and_persists_remainder(self):
        first = execute(self.db, queries(0, seconds=599))
        self.assertEqual((100, 599), first[:2])
        second = execute(self.db, queries(0, seconds=3, nonce=2))
        self.assertEqual((105, 2), second[:2])
        self.assertEqual(5, second[6])

    def test_earn_retry_does_not_duplicate_points_or_time(self):
        intent = queries(0, seconds=1205)
        first = execute(self.db, intent); second = execute(self.db, intent)
        self.assertEqual((110, 5), first[:2]); self.assertEqual(first, second)
        self.assertEqual(1, self.db.execute('SELECT COUNT(*) FROM premium_v3_receipts').fetchone()[0])

    def test_reward_cap_and_lowered_cap_do_not_confiscate(self):
        self.assertEqual(103, execute(self.db, queries(0, seconds=600, cap=103))[0])
        result = execute(self.db, queries(0, seconds=600, cap=50, nonce=2))
        self.assertEqual(103, result[0]); self.assertEqual(0, result[6])

    def test_rental_debit_and_expiry_commit_together(self):
        result = execute(self.db, queries(1))
        self.assertEqual(90, result[0]); self.assertEqual(103600, result[5]); self.assertEqual((-10, 1), result[6:8])

    def test_renewal_extends_original_deadline_once(self):
        first = queries(1); execute(self.db, first)
        renewal = queries(1, nonce=2, now=100100)
        self.assertEqual(107200, execute(self.db, renewal)[5])
        self.assertEqual((80, 107200), (execute(self.db, first)[0], execute(self.db, renewal)[5]))

    def test_declined_purchase_never_debits_and_cannot_replay_later(self):
        intent = queries(1, price=200)
        result = execute(self.db, intent)
        self.assertEqual((100, 0, 0), (result[0], result[5], result[7]))
        execute(self.db, queries(3, seconds=500, nonce=2))
        replay = execute(self.db, intent)
        self.assertEqual((600, 0, 0), (replay[0], replay[5], replay[7]))

    def test_disabled_renewal_and_duration_cap_decline_without_charge(self):
        execute(self.db, queries(1))
        declined = execute(self.db, queries(1, nonce=2, renew=0))
        self.assertEqual((90, 0), (declined[0], declined[7]))
        too_long = execute(self.db, queries(1, nonce=3, duration=86400, days=1))
        self.assertEqual((90, 0), (too_long[0], too_long[7]))

    def test_trial_limit_and_active_trial_cannot_be_bypassed(self):
        result = execute(self.db, queries(2, limit=2))
        self.assertEqual((90, 103600, 1), (result[0], result[2], result[3]))
        self.assertEqual(0, execute(self.db, queries(2, limit=2, nonce=2, now=100010))[7])
        result = execute(self.db, queries(2, limit=2, nonce=3, now=104000))
        self.assertEqual((80, 2), (result[0], result[3]))
        result = execute(self.db, queries(2, limit=2, nonce=4, now=108000))
        self.assertEqual((80, 2, 0), (result[0], result[3], result[7]))

    def test_unlimited_trial_setting_and_receipt_retry(self):
        intent = queries(2, limit=0)
        execute(self.db, intent); execute(self.db, intent)
        result = execute(self.db, queries(2, limit=0, nonce=2, now=104000))
        self.assertEqual((80, 2), (result[0], result[3]))

    def test_two_purchases_cannot_overdraw_shared_wallet(self):
        intents = [queries(1, price=75, nonce=1, feature=8), queries(1, price=75, nonce=2, feature=25)]
        barrier = threading.Barrier(2)
        def buy(intent):
            with closing(sqlite3.connect(self.path, isolation_level=None, timeout=10)) as db:
                barrier.wait(timeout=5); return execute(db, intent)[7]
        with ThreadPoolExecutor(max_workers=2) as pool:
            self.assertEqual([0, 1], sorted(pool.map(buy, intents)))
        self.assertEqual(25, self.db.execute('SELECT points FROM premium_v3_wallets').fetchone()[0])

    def test_two_trial_requests_share_one_lifetime_counter(self):
        intents = [queries(2, limit=1, nonce=i) for i in (1, 2)]
        barrier = threading.Barrier(2)
        def buy(intent):
            with closing(sqlite3.connect(self.path, isolation_level=None, timeout=10)) as db:
                barrier.wait(timeout=5); return execute(db, intent)[7]
        with ThreadPoolExecutor(max_workers=2) as pool:
            self.assertEqual([0, 1], sorted(pool.map(buy, intents)))
        self.assertEqual((90, 1), self.db.execute('SELECT points, trial_purchases FROM premium_v3_wallets').fetchone())

    def test_failed_transaction_rolls_back_debit_rental_and_receipt(self):
        intent = queries(1); intent.insert(-1, 'INSERT INTO missing_table VALUES (1)')
        with self.assertRaises(sqlite3.OperationalError): execute(self.db, intent)
        self.assertEqual(100, self.db.execute('SELECT points FROM premium_v3_wallets').fetchone()[0])
        self.assertEqual(0, self.db.execute('SELECT COUNT(*) FROM premium_v3_rentals').fetchone()[0])
        self.assertEqual(0, self.db.execute('SELECT COUNT(*) FROM premium_v3_receipts').fetchone()[0])

    def test_wallet_and_rentals_survive_connection_restart(self):
        execute(self.db, queries(1)); self.db.close(); self.db = sqlite3.connect(self.path, isolation_level=None)
        self.assertEqual(90, self.db.execute('SELECT points FROM premium_v3_wallets').fetchone()[0])
        self.assertEqual(103600, self.db.execute('SELECT expires FROM premium_v3_rentals').fetchone()[0])

    def test_other_player_wallet_untouched(self):
        execute(self.db, queries(0, seconds=600, player=86))
        self.assertEqual(100, self.db.execute("SELECT points FROM premium_v3_wallets WHERE steamid='[U:1:85]'").fetchone()[0])
        self.assertEqual(5, self.db.execute("SELECT points FROM premium_v3_wallets WHERE steamid='[U:1:86]'").fetchone()[0])

    def test_signed_admin_adjustment_is_clamped_and_idempotent(self):
        intent = queries(3, seconds=-250)
        self.assertEqual(0, execute(self.db, intent)[0]); self.assertEqual(0, execute(self.db, intent)[0])
        self.assertEqual(40, execute(self.db, queries(3, seconds=100, cap=40, nonce=2))[0])

    def test_mysql_builder_uses_innodb_compatible_dialect(self):
        sql = '\n'.join(queries(0, seconds=600, mysql=1))
        self.assertIn('INSERT IGNORE', sql); self.assertIn(' DIV ', sql)
        self.assertNotIn('OR IGNORE', sql); self.assertNotIn(' AS INTEGER', sql)

    def test_trial_timestamp_overflow_declines_without_debit(self):
        result = execute(self.db, queries(2, now=2147483600, duration=3600))
        self.assertEqual((100, 0), (result[0], result[7]))
