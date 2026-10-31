# Premium Mod 3.0.0-rc13

A new TF2 / SourceMod implementation of Premium Mod, replacing the 2.0.2.1 development source audited on 31 October 2026. The original plugin files are untouched. This is a release candidate with a compiled plugin and automated coverage, awaiting TF2 server acceptance testing.

**rc3 translation hotfix:** rc2 used inline phrase sections that SourceMod rejects with `Section declared with unknown tokens`. Every phrase now uses the conservative multiline SMC layout. To fix an existing rc2 server, replace `addons/sourcemod/translations/premium_rebuilt.phrases.txt`, then run `sm_reload_translations` and `sm plugins reload premium` in the server console. Your existing plugin binary and configuration can stay in place for this translation-only fix. The rc3 package also contains a rebuilt binary reporting the new version. The format regression test rejects the original line-3 failure; it is a layout guard, not an execution of SourceMod's native parser.

The configuration files now use grouped sections, aligned values and shorter comments, with the main switches first. Every setting/default is unchanged. See `docs/CONFIGURATION.md` for a quick guide to the four files.

## Install

1. Use a current supported SourceMod installation with SDKTools, SDKHooks and the standard Client Preferences extension/plugin. This build was compiled with **SourceMod 1.12.0.7259**.
2. Stop/unload the old `premium.smx`. Both versions register overlapping commands and must not run together.
3. Back up the server's old plugin, configuration, translations, and database. Extract the install archive into the TF2 server's `tf/` directory, preserving `addons/` and `cfg/`.
4. Install **TF2Attributes** separately from [the author's releases](https://github.com/FlaminSarge/tf2attributes/releases): `tf2attributes.smx` goes in `addons/sourcemod/plugins/`, and `tf2.attributes.txt` goes in `addons/sourcemod/gamedata/`. This build targets API 1.7.5.1. If it is missing, incompatible, or fails to initialize, Premium logs an explicit warning with this link and stops before applying perks. The ZIP does not bundle that plugin, its gamedata, or its source.
5. Install `addons/sourcemod/configs/premium_rules.cfg` (new in rc8) and `addons/sourcemod/configs/premium_features.cfg` (new in rc12); install `addons/sourcemod/configs/premium_shop.cfg` for the optional RC13 shop, then review them and `cfg/sourcemod/premium_rebuilt.cfg`. Missing/invalid rules block flight and teleport until corrected/reloaded. Start a test map/restart the server and check `sm plugins list` and the SourceMod error log. Confirm `premium.smx` and `tf2attributes.smx` load without errors.
6. The standard SourceMod `storage-local` SQLite configuration is used by default. Tables are created asynchronously. No manual SQL import or database command is needed for a fresh installation. If your `databases.cfg` does not contain the standard `storage-local` entry, add it using the example supplied with the package.
7. Set premium flag `o` on a test account, or add that account using `sm_premadduser`. Exercise the acceptance checklist before moving the build to a populated server.

Premium's source tree is under `development/` in the install ZIP and is also available in `premium-rebuilt-3.0.0-rc13-source.zip`. The installed `addons/sourcemod/scripting/` copy includes Premium's main source and modules. Third-party headers are downloaded separately before rebuilding.

**More Colors is a compile-time include, not a runtime plugin.** The precompiled `premium.smx` already contains the colour implementation; installing `morecolors.inc` on the server has no runtime effect. To rebuild, download [More Colors from its author](https://github.com/DoctorMcKay/sourcemod-plugins/blob/master/scripting/include/morecolors.inc) and `tf2attributes.inc` from the TF2Attributes releases, then place them in `scripting/include/`. Missing headers produce an actionable build warning and explicit SourcePawn compile errors. The standard SourceMod Regex extension used by More Colors must be present on the server.

Separate TF2Attributes files and both dependency headers are excluded from the release ZIPs. The compiled install ZIP still embeds More Colors code, so excluding its `.inc` file does not remove the compiled third-party implementation. Attribution is retained in `THIRD-PARTY.txt`; this package does not claim redistribution clearance for upstream components. The separate source-only ZIP contains no compiled plugin or third-party headers/binaries, and lets recipients obtain the dependencies upstream before compiling.

**rc4 forum review:** all 40 pages / 394 posts have been reviewed (one old screenshot is unavailable). Sudden-death respawns are now blocked, continuous Pyro flight no longer stalls on the per-shot cooldown, Scout ball reserves use the game's grenade ammo pool, and movement boosts respect frozen input. See `docs/FORUM_REVIEW.md` for the issue-by-issue status and remaining server checks. When upgrading rc3, replace the plugin binary; keep your custom configuration and the corrected translations.

**rc13 optional points shop:** disabled by default. Configurable play-time rewards, private chat notices, persistent wallets, timed individual-feature rentals and separately limited points-bought Premium trials. Install the new `addons/sourcemod/configs/premium_shop.cfg`; use `!premium_points` and `!premium_shop`. Accepted operations use a durable local journal and transactional DB receipts to prevent repeat charges/rewards. See `docs/RC13_SERVER_TEST.md` for all switches, prices/durations and recovery checks.

**rc12 independent access:** selected features can be public or granted to individual Steam IDs without Premium membership, chat tags or unrelated perks. Install the new `addons/sourcemod/configs/premium_features.cfg`. Root commands `sm_premium_feature_public` and `sm_premium_feature` save grants; `sm_premium_features_reload` loads manual edits and `!premium_features` shows own access. `premium_trial 0` now also stops existing trial access while retaining original deadlines. See `docs/RC12_SERVER_TEST.md`.

**rc11 extra buildings:** configurable 1–3 dispensers, entrances and exits through the normal PDA, sharing the existing fixed six-building total. The compact owner tooltip and benefits page show the counts. No game DLL/SO replacement, memory patch, new dependency or gamedata is required. See `docs/RC11_SERVER_TEST.md` for configuration, the native PDA workaround and live acceptance checks.

**rc10 screenshot fixes:** native chat tags now accept TF2's actual `TF_Chat_All`-style keys without a leading `#`. The large white Engineer overlay is removed. An owner-only native tooltip appears on the turret while aiming, showing a compact role/count; the existing health/upgrade panel stays native. `!premium_chat` reports native chat diagnostics. See `docs/RC10_SERVER_TEST.md` for upgrade and live checks.

**rc9 airblast jump:** right-click with a real fuel-consuming airblast adds configurable recoil/lift, including takeoff from standing. Native ammo cost, cooldown and reflection remain native. A separate boost cooldown and speed cap prevent per-frame or unbounded thrust. Primary-fire flight is separate, and map flight bans cover both. See `docs/RC9_SERVER_TEST.md`.

**rc8 controls and presentation:** all 15 selected additions are implemented: Engineer counter/aimed labels, shared extra-sentry budgets, swap availability, per-class/map rules, trial reminders, a numeric benefits page, round-start preset queue, saved quiet/tag/glow/visibility choices and a bounded round-win effect. Install the new `addons/sourcemod/configs/premium_rules.cfg` and enable SourceMod Client Preferences. See `docs/RC8_SERVER_TEST.md` for the complete feature/control table and live checks.

**rc7 ammunition correction:** the perk increases primary/secondary reserve capacity and spawn ammunition, including Pyro fuel and other clipless normal-ammo weapons. It leaves magazines at their native size. Use `premium_ammo_multiplier`; the old `premium_clip_multiplier` is a synchronized compatibility alias. There is no ammo regeneration. Replace the plugin and translations, merge the new setting, then respawn to test. See `docs/RC7_SERVER_TEST.md`.

**rc6 server-feedback changes:** metal is extra capacity plus the initial spawn reserve, with no refill command or automatic top-up. Cloak has finite configurable duration multipliers, including a separate Dead Ringer value. Extra sentries use normal PDA selection and native blueprint construction; the first upgrades normally and the next two stay at level 1. Flight applies after movement, compensates actual server gravity, and supports the shared flamethrower class (including Backburner) plus Dragon's Fury. Editable Cosmetic/Balanced/Fun profiles and team controls are supplied. See `docs/RC6_SERVER_TEST.md` and `docs/BALANCE.md`. rc5's command-based sentries/metal and unlimited cloak are superseded.

## What changed

- Premium access is the union of a SourceMod permission check, database membership, and an unexpired persisted trial. The plugin never adds, removes, or replaces admin flags.
- Trials use one authoritative Unix expiry timestamp per Steam account. Offline time counts. Disconnect, map change, server restart, and disabling/re-enabling perks do not reset eligibility or extend a trial.
- The trial claim is an atomic database update. Two servers sharing the same ledger cannot both consume an unused trial successfully.
- Database connections, schema creation, reads, writes, and imports use asynchronous APIs. Failed storage denies database/trial entitlements; flag members keep access. Recovery retries every 30 seconds. Stale connection generations and player-session callbacks are rejected.
- One shared maintenance timer runs every 0.25 seconds. There is at most one owned respawn timer per player; no movement-command timers and no Heavy polling timers exist.
- Client storage uses `MAXPLAYERS + 1`; client natives require an in-game player index. Weapons are validated before property access.
- Speed uses the game's `move speed bonus` attribute. Reserve ammunition uses the player's primary/secondary maximum-ammo attributes, including Pyro fuel. Existing item modifiers are preserved, magazines stay native, and attribute ownership restores earlier runtime values conditionally.
- Render color has one final state: default, custom RGB, or dark tint. Dark is black with full alpha, and Spies are not made globally invisible.
- `!glow` is rebuilt: all four legacy particle entities are tracked, and partial creation is rolled back. Toggle-off, expiry, disabling, disconnect, map/plugin end, and player-state changes clean up the owned emitters. Cloak/disguise hides the aura without disabling the player's preference. Respawn restores it if access remains valid.
- Saved teleports are bound to the current connection, map, life, and team. They require a safe standing hull and have a load cooldown. Objective carriers cannot teleport.
- Expiry/disable restores effects owned by this plugin; it does not rewind already-spent ammunition or previous reward events. Other plugins that change the same attributes/properties can still require coordination.

## Commands

Commands accept their `sm_` form from the console and their `!` or `/` chat form. Old unprefixed console bindings are retained.

| Command | Behavior |
| --- | --- |
| `!premium` | Premium menu |
| `!premium_options` | Saved quiet, tag, glow, particle visibility, reminders and counter preferences |
| `!premium_benefits` | Numeric bonuses for your class/team, held ammo and limits |
| `!premium_swapstatus` | Why swapping is blocked, seconds left, or ready now |
| `sm_premium_rules_reload` | Root: reload class/map rules |
| `!trial` | Request the account's one-time trial, or show active status |
| `!premium_status` | Access and remaining trial time |
| `!premium_chat` | Your own tag configuration, processing counters and raw native message key |
| `!premium_weapons` | Report normal ammo pools, reserve counts, active capacity hooks, and attribute ownership |
| `!premium_flight` | Print your own flight access, weapon, firing state, ammo and movement diagnostics |
| `sm_premium_profile <cosmetic/balanced/fun> [next/now]` | Root: queue for next round by default; `status` / `cancel` also accepted |
| `!premium_fov 75-120` / `!premium_fov reset` | Validated FOV preference |
| `!colorme R G B` / `!colorme reset` | RGB tint, each channel 0-255 |
| `!dark` | Toggle black tint |
| `!glow` / `!glow on` / `!glow off` | Toggle the rebuilt particle aura; off always permits cleanup |
| `!swapteam` | Switch RED/BLU; unavailable in MvM or from spectator |
| `!save_loc` / `!load_loc` | Save/return to a safe location; `saveloc`/`loadloc` aliases also work |

Administrator commands default to root and honor normal SourceMod command overrides:

| Command | Behavior |
| --- | --- |
| `sm_premadduser <target or Steam ID>` | Add permanent database membership |
| `sm_premremuser <target or Steam ID>` | Remove database membership; other flags/trials can still grant access |
| `sm_createtrial <target or Steam ID>` | Grant/replace a finite trial using the configured duration |
| `sm_resettrial <target or Steam ID>` | Revoke that trial and allow a future one |
| `sm_prem_reload` | Reconcile live cvars, effects, and online membership; does not overwrite/save a config file |
| `sm_premium_database_create` | Reconnect and idempotently verify/create tables |
| `sm_premium_import_legacy [offset]` | Import up to 100 old membership rows, without altering the old table |
| `sm_premium_diagnostics` | Show version, storage readiness, active access/trial counts, glow emitter ownership and entity usage |

Membership commands accept online selectors (including dead players and multiple targets) or public-universe Steam2, Steam3, or Steam64 individual account IDs. Steam3 is the canonical stored key, including unsigned account IDs. Bots, malformed IDs, zero accounts, and non-public universes are rejected.

The default access override is `premium_access`, using `premium_flags "o"`. Independent feature overrides are `premium_feature_fov`, `premium_feature_dark`, `premium_feature_glow`, `premium_feature_color`, `premium_feature_swap`, `premium_feature_teleport`, `premium_feature_sentry`, `premium_feature_dispenser`, and `premium_feature_teleporter`. These feature overrides default to allowing an already-entitled player. RC12 also accepts explicitly defined `premium_feature_<key>` global overrides as independent feature grants for users who pass that permission check; an undefined override never gives non-members access. Feature permission denials apply to all access sources. Public/user grants are configured separately in `premium_features.cfg`; none creates membership. See `docs/RC12_SERVER_TEST.md` for all 29 feature keys.

## Settings and compatibility

The supplied configuration lists all supported settings and bounds. Cvars take effect live. `premium_enable 0` disables all perks and restores owned effects while the trial clock continues to expire.

| Legacy setting/behavior | Rewrite |
| --- | --- |
| `premium_speedmod_<class>` absolute speed | `premium_speedmult_<class>` multiplier, default **1.0** for every class |
| Heavy attack speed timer | Removed; TF2 handles minigun/weapon slows normally |
| Ammo bonus | `premium_ammo_multiplier`, default 1.25; primary/secondary reserve capacity and spawn ammo, including Pyro fuel; `premium_clip_multiplier` is a compatibility alias |
| Special weapon resources | Normal primary/secondary reserve pools receive the multiplier, including clipless weapons. Metal, grenades and meter/energy resources keep their own rules. Magazines are unchanged |
| `premium_color 1` forced green | 1 permits custom RGB without forcing a default tint; 2-6 select red/blue/yellow/cyan/purple defaults |
| Alpha-zero dark / Spy override | Full-alpha black tint; no automatic Spy invisibility |
| Cloak / Dead Ringer duration | Finite `premium_cloak_multiplier` / `premium_deadringer_multiplier`; activation fee, normal recharge and resistance duration stay native |
| Extra Engineer metal above 200 | Configurable capacity up to 2000 and initial spawn reserve, default 200; no command or automatic refill |
| Trial cookie reset on leave | Replaced by the persistent database ledger |
| Particle glow | Rebuilt and enabled via `premium_glow 1`; `premium_glow_style 0` retains all four original effects, or 1 uses two team-matching effects |
| More Colors markup in `premium_tag` | Preserved: `{green}`, `{lightgreen}`, `{default}` and other supported colour tags; console replies strip tags |

Health boost modifies max health through SDKHooks and grants the spawn bonus once per life. Kill/assist rewards only apply to living eligible recipients, exclude feigned deaths and self/team kills, and respect `premium_reward_health_cap`. Respawn skips sudden death, Arena, MvM, spectator/already-alive players, feigned deaths, and finished rounds. Special throwables and mediguns are checked before use.

Glow is a particle aura, not a through-wall player outline. It uses `player_recent_teleport_blue`, `player_recent_teleport_red`, `critical_grenade_blue`, and `critical_grenade_red`, matching the old code. No player targetname is modified, no timer is created per effect, and only this plugin's emitter entities are removed. Normal death/temporary hiding preserves the choice; access loss or disabling glow clears it. An entity allocation failure removes the incomplete set and retries after five seconds with throttled logging. The benefits page in `!premium` now lists the configured passive perks as well as feature commands.

Bunnyhop/trimp and Pyro recoil flight remain separate settings from base running speed. Held jump rearms the game's jump latch only while grounded, preserving airborne Scout and parachute inputs. Disable `premium_bhop_enabled` and/or `premium_pyrofly_enabled` if only normal movement should remain. Pyro flight requires a supported flamethrower in its active flame state, ammunition, and attack eligibility. Aim down for lift and use WASD for horizontal steering; horizontal speed approaches the requested movement rather than accumulating backward recoil. Airblast is excluded. Frozen players receive no bunnyhop or flight manipulation. The optional post-trimp jump is disabled by default and avoids zero-length vector normalization.

## Added perks and rechargeable resources

`premium_stunballs` and `premium_jarates` set supported throwable reserve capacities as well as the initial spawn reserve. The native effect bar regenerates **one item per native recharge interval** until full. Premium does not refill batches or add ammo every heartbeat. Ordinary resupply may fill the reserve normally. Disabling/revoking the perk restores owned capacity, without deleting ammunition already awarded.

`premium_ammo_multiplier` increases primary and secondary reserve capacity and fills the applicable equipped pools on spawn using the native capped ammo-supply function. At 2x, a stock flamethrower has 400 fuel instead of 200, and a stock Soldier has 40 reserve rockets while still loading 4. The range is 1-10; 1 keeps native capacity. Existing item ammo bonuses/penalties still combine. Live setting changes update capacity without filling the current life. Normal pickups/resupply can replenish it; Premium never refills spent ammo on its heartbeat. `!premium_weapons` reports the actual player capacity hooks, ownership and reserves. Energy/meter weapons without a normal ammo pool, Engineer metal and throwable pools are separate. `premium_clip_multiplier` remains a synchronized alias for old configs; use one name in saved configuration, preferably the new one. A selected profile may replace the value.

`premium_chat_tags 1` adds the configured `premium_chat_tag` to flag/database members and `premium_trial_chat_tag` to trial users. Defaults are `[Premium]` and `[Trial]`. These are player-chat tags; `premium_tag` still controls plugin-message prefixes. The module uses Simple Chat Processor / Chat-Processor forwards when available, otherwise intercepts TF2 player chat while preserving its recipients, localization and message text. It does not broadcast team chat globally. Extremely long names/messages may omit the tag to preserve the packet/name limit. Test other custom chat formatters; disable Premium tagging if another plugin supplies the desired tag.

Extra sentries use your normal construction PDA: select sentry, aim the blueprint, and place it. Native placement rules, construction time, metal cost, repairs, ammo and ownership registration apply. `premium_sentry_limit` caps the total at 1–3. The first sentry can upgrade normally; extra two are locked at level 1, including teammate wrench hits. With Gunslinger, native mini-sentry rules still apply. Replacing a destroyed primary creates an upgradeable primary even while extras remain. Tracked extras are removed on access/team/class changes, disable, disconnect, map/plugin end, or a lower cap; the ordinary primary is preserved. Extra sentries are disabled in Arena/MvM. The mode of existing sentries is temporarily changed during PDA/blueprint selection to satisfy native client/server mode-0 build checks, then restored. Their real type, builder, team and disposable flags stay intact. A carried sentry remains the mode-0 redeploy target even at cap. This interaction, sappers, hauling and multiple Wrangler targets need live server acceptance; the VM tests cover our tracking/restoration, not TF2 prediction.

`premium_metal` sets extra capacity and the initial Engineer spawn reserve. Inventory maintenance changes capacity only; spending never triggers a Premium refill. Native ammo pickups, dispensers and resupply can replenish metal normally. There is no `!metal` command.

`premium_cloak 1` enables finite drain scaling: `premium_cloak_multiplier 2` gives approximately twice the native cloak duration; `premium_deadringer_multiplier 3` gives approximately three times Dead Ringer's post-activation cloak duration. Both accept 1–20; 1 preserves native drain. The plugin scales the actual meter decrease across the game's cloaked PreThink, so existing item drain modifiers are retained. It never fills the meter to 100, accelerates recharge, refunds activation costs or extends resistance. Native terminal decloak is preserved, so time can be short by less than the multiplier times one simulation tick. Changes apply to current draining cloaks immediately. Cloak and Dagger retains native stationary recharge.

For flight, jump and hold primary fire. Aim down to climb, level to hover, up to descend, and use WASD to steer. Backburner, stock, Degreaser, Phlog and flame reskins share class support; Dragon's Fury arms flight on real primary shots. Release fire, run out of ammo, switch weapons, airblast, taunt, enter deep water, or lose access and the boost stops. `premium_pyrofly_speed`, `premium_pyrofly_lift` and `premium_pyrofly_acceleration` control speed, climb and handling. Run `sm_premium_flight` in your client console while holding fire if a weapon still fails.

`sm_premium_profile cosmetic`, `balanced`, or `fun` queues an editable preset for the next round start; add `now` for an explicit immediate change. Presets load from `cfg/sourcemod/premium_profiles/`. No preset is selected automatically. Execute one at the end of your main cfg to persist it. See `docs/BALANCE.md` for values, individual controls and optional cosmetic/team feature ideas.

## Storage and migration

SQLite is the default for both trial and membership records. To use the existing named MySQL configuration, set `premium_use_mysql 1` and `premium_database "premium"`. The named configuration can also point to SQLite. Supported drivers are MySQL and SQLite.

Two new tables are created: `premium_v3_members` (primary key `steamid`) and `premium_v3_trials` (`steamid`, `used`, `trial_until`). Player names never enter these queries. The old `premium` table is not modified.

To import old memberships, choose the database that contains the old table, stop the old plugin, and run `sm_premium_import_legacy 0`. Repeat using the next offset printed by the command until no rows remain. Steam2 and Steam3 rows normalize to the same key; duplicates are ignored. Invalid rows are skipped and batches use transactions. The import copies membership only; old clientprefs trial cookies are not imported, so this upgrade starts a new trial ledger for those accounts.

Changing databases switches to that database's ledger; it does not copy SQLite records into MySQL automatically. Back up and migrate records deliberately when changing backends. Membership changes on another server sharing the database are picked up on reconnect or `sm_prem_reload`; this release does not poll all memberships continuously.

## Build and tests

On Windows, with Python available:

```powershell
python tools/bootstrap.py --with-tests
python build.py --test
```

Bootstrap downloads the pinned official compiler and the TF2Attributes/More Colors compile-time headers directly from their authors, verifies SHA-256 hashes, and optionally installs `pysmx==0.4.0` into **this project's** `tools/python-libs`. It does not install Python packages globally or bundle the downloaded dependency files into release ZIPs. The normal build can be run with `python build.py` without the test interpreter.

The release compiles with **SourcePawn 1.12.0.7259 and warnings as errors**. The compiled shared policy tests execute in **pysmx 0.4.0 using SourcePawn 1.11.0.6931**, because that interpreter does not correctly execute all 1.12 bytecode features. This tests the actual unchanged `core.inc`, not a Python reimplementation of the policies. Small host adapters supply missing standard string/float natives, read mutable heap strings for formatting/replacement, and correct the interpreter's RTTI typeset-offset decoder; they do not change production bytecode or simulate TF2.

Current checks: **752 SourcePawn policy assertions**, **32 dependency assertions**, **269 glow ownership assertions**, **15 attribute ownership assertions**, **14 on-disk SQLite integration tests**, **78 building ownership assertions**, **18 reserve ammo assertions**, **1,064 preference/rule assertions**, **14 celebration ownership assertions**, **26 airblast boost assertions**, **34 native chat tag assertions**, **16 turret tooltip assertions**, and **29 artifact/translation/lifecycle checks**, plus **395 independent feature access assertions**, **230 points economy policy assertions** and **17 real SQLite economy integration tests**. The dependency harness executes the production checker with absent, incomplete, unready, and healthy providers. Glow tests execute the production controller with fake engine boundaries for full/partial creation, rollback, repeated toggles, stale/reused references, multiple players, slot 32 and the highest supported slot, visibility changes, and cleanup. They do not verify TF2 rendering. SQLite tests execute query/schema text extracted from the production source, including concurrent claims and transaction rollback. The attribute harness executes production ownership code, including isolation, missing-attribute repair, provider failure, owner-cache refresh and restoration. The ammo harness executes the production reserve controller and ownership helper with fake native/provider boundaries, including Pyro fuel, independent pools, native-cap delegation, item penalties, no maintenance refill and cleanup. The building harness executes production sentry/utility tracking and PDA restoration with fake entities, including six shared slots, separate caps, delayed wrench protection, hauling at cap, cap reduction, stale edicts and access loss. It does not simulate the engine build menu. Build results and file hashes are in `dist/build-report.json`; assertion counts are captured from the test runs.

If upgrading from rc1, keep your existing custom cvar values and add `premium_glow "1"` and `premium_glow_style "0"` to the configuration. Replace the plugin and translations; do not overwrite your existing configuration blindly. `docs/SERVER_TEST.md` gives the short server test sequence, and `docs/FEATURES.md` maps every original feature to its rewrite.

Not yet validated: live TF2 entity behavior, TF2Attributes gamedata against your server build, interactions with other plugins, and an actual MySQL server. Follow `docs/ACCEPTANCE.md` before treating this as a production release. Unload Premium before unloading/replacing TF2Attributes; third-party dependency unloading is not a supported hot-reload workflow.

Class/map overrides and all 15 rc8 features are documented in `docs/RC8_SERVER_TEST.md`. Preferences are stored separately in SourceMod clientprefs. The shared team budget defaults to four extra sentries per team; primary sentries do not count. Profile budgets are 0/2/8 for Cosmetic/Balanced/Fun. Map bans override class enables; class values override global profile cvars.

RC10 replaces the large Engineer HUD with a short, aimed-at native turret tooltip. Existing `premium_engineer_hud`, `premium_sentry_labels` and saved counter preferences control it. It uses owner-only native annotation events with no sound/world entities. The exact native health/upgrade panel has no custom text field; it is preserved unchanged. See `docs/RC10_SERVER_TEST.md`.

RC11 extends that compact tooltip to owned dispensers and teleporter ends, showing per-type counts and total slots. `premium_dispenser_limit`, `premium_teleporter_entrance_limit` and `premium_teleporter_exit_limit` accept 1–3. All types share six slots, including one per teleporter end. Ordinary utility upgrade levels stay native; only extra sentries have a level-one restriction. See `docs/RC11_SERVER_TEST.md`, including the brief native PDA pause during a nearby friendly wrench swing.

RC12 separates every automatic/command feature gate from membership. Public or personal grants never award Premium chat tags, announcements or other bonuses. Selected features can remain after trial/membership expiry. Disabling trials removes trial-only access immediately without resetting ledger deadlines. The feature resolver VM tests exercise the production auth/permission/source merge, isolated grants, revocation, denied overrides, team/global gates and membership/trial separation; SourceMod native file persistence and live TF2 execution remain server acceptance checks.

RC13 adds the optional points economy described in `docs/RC13_SERVER_TEST.md`. Individual rentals remain separate from membership; a points-bought trial intentionally supplies timed Premium trial membership and its Trial tag, with a separate lifetime purchase counter. Default economy and paid-trial switches are off. The SQL integration harness runs statements emitted by the compiled production query builder, including insufficient balance, declined-intent replay, renewal, rollback, concurrent buys, trial limits and restart. SourceMod filesystem/callback recovery and live MySQL/game execution remain server acceptance checks.
