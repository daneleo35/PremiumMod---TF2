# Configuration reference

Defaults from the current plugin. Edit quoted values in `cfg/sourcemod/premium_rebuilt.cfg`; the main guide explains how to apply them.

[Setup guide](Configuration.md) · [RC21 additions](RC21-Additions.md)

## 01 / Main Switches

Start here. Other settings below choose amounts and limits.
For points-only trials: free trial OFF, points ON, purchased trials ON.

| Setting | Default | Bounds | Meaning |
| --- | --- | --- | --- |
| `premium_enable` | `1` | 0.0–1.0 | Master switch for all plugin features. |
| `premium_trial` | `1` | 0.0–1.0 | Free trials, including existing free-trial access. |
| `premium_points_enable` | `0` | 0.0–1.0 | Master points system; OFF by default. |
| `premium_points_earning` | `1` | 0.0–1.0 | Earn points from eligible play time. |
| `premium_points_shop` | `1` | 0.0–1.0 | Allow new purchases. |
| `premium_points_rentals` | `1` | 0.0–1.0 | Honor purchased individual features. |
| `premium_points_trials` | `0` | 0.0–1.0 | Allow/honor points-bought Premium trials. |

## 02 / Access And Free Trials

Free/per-player features: addons/sourcemod/configs/premium_features.cfg
Individual features do not grant Premium membership or a chat tag.
Trial clocks include offline time; disabling access never resets them.

| Setting | Default | Bounds | Meaning |
| --- | --- | --- | --- |
| `premium_flags` | `o` | Text | Premium member flag; premium_access override wins. |
| `premium_gameplay_teams` | `3` | 0.0–3.0 | 0 neither team / 1 RED / 2 BLU / 3 both. |
| `premium_trial_timelimit` | `900` | 1.0–86400.0 | Free trial length in SECONDS: 900 = 15 minutes. |
| `premium_trial_reminders` | `1` | 0.0–1.0 | Optional notices at 5 minutes, 1 minute, 10 seconds. |
| `premium_displaypremfeatures` | `1` | 0.0–1.0 | Show the menu after a successful free-trial claim. |

## 03 / Points: Earning

Example defaults: award 5 points per 10 eligible minutes.
Bots never earn. Dead/spectator/idle settings below control eligibility.

| Setting | Default | Bounds | Meaning |
| --- | --- | --- | --- |
| `premium_points_interval_minutes` | `10` | 1.0–1440.0 | Eligible MINUTES per award. |
| `premium_points_per_interval` | `5` | 1.0–1000000.0 | Points per completed interval. |
| `premium_points_cap` | `1000000` | 1.0–1000000000.0 | Cap on NEW credits; lowering keeps existing balances. |
| `premium_points_min_players` | `1` | 1.0–100.0 | Minimum humans on counted teams. |
| `premium_points_count_spectators` | `0` | 0.0–1.0 | Include spectator/unassigned humans. |
| `premium_points_count_dead` | `1` | 0.0–1.0 | Count dead players' time on a playing team. |
| `premium_points_afk_seconds` | `0` | 0.0–3600.0 | Idle cutoff in SECONDS; 0 disables this filter. |
| `premium_points_earn_notice` | `1` | 0.0–1.0 | Private chat award notice; quiet mode suppresses it. |

## 04 / Points: Shop, Rentals And Purchased Trials

Prices and durations: addons/sourcemod/configs/premium_shop.cfg
Reload offers: sm_premium_shop_reload. Players: !points and !shop.
Purchased-trial limits are separate from the free-trial record.
Rental/trial expiry continues while offline or disabled. No auto-refunds.

