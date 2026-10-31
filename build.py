"""Build and package Premium Mod using a pinned SourceMod toolchain."""
from pathlib import Path
import argparse
import hashlib
import json
import re
import shutil
import subprocess
import sys
import zipfile

ROOT = Path(__file__).resolve().parent
VERSION = '3.0.0-rc19'
EXTERNAL_HEADERS = {'morecolors.inc', 'tf2attributes.inc'}


def check_build_dependencies(root, compiler_root):
    downloads = {
        'morecolors.inc': 'More Colors: https://github.com/DoctorMcKay/sourcemod-plugins/blob/master/scripting/include/morecolors.inc',
        'tf2attributes.inc': 'TF2Attributes: https://github.com/FlaminSarge/tf2attributes/releases',
    }
    missing = [name for name in downloads if not (root/'scripting/include'/name).is_file()
               and not (compiler_root/'include'/name).is_file()]
    if missing:
        detail = '\n'.join(f'  Missing {name}. {downloads[name]}' for name in missing)
        raise SystemExit('Premium build dependency warning:\n' + detail
                         + '\nPlace these headers in scripting/include, or run: python tools/bootstrap.py'
                         + '\nMore Colors is a compile-time include, not a runtime plugin.')


def compile_source(compiler, source, output, includes):
    command = [str(compiler), str(source), f'-o{output}', '-E']
    command += [f'-i{include}' for include in includes]
    command += [f'-i{source.parent}']
    result = subprocess.run(command, cwd=ROOT, text=True, capture_output=True)
    print(result.stdout, end='')
    if result.stderr: print(result.stderr, file=sys.stderr, end='')
    result.check_returncode()
    return result.stdout


