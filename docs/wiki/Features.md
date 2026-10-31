# Features and grant keys

Each key below can be used in `premium_features.cfg`, grant commands and individual shop offers. A grant gives permission; settings, class/team/map restrictions and player preferences still determine the effect. It does not give membership or a chat tag.

## Gameplay

| Key | Effect / controls |
| --- | --- |
| `speed` | Configured class movement multiplier; `premium_speed` and class values apply |
| `gravity` | Configured gravity multiplier |
| `health` | Additional maximum/spawn health |
| `ammo` | Primary/secondary reserve capacity and spawn ammo; includes Pyro fuel/Heavy ammo, leaves magazines unchanged |
| `kill_health` | Health reward for a real kill, subject to health cap |
| `assist_health` | Health reward for an assist, subject to health cap |
| `balls` | Scout ball spawn reserve, supported Sandman/Wrap Assassin; normal recharge one at a time |
| `jarate` | Supported Sniper jar spawn reserve; normal recharge one at a time |
| `uber` | Minimum Medic spawn charge in percent |
| `respawn` | One-second respawn where game mode/round rules permit |
| `swap` | `!swapteam`; `!premium_swapstatus` explains current availability |
| `cloak` | Finite ordinary/motion-watch cloak-duration multiplier |
| `deadringer` | Finite Dead Ringer duration multiplier |
| `metal` | Engineer spawn metal/capacity; no continuous refill |
| `sentry` | Extra sentries through the normal PDA |
| `dispenser` | Extra dispensers through the normal PDA |
| `teleporter` | Extra teleporter entrances/exits through the normal PDA |
| `firearrows` | Ignite supported bow arrows |
| `bhop` | Held-jump bunnyhop/trimp with gain and speed cap |
| `doublejump` | Optional follow-up trimp jump; also needs the bhop/trimp feature |
| `flight` | Pyro jump + primary-fire flight, WASD steering and aim-controlled height |
| `airblast` | Pyro recoil/jump boost while looking down, from an actual fuel-consuming native right-click airblast |
| `teleport` | `!save_loc` / `!load_loc` saved-location teleport |

`teleporter` buildings and `teleport` saved locations are different features. The `deadringer` grant key controls duration, while the cvar named `premium_deadringer` controls the separate fake-kill notice below.

## Cosmetics and information

| Key | Effect / controls |
| --- | --- |
| `color` | `!colorme R G B` custom RGB where enabled; `reset` clears preference |
| `dark` | `!dark` black render tint with full opacity |
| `glow` | `!glow` particle aura; owner controls approved styles |
| `fov` | `!premium_fov 75–120` or `reset` |
| `celebration` | Short winning-team particle after combat ends |
| `fakekill` | Optional feigned-kill notice; does not extend cloak |

Chat tags reflect full membership or active trials and are not an individual feature grant. RGB/dark tint do not make a player invisible. Glow hides for cloaked/disguised spies and has tracked cleanup. Players can use `!premium_options` for supported tag, quiet, visibility, glow and tooltip preferences.

## Defaults and practical limits

The default reserve ammo multiplier is 1.25, health bonus 15, kill/assist reward 25 and reward cap 1.0 × maximum health. Class speed multipliers start at 1.0. Metal starts at 200, so raising the configured amount is necessary for extra metal. Scout/jar reserves start at one. Ordinary and Dead Ringer cloak duration multipliers start at 2.0; these do not change activation cost, resistance or recharge.

Flight checks weapon, fuel, movement and player state every update. Native airblast costs/cooldowns/reflection remain; a weapon without airblast does not acquire one. Map flight restrictions also block the airblast boost. Respawn and extra buildings have mode/round restrictions, including Arena/MvM limitations. Saved locations expire with life/team/map/connection changes and require a safe destination and eligible objective/movement state.

See [Engineer buildings](Engineer-Buildings.md), [Player guide](Player-Guide.md) and [all defaults](Configuration-Reference.md) for limits and use. Use `!premium_benefits` on the server to see the actual configured bonuses rather than assuming the defaults apply.

[Access](Access-and-Trials.md) · [Configuration](Configuration.md) · [Home](Home.md)