| Setting | Default | Bounds | Meaning |
| --- | --- | --- | --- |
| `premium_points_gifts` | `0` | 0.0–1.0 | Buy individual feature rentals for other connected players. |
| `premium_points_transfers` | `0` | 0.0–1.0 | Gift points to other connected players. |
| `premium_points_transfer_max` | `1000` | 1.0–1000000.0 | Maximum points per transfer; minimum 1. |
| `premium_points_transfer_daily` | `5000` | 0.0–1000000000.0 | Maximum points sent per UTC day; 0 unlimited. |
| `premium_shop_specials` | `0` | 0.0–1.0 | Apply offer windows and scheduled discounts in the shop file. |
| `premium_shop_pause_unavailable` | `1` | 0.0–1.0 | Pause purchases for map-disabled movement features. |
| `premium_points_trial_limit` | `1` | 0.0–10000.0 | Lifetime trial purchases per player; 0 = unlimited. |
| `premium_points_renewals` | `1` | 0.0–1.0 | Extend individual rentals; active trials cannot stack. |
| `premium_points_max_rental_days` | `365` | 1.0–365.0 | Maximum remaining individual rental time in DAYS. |
| `premium_points_purchase_notice` | `1` | 0.0–1.0 | Private successful-purchase receipt. |
| `premium_points_expiry_notice` | `1` | 0.0–1.0 | Private expiry notices; quiet mode suppresses them. |

## 05 / Engineer

SIX total building slots per player, shared across every building type.
Each teleporter end uses one slot. A pair uses two.
Type limits: 1 disables extras; maximum 3. Extra sentries stay LEVEL 1.
Metal is spawn reserve/capacity only: no command or automatic refill.

| Setting | Default | Bounds | Meaning |
| --- | --- | --- | --- |
| `premium_metal` | `200` | 0.0–2000.0 | Spawn metal/capacity; 200 is stock, 0 disables the perk. |
| `premium_sentry_limit` | `3` | 1.0–3.0 | Total sentries, including the ordinary sentry. |
| `premium_dispenser_limit` | `3` | 1.0–3.0 | Total dispensers. |
| `premium_teleporter_entrance_limit` | `3` | 1.0–3.0 | Total teleporter entrances. |
| `premium_teleporter_exit_limit` | `3` | 1.0–3.0 | Total teleporter exits. |
| `premium_team_extra_dispensers` | `-1` | -1.0–128.0 | Team extra dispensers: -1 unlimited, 0 disables extras. |
| `premium_team_extra_entrances` | `-1` | -1.0–128.0 | Team extra entrances: -1 unlimited, 0 disables extras. |
| `premium_team_extra_exits` | `-1` | -1.0–128.0 | Team extra exits: -1 unlimited, 0 disables extras. |
| `premium_building_overview` | `1` | 0.0–1.0 | Allow !premium_buildings health, level and sapper menu. |
| `premium_team_extra_sentries` | `4` | 0.0–32.0 | Extra sentries per TEAM; 0 disables extras. |
| `premium_engineer_hud` | `1` | 0.0–1.0 | Small aimed-at building tooltip, not a large overlay. |
| `premium_sentry_labels` | `1` | 0.0–1.0 | Include primary/extra sentry role in the tooltip. |

## 06 / Health, Ammo And Respawn

Ammo changes RESERVES, including Pyro fuel; magazines stay normal.
Spawn bonuses are not continuous refills.

| Setting | Default | Bounds | Meaning |
| --- | --- | --- | --- |
| `premium_health_boost` | `15` | 0.0–500.0 | Extra maximum health and spawn health; 0 disables. |
| `premium_ammo_multiplier` | `1.25` | 1.0–10.0 | Reserve capacity/spawn ammo multiplier; 1 = normal. |
| `premium_healthbonus_kill` | `25` | 0.0–500.0 | Health per real kill; 0 disables. |
| `premium_healthbonus_assist` | `25` | 0.0–500.0 | Health per assist; 0 disables. |
| `premium_reward_health_cap` | `1.0` | 1.0–2.0 | Reward cap x maximum health; 1 = no overheal. |
| `premium_instant_respawn` | `1` | 0.0–1.0 | One-second respawn; mode/round restrictions apply. |

## 07 / Movement And Class Speed

Speed values are MULTIPLIERS, not movement units. 1 = normal speed.
Class/map overrides: addons/sourcemod/configs/premium_rules.cfg
Class rules override these defaults; map bans take priority.

