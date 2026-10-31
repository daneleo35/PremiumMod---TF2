# Premium Mod for Team Fortress 2

A configurable SourceMod plugin for TF2 servers. Give players a configured Premium bundle, grant selected features free, or optionally let players earn points and rent individual features or buy limited Premium trials.

**Current version: 3.0.0-rc13 — release candidate.** The local build passed 2,943 SourcePawn assertions and 60 Python/SQLite checks. Live TF2, native filesystem/callback recovery and real MySQL acceptance are still required; these checks are not a promise of bug-free gameplay.

## Documentation

Start with the [documentation home](docs/wiki/Home.md). The same pages are prepared for the repository's [GitHub Wiki](https://github.com/daneleo35/PremiumMod---TF2/wiki).

| I want to… | Read |
| --- | --- |
| Install or update the plugin | [Installation](docs/wiki/Installation.md) |
| Understand membership versus individual features | [How it works](docs/wiki/How-It-Works.md) |
| Use the plugin as a player | [Player guide](docs/wiki/Player-Guide.md) |
| Find a command and example | [Command reference](docs/wiki/Commands.md) |
| Set up the four configuration files | [Configuration](docs/wiki/Configuration.md) |
| Look up every main setting | [All 97 settings](docs/wiki/Configuration-Reference.md) |
| Give free features or manage trials | [Access and trials](docs/wiki/Access-and-Trials.md) |
| Enable points, rental offers and purchased trials | [Points and shop](docs/wiki/Points-and-Shop.md) |
| Understand a bonus or building limit | [Features](docs/wiki/Features.md), [Engineer buildings](docs/wiki/Engineer-Buildings.md) |
| Restrict classes/maps or choose a preset | [Class/map rules](docs/wiki/Class-and-Map-Rules.md) |
| Back up or recover points/storage | [Storage and recovery](docs/wiki/Storage-and-Recovery.md) |
| Diagnose a problem | [Troubleshooting](docs/wiki/Troubleshooting.md) |
| Compile or contribute | [Development/testing](docs/wiki/Development-and-Testing.md) |

## What it includes

- Premium membership through SourceMod permissions or the membership database, plus optional free and points-bought trials.
- Separate public/Steam-ID grants for 29 feature keys; individual grants and rentals do not award Premium membership or player-chat tags.
- Reserve ammunition including Pyro fuel, spawn health/metal, class speed/gravity, finite cloak multipliers, throwable reserves and configurable combat rewards.
- Pyro primary-fire flight and a separate right-click airblast jump.
- Normal-PDA extra sentries, dispensers and teleporter ends, each configurable up to three, within **six total building slots per Engineer**. Extra sentries stay at level one.
- Automatic membership/trial chat tags, optional glow/colour/FOV, saved personal preferences, small building tooltips and optional round-win effects.
- Optional play-time points, private award notices, configurable rental prices/durations and lifetime limits on purchased Premium trials.
- SQLite by default, optional MySQL, asynchronous operations and durable receipt tokens for points settlement.

All access still obeys the configured enable settings, permissions, class/map/team rules, cooldowns and balance limits. Nothing requires modified TF2 game DLLs/SOs.

## Quick start: build and install

This repository contains **source and configuration**, not bundled third-party runtime plugins. The supplied automatic build workflow runs on **Windows with Python 3.10+**. The resulting SourcePawn plugin can be installed on a Linux or Windows TF2 server.

```powershell
git clone https://github.com/daneleo35/PremiumMod---TF2.git
cd PremiumMod---TF2
python tools/bootstrap.py
python build.py
```

The bootstrap downloads pinned compiler/headers into ignored local folders. It does not install server dependencies. The build creates an install ZIP and a separate source ZIP in `dist/`.

1. Install SourceMod with SDKTools, SDKHooks, Regex and Client Preferences available. The pinned release compiler is SourcePawn **1.12.0.7259**.
2. Install [TF2Attributes](https://github.com/FlaminSarge/tf2attributes/releases/tag/v1.7.5.1) separately, including its matching gamedata.
3. Extract the built install ZIP into the server's `tf/` directory. Disable the old Premium plugin first and preserve existing custom configuration/database files.
4. Review `tf/cfg/sourcemod/premium_rebuilt.cfg` and the three companion configs under `tf/addons/sourcemod/configs/`.
5. Restart/change map, check `sm plugins list` and the SourceMod error log, and test with both an ordinary account and a configured member.

[Full installation paths, dependencies and upgrade steps →](docs/wiki/Installation.md)

## Common setups

### Give one feature free, without Premium

Server console/root admin:

```text
sm_premium_feature_public airblast 1
sm_premium_feature "Dane" glow 1
```

The first command grants everyone airblast access; the second grants only the targeted account glow access. Neither creates Premium membership. The corresponding feature settings must also enable those effects. Use `0` to remove that grant source; other grants/membership may still supply access.

### Enable optional points and limit purchased trials

Put the desired settings in `tf/cfg/sourcemod/premium_rebuilt.cfg`:

```cfg
premium_points_enable "1"
premium_points_interval_minutes "10"
premium_points_per_interval "5"
premium_points_shop "1"
premium_points_rentals "1"
premium_points_trials "1"
premium_points_trial_limit "3"
```

Edit prices/durations in `premium_shop.cfg`. Players use `!premium_points` / `!premium_shop`; `!points` / `!shop` are convenience aliases when available. Set `premium_trial "0"` if you want points-only trials. Purchased-trial limits are lifetime per Steam ID across all trial offers; `0` explicitly means unlimited.

**Defaults:** the points master and purchased-trial switches are off. Free trials are on for 900 seconds. An individual rental gives only its feature; a purchased Premium trial intentionally gives the configured Premium bundle and Trial tag. Offline/disabled time counts toward expiry.

## Player commands

Start with `!premium`, `!premium_features`, `!premium_benefits` and `!premium_options`. Available commands include `!glow`, `!colorme`, `!premium_fov`, `!swapteam`, `!save_loc` and `!load_loc` when access/settings permit.

Tags apply automatically in normal chat. `sm_premium_chat` is only a diagnostic command, never an activation step.

[Full player/admin command reference →](docs/wiki/Commands.md)

## Project layout

- `scripting/` — Premium source and modules.
- `configs/` — main config, free-feature grants, shop catalog, class/map rules and balance profiles.
- `translations/` — SourceMod phrases.
- `docs/wiki/` — complete, versioned documentation and Wiki source pages.
- `docs/RC*_SERVER_TEST.md` — detailed historical acceptance checklists.
- `tests/`, `tools/`, `build.py` — build and verification tooling; not server runtime dependencies.

## License and credits

Original concept: **daneleo and noodleboy347**. The rewrite is licensed under GPL-3.0-or-later with the SourceMod linking exception; see [LICENSE.txt](LICENSE.txt) and [THIRD-PARTY.txt](THIRD-PARTY.txt).

TF2Attributes and More Colors are obtained from their authors. Dependency headers, runtime plugins, gamedata, compiler binaries and research/game binaries are excluded from this source repository. A locally compiled `premium.smx` embeds More Colors code; excluding the include does not erase its upstream licensing obligations. See the third-party notices before redistributing compiled artifacts.
