# Configuration reference

Use this page when you know what you want to change and need the setting name. The starting values and allowed ranges are taken from the plugin code. There are 97 settings in the main file.

Most switches use `0` for off and `1` for on. A setting may be on but still do nothing while its main system is off: for example, earning starts on, but points start off. Times and other units are explained beside each setting.

Set values in `tf/cfg/sourcemod/premium_rebuilt.cfg`. Use [Configuration](Configuration.md) for applying changes and examples. Per-class/map overrides, grant records and offer prices belong in their separate files.

## 01 / Main Switches

Start here. Other settings below choose amounts and limits.
For points-only trials: free trial OFF, points ON, purchased trials ON.

| Setting | Default | Bounds | Meaning |
| --- | --- | --- | --- |
| `premium_enable` | `1` | 0–1 | Master switch for all plugin features. |
| `premium_trial` | `1` | 0–1 | Free trials, including existing free-trial access. |
| `premium_points_enable` | `0` | 0–1 | Master points system; OFF by default. |
| `premium_points_earning` | `1` | 0–1 | Earn points from eligible play time. |
| `premium_points_shop` | `1` | 0–1 | Allow new purchases. |
| `premium_points_rentals` | `1` | 0–1 | Honor purchased individual features. |
| `premium_points_trials` | `0` | 0–1 | Allow/honor points-bought Premium trials. |

## 02 / Access And Free Trials

Free/per-player features: addons/sourcemod/configs/premium_features.cfg
Individual features do not grant Premium membership or a chat tag.
Trial clocks include offline time; disabling access never resets them.

| Setting | Default | Bounds | Meaning |
| --- | --- | --- | --- |
| `premium_flags` | `o` | Text | Premium member flag; premium_access override wins. |
| `premium_gameplay_teams` | `3` | 0–3 | 0 neither team | 1 RED | 2 BLU | 3 both. |
| `premium_trial_timelimit` | `900` | 1–86400 | Free trial length in SECONDS: 900 = 15 minutes. |
| `premium_trial_reminders` | `1` | 0–1 | Optional notices at 5 minutes, 1 minute, 10 seconds. |
| `premium_displaypremfeatures` | `1` | 0–1 | Show the menu after a successful free-trial claim. |

## 03 / Points: Earning

Example defaults: award 5 points per 10 eligible minutes.
Bots never earn. Dead/spectator/idle settings below control eligibility.

| Setting | Default | Bounds | Meaning |
| --- | --- | --- | --- |
| `premium_points_interval_minutes` | `10` | 1–1440 | Eligible MINUTES per award. |
| `premium_points_per_interval` | `5` | 1–1e+06 | Points per completed interval. |
| `premium_points_cap` | `1000000` | 1–1e+09 | Cap on NEW credits; lowering keeps existing balances. |
| `premium_points_min_players` | `1` | 1–100 | Minimum humans on counted teams. |
| `premium_points_count_spectators` | `0` | 0–1 | Include spectator/unassigned humans. |
| `premium_points_count_dead` | `1` | 0–1 | Count dead players' time on a playing team. |
| `premium_points_afk_seconds` | `0` | 0–3600 | Idle cutoff in SECONDS; 0 disables this filter. |
| `premium_points_earn_notice` | `1` | 0–1 | Private chat award notice; quiet mode suppresses it. |

## 04 / Points: Shop, Rentals And Purchased Trials

Prices and durations: addons/sourcemod/configs/premium_shop.cfg
Reload offers: sm_premium_shop_reload. Players: !points and !shop.
Purchased-trial limits are separate from the free-trial record.
Rental/trial expiry continues while offline or disabled. No auto-refunds.

| Setting | Default | Bounds | Meaning |
| --- | --- | --- | --- |
| `premium_points_trial_limit` | `1` | 0–10000 | Lifetime trial purchases per player; 0 = unlimited. |
| `premium_points_renewals` | `1` | 0–1 | Extend individual rentals; active trials cannot stack. |
| `premium_points_max_rental_days` | `365` | 1–365 | Maximum remaining individual rental time in DAYS. |
| `premium_points_purchase_notice` | `1` | 0–1 | Private successful-purchase receipt. |
| `premium_points_expiry_notice` | `1` | 0–1 | Private expiry notices; quiet mode suppresses them. |

## 05 / Engineer

SIX total building slots per player, shared across every building type.
Each teleporter end uses one slot. A pair uses two.
Type limits: 1 disables extras; maximum 3. Extra sentries stay LEVEL 1.
Metal is spawn reserve/capacity only: no command or automatic refill.

| Setting | Default | Bounds | Meaning |
| --- | --- | --- | --- |
| `premium_metal` | `200` | 0–2000 | Spawn metal/capacity; 200 is stock, 0 disables the perk. |
| `premium_sentry_limit` | `3` | 1–3 | Total sentries, including the ordinary sentry. |
| `premium_dispenser_limit` | `3` | 1–3 | Total dispensers. |
| `premium_teleporter_entrance_limit` | `3` | 1–3 | Total teleporter entrances. |
| `premium_teleporter_exit_limit` | `3` | 1–3 | Total teleporter exits. |
| `premium_team_extra_sentries` | `4` | 0–32 | Extra sentries per TEAM; 0 disables extras. |
| `premium_engineer_hud` | `1` | 0–1 | Small aimed-at building tooltip, not a large overlay. |
| `premium_sentry_labels` | `1` | 0–1 | Include primary/extra sentry role in the tooltip. |