def create_config():
    from tools.config_layout import SECTIONS

    source = (ROOT/'scripting/premium/config.inc').read_text()
    entries = re.findall(r'(?:Setting|CreateConVar)\("(premium_[^\"]+)",\s*"([^\"]*)",\s*"([^\"]*)"', source)
    defaults = {name: value for name, value, _ in entries if name != 'premium_clip_multiplier'}
    for name in ('scout', 'sniper', 'soldier', 'demoman', 'medic', 'heavy', 'pyro', 'spy', 'engineer'):
        defaults[f'premium_speedmult_{name}'] = '1.0'
    ordered = [name for _, _, settings in SECTIONS for name, _ in settings]
    if len(ordered) != len(set(ordered)) or set(ordered) != set(defaults):
        raise ValueError(f'Config layout mismatch: missing={sorted(set(defaults) - set(ordered))}; '
                         f'unknown={sorted(set(ordered) - set(defaults))}; duplicate={len(ordered) != len(set(ordered))}')
    lines = [f'// PREMIUM MOD {VERSION} - SERVER SETTINGS',
             '// Switches: 0 = OFF, 1 = ON, unless a line says otherwise.',
             '// Edit quoted values. Keep setting names and quotes intact.',
             '// Units are labelled: seconds, minutes, days, percent, or multiplier.',
             '//',
             '// Companion files (addons/sourcemod/configs/):',
             '//   premium_features.cfg  - public/free and individual Steam-ID grants',
             '//   premium_shop.cfg      - points prices, durations and offers',
             '//   premium_rules.cfg     - class overrides and map restrictions',
             '// Keep your custom values when updating; do not blindly overwrite this file.', '']
    for title, notes, settings in SECTIONS:
        lines.extend(['// ' + '=' * 74, f'// {title}', '// ' + '=' * 74])
        lines.extend('// ' + note for note in notes)
        lines.append('')
        for name, description in settings:
            assignment = f'{name:<39} "{defaults[name]}"'
            lines.append(f'{assignment:<56} // {description}')
        lines.append('')
    lines.extend(['// ' + '=' * 74, '// OPTIONAL BALANCE PRESET', '// ' + '=' * 74,
                  '// Uncomment ONE line to override the matching values above.',
                  '// Keep the exec at the END: a preset runs last and wins.',
                  '// exec sourcemod/premium_profiles/cosmetic.cfg',
                  '// exec sourcemod/premium_profiles/balanced.cfg',
                  '// exec sourcemod/premium_profiles/fun.cfg', ''])
    return '\n'.join(lines)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--test', action='store_true', help='Run policy VM, SQLite integration, and artifact checks')
    arguments = parser.parse_args()
    from tools.prepare_ui import generate
    generate()
    compiler_root = ROOT/'tools/sourcemod/addons/sourcemod/scripting'
    compiler = compiler_root/'spcomp64.exe'
    if not compiler.exists():
        raise SystemExit('Missing toolchain. Run: python tools/bootstrap.py')
    check_build_dependencies(ROOT, compiler_root)
    dist = ROOT/'dist'
    dist.mkdir(exist_ok=True)
    output = dist/'premium.smx'
    compiler_output = compile_source(compiler, ROOT/'scripting/premium.sp', output,
                                      [compiler_root/'include', ROOT/'scripting/include'])
    test_report = {'executed': False, 'note': 'No TF2 server or MySQL server has been exercised.'}
    if arguments.test:
        libs = ROOT/'tools/python-libs'
        test_compiler = libs/'smx/spcomp/spcomp.exe'
        if not test_compiler.exists():
            raise SystemExit('Install test tools locally: python -m pip install --target tools/python-libs pysmx==0.4.0')
        # pysmx supports SourcePawn 1.11 bytecode. These tests compile the exact
        # shared core.inc with its bundled compiler; release compilation is 1.12.
        compile_source(test_compiler, ROOT/'tests/core_tests.sp', ROOT/'tests/core_tests.smx',
                       [libs/'smx/include'])
        compile_source(test_compiler, ROOT/'tests/dependencies_tests.sp', ROOT/'tests/dependencies_tests.smx',
                       [libs/'smx/include'])
        compile_source(test_compiler, ROOT/'tests/glow_tests.sp', ROOT/'tests/glow_tests.smx',
                       [libs/'smx/include'])
        compile_source(test_compiler, ROOT/'tests/attributes_tests.sp', ROOT/'tests/attributes_tests.smx',
                       [libs/'smx/include'])
        compile_source(test_compiler, ROOT/'tests/buildings_tests.sp', ROOT/'tests/buildings_tests.smx',
                       [libs/'smx/include'])
        compile_source(test_compiler, ROOT/'tests/ammo_tests.sp', ROOT/'tests/ammo_tests.smx',
                       [libs/'smx/include'])
        for name in ('options', 'celebrations', 'airblast', 'chat', 'tooltip', 'features', 'economy', 'ui'):
            compile_source(test_compiler, ROOT/f'tests/{name}_tests.sp', ROOT/f'tests/{name}_tests.smx', [libs/'smx/include'])
        compile_source(test_compiler, ROOT/'tests/economy_queries.sp', ROOT/'tests/economy_queries.smx', [libs/'smx/include'])
        policy = subprocess.run([sys.executable, 'tests/run_policy.py'], cwd=ROOT, text=True, capture_output=True)
        print(policy.stdout + policy.stderr)
        policy.check_returncode()
        checks = {label: int(number) for number, label in re.findall(r'PASS: (\d+) ([^\r\n]+) assertions', policy.stdout)}
        python_tests = subprocess.run([sys.executable, '-m', 'unittest', 'discover', '-s', 'tests', '-p', 'test_*.py', '-v'],
                                      cwd=ROOT, text=True, capture_output=True)
        print(python_tests.stdout + python_tests.stderr)
        python_tests.check_returncode()
        test_report.update(executed=True, policy_assertions=checks['policy'], dependency_assertions=checks['dependency'],
                           glow_assertions=checks['glow ownership'], attribute_assertions=checks['attribute ownership'],
                           sentry_assertions=checks['sentry ownership'], ammo_assertions=checks['reserve ammo'],
                           options_assertions=checks['preferences and rules'], celebration_assertions=checks['celebration ownership'],
                           airblast_assertions=checks['airblast boost'], chat_assertions=checks['native chat tag'],
                           tooltip_assertions=checks['turret tooltip'], feature_assertions=checks['independent feature access'], economy_assertions=checks['points economy policy'], ui_assertions=checks['in-game UI safety'],
                           sqlite_tests=len(re.findall(r'\(test_storage\.', python_tests.stderr)),
                           artifact_tests=len(re.findall(r'\(test_artifacts\.', python_tests.stderr)),
                           economy_tests=len(re.findall(r'\(test_economy\.', python_tests.stderr)),
                           policy_vm='pysmx 0.4.0 / SourcePawn 1.11.0.6931', release_compiler='SourcePawn 1.12.0.7259')
    config = create_config()
    (ROOT/'configs/premium_rebuilt.cfg').write_text(config, encoding='utf-8')
    members = {
        'addons/sourcemod/plugins/premium.smx': output,
        'addons/sourcemod/translations/premium_rebuilt.phrases.txt': ROOT/'translations/premium_rebuilt.phrases.txt',
        'cfg/sourcemod/premium_rebuilt.cfg': ROOT/'configs/premium_rebuilt.cfg',
        'addons/sourcemod/configs/premium_rules.cfg': ROOT/'configs/premium_rules.cfg',
        'addons/sourcemod/configs/premium_features.cfg': ROOT/'configs/premium_features.cfg',
        'addons/sourcemod/configs/premium_shop.cfg': ROOT/'configs/premium_shop.cfg',
    }
    for source in (ROOT/'configs/premium_profiles').glob('*.cfg'):
        members['cfg/sourcemod/premium_profiles/' + source.name] = source
    for source in (ROOT/'scripting').rglob('*'):
        if source.is_file() and source.name not in EXTERNAL_HEADERS:
            members['addons/sourcemod/scripting/' + source.relative_to(ROOT/'scripting').as_posix()] = source
    for name in ('README.md', 'LICENSE.txt', 'THIRD-PARTY.txt'):
        members[name] = ROOT/name
    for source in (ROOT/'docs').rglob('*.md'): members['docs/' + source.relative_to(ROOT/'docs').as_posix()] = source
    members['configs/premium-databases.example.txt'] = ROOT/'configs/premium-databases.example.txt'
    # Include a self-contained source tree with the bootstrap/build/test scripts.
    development = ['build.py', '.gitignore', 'README.md', 'LICENSE.txt', 'THIRD-PARTY.txt', 'tools/bootstrap.py', 'tools/config_layout.py', 'tools/prepare_ui.py',
                   'configs/premium-databases.example.txt', 'configs/premium_rebuilt.cfg', 'configs/premium_rules.cfg', 'configs/premium_features.cfg', 'configs/premium_shop.cfg']
    for directory in ('scripting', 'translations', 'docs', 'tests'):
        development += [p.relative_to(ROOT).as_posix() for p in (ROOT/directory).rglob('*')
                        if p.is_file() and p.suffix in ('.sp', '.inc', '.py', '.txt', '.md')
                        and p.name not in EXTERNAL_HEADERS
                        and '__pycache__' not in p.parts and 'tmp' not in p.relative_to(ROOT).parts]
    development += [p.relative_to(ROOT).as_posix() for p in (ROOT/'configs/premium_profiles').glob('*.cfg')]
    for name in development: members['development/' + name] = ROOT/name
    hashes = {name: hashlib.sha256(path.read_bytes()).hexdigest() for name, path in members.items()}
    manifest = dict(version=VERSION, compiler='1.12.0.7259', tf2attributes='1.7.5.1', morecolors='1.9.1 / 5fd5e7ae77cd4f53e0805d0792390545d6037f30', tests=test_report,
                    dependency_packaging={'tf2attributes': 'Install separately; no dependency files bundled',
                                          'morecolors': 'Embedded in premium.smx; separate header excluded'},
                    compiler_output=compiler_output, files=hashes)
    (dist/'build-report.json').write_text(json.dumps(manifest, indent=2), encoding='utf-8')
    archive = dist/f'premium-rebuilt-{VERSION}.zip'
    with zipfile.ZipFile(archive, 'w', compression=zipfile.ZIP_DEFLATED) as package:
        for name, path in sorted(members.items()): package.write(path, name)
        package.write(dist/'build-report.json', 'build-report.json')
    print(f'Built {archive} ({archive.stat().st_size:,} bytes)')
    source_archive = dist/f'premium-rebuilt-{VERSION}-source.zip'
    with zipfile.ZipFile(source_archive, 'w', compression=zipfile.ZIP_DEFLATED) as package:
        for name in development: package.write(ROOT/name, name)
    print(f'Built source-only archive {source_archive} ({source_archive.stat().st_size:,} bytes)')


if __name__ == '__main__':
    main()
