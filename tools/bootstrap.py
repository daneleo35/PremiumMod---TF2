"""Fetch pinned official compiler/dependency files into this project only."""
from pathlib import Path
import argparse
import hashlib
import subprocess
import sys
import urllib.request
import zipfile

ROOT = Path(__file__).resolve().parents[1]
ASSETS = [
    ('https://sm.alliedmods.net/smdrop/1.12/sourcemod-1.12.0-git7259-windows.zip',
     'tools/sourcemod-1.12.0-git7259-windows.zip', '5711ce719fa2f3fa4a762702f55c7906aa951690c739499beb5f2b62a9161362'),
    ('https://github.com/FlaminSarge/tf2attributes/releases/download/v1.7.5.1/tf2attributes.inc',
     'scripting/include/tf2attributes.inc', '3ea2b74d6ac501c59e95303e3b7fcba5700a6ef6a2bf015407a8b081d8b79639'),
    ('https://raw.githubusercontent.com/DoctorMcKay/sourcemod-plugins/5fd5e7ae77cd4f53e0805d0792390545d6037f30/scripting/include/morecolors.inc',
     'scripting/include/morecolors.inc', '956eb28a23043ba8d72c6e0fb41c101c7a913a47aa3beb62c717de1933455b57'),
]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--with-tests', action='store_true')
    args = parser.parse_args()
    for url, relative, checksum in ASSETS:
        path = ROOT / relative
        path.parent.mkdir(parents=True, exist_ok=True)
        if path.exists() and hashlib.sha256(path.read_bytes()).hexdigest() == checksum:
            continue
        with urllib.request.urlopen(url, timeout=120) as response:
            payload = response.read()
        if hashlib.sha256(payload).hexdigest() != checksum:
            raise SystemExit(f'Checksum mismatch: {url}')
        path.write_bytes(payload)
        print(f'Downloaded and verified {relative}')
    destination = ROOT/'tools/sourcemod'
    with zipfile.ZipFile(ROOT/ASSETS[0][1]) as archive:
        for name in archive.namelist():
            if not name.startswith('addons/sourcemod/scripting/'):
                continue
            if not (destination/name).resolve().is_relative_to(destination.resolve()):
                raise SystemExit('Unsafe archive path')
            archive.extract(name, destination)
    if args.with_tests:
        subprocess.run([sys.executable, '-m', 'pip', 'install', '--target', str(ROOT/'tools/python-libs'), 'pysmx==0.4.0'], check=True)
    print('Ready. Run: python build.py --test' if args.with_tests else 'Ready. Run: python build.py')


if __name__ == '__main__':
    main()