## 06 / Health, Ammo And Respawn

Ammo changes RESERVES, including Pyro fuel; magazines stay normal.
Spawn bonuses are not continuous refills.

| Setting | Default | Bounds | Meaning |
| --- | --- | --- | --- |
| `premium_health_boost` | `15` | 0–500 | Extra maximum health and spawn health; 0 disables. |
| `premium_ammo_multiplier` | `1.25` | 1–10 | Reserve capacity/spawn ammo multiplier; 1 = normal. |
| `premium_healthbonus_kill` | `25` | 0–500 | Health per real kill; 0 disables. |
| `premium_healthbonus_assist` | `25` | 0–500 | Health per assist; 0 disables. |
| `premium_reward_health_cap` | `1.0` | 1–2 | Reward cap x maximum health; 1 = no overheal. |
| `premium_instant_respawn` | `1` | 0–1 | One-second respawn; mode/round restrictions apply. |

## 07 / Movement And Class Speed

Speed values are MULTIPLIERS, not movement units. 1 = normal speed.
Class/map overrides: addons/sourcemod/configs/premium_rules.cfg
Class rules override these defaults; map bans take priority.

| Setting | Default | Bounds | Meaning |
| --- | --- | --- | --- |
| `premium_speed` | `1` | 0–1 | Enable the class speed multipliers below. |
| `premium_speedmult_scout` | `1.0` | 1–2 | Scout speed multiplier. |
| `premium_speedmult_sniper` | `1.0` | 1–2 | Sniper speed multiplier. |
| `premium_speedmult_soldier` | `1.0` | 1–2 | Soldier speed multiplier. |
| `premium_speedmult_demoman` | `1.0` | 1–2 | Demoman speed multiplier. |
| `premium_speedmult_medic` | `1.0` | 1–2 | Medic speed multiplier. |
| `premium_speedmult_heavy` | `1.0` | 1–2 | Heavy speed multiplier. |
| `premium_speedmult_pyro` | `1.0` | 1–2 | Pyro speed multiplier. |
| `premium_speedmult_spy` | `1.0` | 1–2 | Spy speed multiplier. |
| `premium_speedmult_engineer` | `1.0` | 1–2 | Engineer speed multiplier. |
| `premium_gravity_mult` | `0.75` | 0.1–2 | Gravity multiplier; 1 = normal gravity. |
| `premium_bhop_enabled` | `1` | 0–1 | Held-jump bunnyhop/trimp. |
| `premium_bhop_increment` | `1.15` | 1–1.5 | Horizontal jump-speed multiplier. |
| `premium_bhop_speed_cap` | `640` | 300–1000 | Maximum boosted horizontal speed. |
| `premium_doublejump` | `0` | 0–1 | Optional follow-up trimp jump; needs bhop/trimp. |

## 08 / Pyro: Flame Flight And Airblast Jump

Flight and airblast boost are independent. Both obey map flight bans.
Airblast retains the weapon's normal fuel cost and cooldown.

| Setting | Default | Bounds | Meaning |
| --- | --- | --- | --- |
| `premium_pyrofly_enabled` | `1` | 0–1 | Jump + primary fire to fly; WASD steers. |
| `premium_pyrofly_speed` | `400` | 100–800 | Maximum horizontal flight speed. |
| `premium_pyrofly_lift` | `200` | 50–800 | Maximum climb/descent speed; controlled by aim. |
| `premium_pyrofly_acceleration` | `600` | 100–3000 | Flight steering acceleration per second. |
| `premium_airblast_enabled` | `1` | 0–1 | Right-click airblast recoil/jump boost. |
| `premium_airblast_force` | `400` | 0–1500 | Recoil impulse away from the aiming direction. |
| `premium_airblast_lift` | `350` | 0–1000 | Minimum upward boost impulse. |
| `premium_airblast_speed_cap` | `900` | 100–2000 | Cap on added boost; keeps faster existing momentum. |
| `premium_airblast_cooldown` | `0.75` | 0.1–10 | Minimum SECONDS between boosts. |

## 09 / Spy, Scout, Sniper And Medic

Cloak multipliers change duration, not activation cost or recharge.
Balls/jars recharge one at a time through the game's normal system.

