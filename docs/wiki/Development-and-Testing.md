# Building and testing

This repository contains the rewrite source, curated configs, translations, tests and build tooling. Third-party headers/compiler files and game-server dependencies are downloaded separately. No custom TF2 engine DLL/SO is distributed.

## Build on Windows

Install Git and Python 3.10 or later, then:

```powershell
git clone https://github.com/daneleo35/PremiumMod---TF2.git
cd PremiumMod---TF2
python tools/bootstrap.py
python build.py
```

The bootstrap downloads pinned SourceMod 1.12.0.7259 Windows tooling plus TF2Attributes/More Colors headers and checks their hashes. The builder emits `dist/premium.smx`, an install ZIP, a source ZIP and `dist/build-report.json`. Inspect the report and install using [Installation](Installation.md). Runtime TF2Attributes and gamedata still need separate installation on the server.

The packaged `premium.smx` targets SourceMod and can run on supported Windows/Linux TF2 servers; the automatic Python build currently invokes a Windows compiler executable. For a manual Linux build, obtain a compatible Linux SourceMod compiler/includes and both dependency headers, then use its `spcomp64` with the `scripting/include` and SourceMod include paths. Do not execute the Windows bootstrap/build assuming native Linux tooling.

## Automated checks

```powershell
python tools/bootstrap.py --with-tests
python build.py --test
```

The pinned local test dependency is `pysmx==0.4.0`. The full suite compiles test harnesses against shared production policy code, runs the SourcePawn VM and Python artifact/storage/economy checks, then packages. It can take several minutes. Release compilation uses SourcePawn 1.12; the test VM's compatible compiler is SourcePawn 1.11.0.6931.

The RC13 validation baseline is **2,943 SourcePawn assertions and 60 Python checks**, including 17 real SQLite economy integration tests. The SQLite suite executes queries produced by the production SourcePawn query builder and checks transaction/replay behaviour. These counts describe that validation run, not proof that every engine behaviour works.

**Not exercised by that suite:** a running TF2 server on either OS, real MySQL, native SourceMod disk failure/replacement handling or callback interruption/recovery. The plugin remains a release candidate requiring server acceptance testing.

## Server acceptance checklist

Use a private test server and at least one ordinary non-Premium account. Back up first. Check these paths before public deployment:

1. Clean load, dependency/translation errors, reload and map changes; compare ordinary, member, free-trial and individual-grant access.
2. Normal all/team chat tags, hidden/style preferences, quiet mode and expiry/reminders.
3. Reserve ammo/fuel on several weapons; consume the full ball/jar reserve and verify one-at-a-time regeneration over repeated cycles. Test resupply, death, class changes and access loss.
4. Pyro primary flight on stock, Backburner and Dragon's Fury; airblast on supported weapons; fuel exhaustion and map bans.
5. Engineer mixed six-building layouts, team cap, primary versus extra sentry levels, friendly upgrades, hauling and teleporter pairing. Lower caps and remove access while buildings exist.
6. Finite ordinary/Dead Ringer cloak timing, native activation/recharge, health rewards, respawn restrictions, team swap and safe-location rejection.
7. Economy earning eligibility, confirmations, insufficient funds, renewal, bought-trial lifetime limits, reconnect/offline expiry and separate switches.
8. In a disposable backed-up environment, exercise restart/replay and DB outage recovery; test real MySQL independently if deployed.

Detailed acceptance plans: [base](https://github.com/daneleo35/PremiumMod---TF2/blob/main/docs/SERVER_TEST.md), [RC10 building expansion](https://github.com/daneleo35/PremiumMod---TF2/blob/main/docs/RC10_SERVER_TEST.md), [RC11 native upgrade protection](https://github.com/daneleo35/PremiumMod---TF2/blob/main/docs/RC11_SERVER_TEST.md), [RC12 access grants](https://github.com/daneleo35/PremiumMod---TF2/blob/main/docs/RC12_SERVER_TEST.md), [RC13 economy](https://github.com/daneleo35/PremiumMod---TF2/blob/main/docs/RC13_SERVER_TEST.md). Historical notes may describe intermediate implementations; current Wiki pages and source define current behaviour.

## Source layout

| Path | Purpose |
| --- | --- |
| `scripting/premium.sp` | Entry point, lifecycle and event hooks |
| `scripting/premium/*.inc` | Feature, access, storage and gameplay implementation |
| `scripting/include/` | Project headers; external headers are ignored |
| `configs/` | Main and companion configs, profiles, database example |
| `translations/` | SourceMod phrase file |
| `tests/` | SourcePawn harnesses and Python integration/artifact checks |
| `tools/config_layout.py` | Main config organization; values come from SourcePawn |
| `docs/wiki/` | Repository copy of this Wiki |

Keep third-party licensing/attribution in [THIRD-PARTY.txt](https://github.com/daneleo35/PremiumMod---TF2/blob/main/THIRD-PARTY.txt) and the project [license](https://github.com/daneleo35/PremiumMod---TF2/blob/main/LICENSE.txt). Source-only Git history should not contain downloaded dependencies, server credentials or live player databases/outboxes.

[Troubleshooting](Troubleshooting.md) · [Home](Home.md)