| Setting | Default | Bounds | Meaning |
| --- | --- | --- | --- |
| `premium_speed` | `1` | 0.0–1.0 | Enable the class speed multipliers below. |
| `premium_speedmult_scout` | `1.0` | 1–2 | Scout speed multiplier. |
| `premium_speedmult_sniper` | `1.0` | 1–2 | Sniper speed multiplier. |
| `premium_speedmult_soldier` | `1.0` | 1–2 | Soldier speed multiplier. |
| `premium_speedmult_demoman` | `1.0` | 1–2 | Demoman speed multiplier. |
| `premium_speedmult_medic` | `1.0` | 1–2 | Medic speed multiplier. |
| `premium_speedmult_heavy` | `1.0` | 1–2 | Heavy speed multiplier. |
| `premium_speedmult_pyro` | `1.0` | 1–2 | Pyro speed multiplier. |
| `premium_speedmult_spy` | `1.0` | 1–2 | Spy speed multiplier. |
| `premium_speedmult_engineer` | `1.0` | 1–2 | Engineer speed multiplier. |
| `premium_gravity_mult` | `0.75` | 0.1–2.0 | Gravity multiplier; 1 = normal gravity. |
| `premium_bhop_enabled` | `1` | 0.0–1.0 | Held-jump bunnyhop/trimp. |
| `premium_bhop_increment` | `1.15` | 1.0–1.5 | Horizontal jump-speed multiplier. |
| `premium_bhop_speed_cap` | `640` | 300.0–1000.0 | Maximum boosted horizontal speed. |
| `premium_doublejump` | `0` | 0.0–1.0 | Optional follow-up trimp jump; needs bhop/trimp. |

## 08 / Pyro: Flame Flight And Airblast Jump

Flight and airblast boost are independent. Both obey map flight bans.
Airblast retains the weapon's normal fuel cost and cooldown.

| Setting | Default | Bounds | Meaning |
| --- | --- | --- | --- |
| `premium_pyrofly_enabled` | `1` | 0.0–1.0 | Jump + primary fire to fly; WASD steers. |
| `premium_pyrofly_speed` | `400` | 100.0–800.0 | Maximum horizontal flight speed. |
| `premium_pyrofly_lift` | `200` | 50.0–800.0 | Maximum climb/descent speed; controlled by aim. |
| `premium_pyrofly_acceleration` | `600` | 100.0–3000.0 | Flight steering acceleration per second. |
| `premium_airblast_enabled` | `1` | 0.0–1.0 | Look down + right-click airblast recoil/jump boost. |
| `premium_airblast_force` | `400` | 0.0–1500.0 | Recoil impulse away from the aiming direction. |
| `premium_airblast_lift` | `350` | 0.0–1000.0 | Minimum upward boost impulse. |
| `premium_airblast_speed_cap` | `900` | 100.0–2000.0 | Cap on added boost; keeps faster existing momentum. |
| `premium_airblast_cooldown` | `0.75` | 0.1–10.0 | Minimum SECONDS between boosts. |

## 09 / Spy, Scout, Sniper And Medic

Cloak multipliers change duration, not activation cost or recharge.
Balls/jars recharge one at a time through the game's normal system.

| Setting | Default | Bounds | Meaning |
| --- | --- | --- | --- |
| `premium_cloak` | `1` | 0.0–1.0 | Enable both finite cloak-duration multipliers. |
| `premium_cloak_multiplier` | `2` | 1.0–20.0 | Ordinary/motion-watch duration multiplier; 1 = normal. |
| `premium_deadringer_multiplier` | `2` | 1.0–20.0 | Dead Ringer duration multiplier; 1 = normal. |
| `premium_deadringer` | `1` | 0.0–1.0 | Feigned-kill NOTICE only; does not change cloak. |
| `premium_stunballs` | `1` | 0.0–20.0 | Scout ball spawn reserve. |
| `premium_jarates` | `1` | 0.0–20.0 | Sniper jar spawn reserve. |
| `premium_firearrows` | `1` | 0.0–1.0 | Ignite supported bow arrows. |
| `premium_ubercharge` | `1` | 0.0–100.0 | Minimum Medic spawn charge in PERCENT, 0-100. |

## 10 / Team Swap And Saved-Location Teleport

Saved-location teleport is separate from Engineer teleporter buildings.

| Setting | Default | Bounds | Meaning |
| --- | --- | --- | --- |
| `premium_swapteams` | `1` | 0.0–1.0 | Allow team swapping. |
| `premium_swapteam_cooldown` | `60` | 0.0–600.0 | SECONDS between successful swaps. |
| `premium_swapteam_max_difference` | `1` | 0.0–32.0 | Allowed team-size difference; 0 = unrestricted. |
| `premium_teleport_enable` | `1` | 0.0–1.0 | Allow save_loc/load_loc; life/team/map restrictions apply. |
| `premium_teleport_cooldown` | `5` | 1.0–60.0 | Minimum SECONDS between successful location loads. |