| Setting | Default | Bounds | Meaning |
| --- | --- | --- | --- |
| `premium_cloak` | `1` | 0–1 | Enable both finite cloak-duration multipliers. |
| `premium_cloak_multiplier` | `2` | 1–20 | Ordinary/motion-watch duration multiplier; 1 = normal. |
| `premium_deadringer_multiplier` | `2` | 1–20 | Dead Ringer duration multiplier; 1 = normal. |
| `premium_deadringer` | `1` | 0–1 | Feigned-kill NOTICE only; does not change cloak. |
| `premium_stunballs` | `1` | 0–20 | Scout ball spawn reserve. |
| `premium_jarates` | `1` | 0–20 | Sniper jar spawn reserve. |
| `premium_firearrows` | `1` | 0–1 | Ignite supported bow arrows. |
| `premium_ubercharge` | `1` | 0–100 | Minimum Medic spawn charge in PERCENT, 0-100. |

## 10 / Team Swap And Saved-Location Teleport

Saved-location teleport is separate from Engineer teleporter buildings.

| Setting | Default | Bounds | Meaning |
| --- | --- | --- | --- |
| `premium_swapteams` | `1` | 0–1 | Allow team swapping. |
| `premium_swapteam_cooldown` | `60` | 0–600 | SECONDS between successful swaps. |
| `premium_swapteam_max_difference` | `1` | 0–32 | Allowed team-size difference; 0 = unrestricted. |
| `premium_teleport_enable` | `1` | 0–1 | Allow save_loc/load_loc; life/team/map restrictions apply. |
| `premium_teleport_cooldown` | `5` | 1–60 | Minimum SECONDS between successful location loads. |

## 11 / Chat Tags And Cosmetics

Normal player-chat tags are automatic for members/active trials only.
Individual free grants and feature rentals do not give a chat tag.
premium_tag below is the plugin's system-message prefix.

| Setting | Default | Bounds | Meaning |
| --- | --- | --- | --- |
| `premium_chat_tags` | `1` | 0–1 | Enable automatic membership/trial player-chat tags. |
| `premium_chat_tag` | `{green}[Premium]{default}` | Text | Permanent/flag member tag; More Colors markup. |
| `premium_trial_chat_tag` | `{lightgreen}[Trial]{default}` | Text | Active free/points-bought trial tag. |
| `premium_tag` | `{green}[{lightgreen}Premium{green}]{default}` | Text | Plugin system-message prefix; up to 63 characters. |
| `premium_tag_styles` | `7` | 0–7 | Allowed styles: add 1 team + 2 gold + 4 plain; 7 = all. |
| `premium_glow` | `1` | 0–1 | Allow !glow particle aura; hidden for cloaked/disguised spies. |
| `premium_glow_style` | `0` | 0–2 | 0 classic | 1 team | 2 subtle team. |
| `premium_personal_glow` | `1` | 0–1 | Allow players to choose an approved glow style. |
| `premium_color` | `1` | 0–6 | 0 off | 1 custom RGB | 2 red | 3 blue | 4 yellow | 5 cyan | 6 purple. |
| `premium_dark` | `1` | 0–1 | Allow black render tint; does not make players invisible. |
| `premium_fov_enable` | `1` | 0–1 | Allow/apply FOV settings. |
| `premium_fov_default` | `100` | 75–120 | Default FOV, 75-120. |
| `premium_round_celebration` | `1` | 0–1 | Short winning-team particle effect after combat. |
| `premium_round_celebration_seconds` | `2` | 0.5–5 | Celebration duration in SECONDS. |
| `premium_round_celebration_budget` | `8` | 0–16 | Maximum simultaneous celebration effects. |

## 12 / Adverts And Announcements

Player quiet preferences can hide optional adverts/reward messages.

| Setting | Default | Bounds | Meaning |
| --- | --- | --- | --- |
| `premium_advertisement_enable` | `1` | 0–1 | Advertise feature/access availability. |
| `premium_advertisement_interval` | `300` | 30–86400 | SECONDS between adverts. |
| `premium_welcome_message` | `1` | 0–1 | Private member/trial welcome message. |
| `premium_premiumjoin_announce` | `0` | 0–1 | Public member join announcement; off for privacy. |

## 13 / Storage (Advanced)

Default SQLite needs no manual table import; tables are created automatically.
Points uses the same backend. Settle pending purchases BEFORE changing it.
Back up the database and data/premium_points_outbox.cfg together.

| Setting | Default | Bounds | Meaning |
| --- | --- | --- | --- |
| `premium_use_mysql` | `0` | 0–1 | 0 = default storage-local SQLite | 1 = named backend below. |
| `premium_database` | `premium` | Text | databases.cfg entry; used only when premium_use_mysql is 1. |

## Compatibility and read-only values

`premium_clip_multiplier` is a live synchronized legacy alias of `premium_ammo_multiplier` (default `1.25`, bounds 1–10). It changes reserves, including fuel, not magazine capacity. Use the modern name in new configs to avoid contradictory assignments.

`premium_version` reports the running build and is not an owner setting.

A value within numeric bounds can still have no benefit: speed/ammo/cloak multiplier one is native, ball/jar reserve one is native, and metal 200 is stock. Membership/grants, class restrictions, map/team rules and native mode/state checks continue to apply.

[Class/map rules](Class-and-Map-Rules.md) · [Points offers](Points-and-Shop.md) · [Home](Home.md)
