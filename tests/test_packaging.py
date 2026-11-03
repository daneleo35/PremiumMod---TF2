"""Exercise the real release packager against a workspace containing junk."""
from pathlib import Path
import importlib.util
import tempfile
import unittest
from unittest.mock import patch
from zipfile import ZipFile

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('release_build', ROOT/'build.py')
build = importlib.util.module_from_spec(spec)
spec.loader.exec_module(build)


class ReleasePackaging(unittest.TestCase):
    def test_server_and_source_archives_have_separate_contents(self):
        scratch = ROOT/'tests/tmp'
        scratch.mkdir(exist_ok=True)
        with tempfile.TemporaryDirectory(prefix='packaging-', dir=scratch) as location:
            root = Path(location).resolve()
            self.assertTrue(root.is_relative_to(scratch.resolve()))
            files = [
                'build.py', '.gitignore', 'LICENSE.txt', 'THIRD-PARTY.txt',
                'tools/bootstrap.py', 'tools/config_layout.py', 'tools/prepare_ui.py',
                'scripting/premium.sp', 'scripting/premium/example.inc',
                'configs/premium_rebuilt.cfg', 'configs/premium_rules.cfg',
                'configs/premium_features.cfg', 'configs/premium_shop.cfg',
                'configs/premium_profiles/fun.cfg', 'configs/premium-databases.example.txt',
                'translations/premium_rebuilt.phrases.txt', 'tests/example_tests.sp',
                'tests/test_example.py', 'dist/premium.smx',
                # Files that must never leak into either archive.
                'README.md', 'docs/wiki/Home.md', 'build-report.json',
                'tools/sourcemod/spcomp.exe', 'tools/python-libs/cached.py',
                'scripting/include/morecolors.inc', 'scripting/include/tf2attributes.inc',
                'scripting/old.smx', 'tests/compiled.smx', 'tests/tmp/private.txt',
                'tests/__pycache__/stale.py', 'configs/premium_rebuilt.cfg.menu-bak',
                'data/premium_points_outbox.cfg',
                'configs/private-database.cfg',
            ]
            for name in files:
                p = root/name
                p.parent.mkdir(parents=True, exist_ok=True)
                p.write_text(name, encoding='utf-8')
            with patch.object(build, 'ROOT', root):
                runtime, source = build.package_file_sets(root/'dist/premium.smx')
                server_zip, source_zip = build.write_archives(root/'dist', runtime, source)
            with ZipFile(server_zip) as z:
                self.assertIsNone(z.testzip())
                self.assertEqual(set(z.namelist()), {
                    'addons/sourcemod/plugins/premium.smx',
                    'addons/sourcemod/translations/premium_rebuilt.phrases.txt',
                    'addons/sourcemod/configs/premium_rules.cfg',
                    'addons/sourcemod/configs/premium_features.cfg',
                    'addons/sourcemod/configs/premium_shop.cfg',
                    'cfg/sourcemod/premium_rebuilt.cfg',
                    'cfg/sourcemod/premium_profiles/fun.cfg',
                })
                self.assertEqual(z.read('addons/sourcemod/plugins/premium.smx'), b'dist/premium.smx')
            with ZipFile(source_zip) as z:
                self.assertIsNone(z.testzip())
                self.assertEqual(set(z.namelist()), set(files[:18]))
                self.assertFalse(any(p.startswith(('docs/', 'development/', 'addons/')) for p in z.namelist()))
                self.assertFalse(any(p.endswith(('.smx', '.exe', '.md')) for p in z.namelist()))
                for required in ('build.py', 'tools/bootstrap.py', 'tools/prepare_ui.py',
                                 'tools/config_layout.py', 'scripting/premium.sp'):
                    self.assertIn(required, z.namelist())


if __name__ == '__main__':
    unittest.main()