## 11 / Chat Tags And Cosmetics

Normal player-chat tags are automatic for members/active trials only.
Individual free grants and feature rentals do not give a chat tag.
premium_tag below is the plugin's system-message prefix.
Choose any tag wording and More Colors names, ending with {default}.
Member example: premium_chat_tag "{gold}[VIP]{default}"
Trial example: premium_trial_chat_tag "{aqua}[Guest]{default}"
Set premium_tag_styles "0" to keep your chosen colours for everyone.
Apply edits with: exec sourcemod/premium_rebuilt.cfg

| Setting | Default | Bounds | Meaning |
| --- | --- | --- | --- |
| `premium_chat_tags` | `1` | 0.0–1.0 | Enable automatic membership/trial player-chat tags. |
| `premium_chat_tag` | `{green}[Premium]{default}` | Text | Permanent/flag member tag; More Colors markup. |
| `premium_trial_chat_tag` | `{lightgreen}[Trial]{default}` | Text | Active free/points-bought trial tag. |
| `premium_tag` | `{green}[{lightgreen}Premium{green}]{default}` | Text | Plugin system-message prefix; up to 63 characters. |
| `premium_tag_styles` | `7` | 0.0–7.0 | Allowed styles: add 1 team + 2 gold + 4 plain; 7 = all. |
| `premium_glow` | `1` | 0.0–1.0 | Allow !glow particle aura; hidden for cloaked/disguised spies. |
| `premium_glow_style` | `0` | 0.0–2.0 | 0 classic / 1 team / 2 subtle team. |
| `premium_personal_glow` | `1` | 0.0–1.0 | Allow players to choose an approved glow style. |
| `premium_color` | `1` | 0.0–6.0 | 0 off / 1 custom RGB / 2 red / 3 blue / 4 yellow / 5 cyan / 6 purple. |
| `premium_dark` | `1` | 0.0–1.0 | Allow black render tint; does not make players invisible. |
| `premium_fov_enable` | `1` | 0.0–1.0 | Allow/apply FOV settings. |
| `premium_fov_default` | `100` | 75.0–120.0 | Default FOV, 75-120. |
| `premium_spawn_effect` | `0` | 0.0–1.0 | Brief spawn cosmetic for members; OFF by default. |
| `premium_spawn_effect_style` | `0` | 0.0–1.0 | 0 team teleport shimmer / 1 team critical sparkle. |
| `premium_spawn_effect_seconds` | `1.5` | 0.25–5.0 | Spawn effect duration in SECONDS. |
| `premium_spawn_effect_budget` | `8` | 0.0–32.0 | Maximum simultaneous spawn emitters. |
| `premium_round_celebration` | `1` | 0.0–1.0 | Short winning-team particle effect after combat. |
| `premium_round_celebration_seconds` | `2` | 0.5–5.0 | Celebration duration in SECONDS. |
| `premium_round_celebration_budget` | `8` | 0.0–16.0 | Maximum simultaneous celebration effects. |

## 12 / Adverts And Announcements

Player quiet preferences can hide optional adverts/reward messages.

| Setting | Default | Bounds | Meaning |
| --- | --- | --- | --- |
| `premium_advertisement_enable` | `1` | 0.0–1.0 | Advertise feature/access availability. |
| `premium_advertisement_interval` | `300` | 30.0–86400.0 | SECONDS between adverts. |
| `premium_welcome_message` | `1` | 0.0–1.0 | Private member/trial welcome message. |
| `premium_premiumjoin_announce` | `0` | 0.0–1.0 | Public member join announcement; off for privacy. |

## 13 / Storage (Advanced)

Default SQLite needs no manual table import; tables are created automatically.
Points uses the same backend. Settle pending purchases BEFORE changing it.
Back up the database and data/premium_points_outbox.cfg together.

| Setting | Default | Bounds | Meaning |
| --- | --- | --- | --- |
| `premium_use_mysql` | `0` | 0.0–1.0 | 0 = default storage-local SQLite / 1 = named backend below. |
| `premium_database` | `premium` | Text | databases.cfg entry; used only when premium_use_mysql is 1. |
