# Command reference

Player examples use `!name` in chat. Their console forms are `sm_name`; standard `/name` silent chat triggers depend on your SourceMod configuration. Legacy unprefixed console forms are also registered. Admin examples below are server-console/server/admin commands and honour normal SourceMod command permission overrides.

## Player menus and status

| Chat command | Use |
| --- | --- |
| `!premium` | Main menu |
| `!premium_admin` | Admin menu for authorised admins; ordinary players cannot open it |
| `!premium_status` | Membership and remaining free/purchased trial information |
| `!premium_features` | Paged list of your feature access; permission does not override balance/map settings |
| `!premium_benefits` | Actual configured bonuses, held resources and building counts for your current class/team |
| `!premium_options` | Saved display/notification preferences |
| `!premium_swapstatus` | Swap availability, cooldown or blocking reason |
| `!trial` | Request a one-time free trial, or show status if already a member/trial |
| `!premium_points` | Own wallet balance and purchased-trial usage |
| `!premium_shop` | Browse offers, then confirm a purchase |
| `!points`, `!shop` | Optional convenience aliases, registered only if unused at startup |

## Feature actions

| Command | Arguments / example |
| --- | --- |
| `!glow` | Toggle aura |
| `!glow on` / `!glow off` | Explicit state; off permits cleanup even after access changes |
| `!colorme <R> <G> <B>` | Integers 0–255; example `!colorme 255 100 100` |
| `!colorme reset` | Clear custom RGB preference |
| `!dark` | Toggle black tint; not invisibility |
| `!premium_fov <75–120>` | Example `!premium_fov 100` |
| `!premium_fov reset` | Use the server's configured default |
| `!swapteam` | Swap RED/BLU if round, balance and cooldown checks pass |
| `!save_loc` | Save a safe current location |
| `!load_loc` | Return to the saved location if still valid |
| `!saveloc` / `!loadloc` | Aliases for save/load |

These commands do not create access. The corresponding feature grant/membership, master/feature enable settings and gameplay checks must permit them. Normal-PDA building, automatic ammo/health bonuses, cloak multipliers and Pyro movement do not require a separate Premium activation command.

## Own diagnostic commands

| Console command | Output |
| --- | --- |
| `sm_premium_chat` | Tag enable/hidden/style state, processor/counters and native chat format key; not an activation command |
| `sm_premium_weapons` | Reserve pools, held ammo and capacity/attribute ownership |
| `sm_premium_flight` | Weapon/item/state, fuel, movement, configured flight and airblast diagnostics |

## Membership and free trials

Most common admin actions are also available through `!premium_admin`: select players, manage membership/free features/points, choose presets and change/save numeric settings. See [In-game menus](In-Game-Menus.md).

| Server/admin command | Behaviour |
| --- | --- |
| `sm_premadduser <target-or-Steam-ID>` | Add permanent database membership |
| `sm_premremuser <target-or-Steam-ID>` | Remove database membership only; flags/trials/grants can still provide access |
| `sm_createtrial <target-or-Steam-ID>` | Grant/replace a finite free-trial record using `premium_trial_timelimit`; trial access still requires `premium_trial 1` |
| `sm_resettrial <target-or-Steam-ID>` | Revoke/reset the free-trial record and allow a future free claim; does not reset purchased-trial usage |

Examples:

```text
sm_premadduser "Dane"
sm_premremuser "STEAM_1:0:123456"
sm_createtrial "STEAM_1:0:123456"
sm_resettrial "STEAM_1:0:123456"
```

Membership commands support online selectors, dead players, multiple targets and offline Steam IDs. Public-universe Steam2, Steam3 and Steam64 individual IDs are converted to canonical Steam3. Bots and invalid IDs are rejected. Name/online selectors use SourceMod targeting/immunity rules; a supplied Steam ID addresses that account directly.

## Individual free features

| Server/admin command | Behaviour |
| --- | --- |
| `sm_premium_feature_public <feature> <0-or-1>` | Persist public access for one feature |
| `sm_premium_feature <target-or-Steam-ID> <feature> <0-or-1>` | Persist an individual account's grant |
| `sm_premium_features_reload` | Read manual changes to `premium_features.cfg` |

```text
sm_premium_feature_public airblast 1
sm_premium_feature "Dane" glow 1
sm_premium_feature "STEAM_1:0:123456" dispenser 1
sm_premium_feature "STEAM_1:0:123456" dispenser 0
```

`0` removes that source's grant, not every possible source of access. Commands rewrite the grant file in a standard layout and retain a `.bak`; reload manual edits first. See [all feature keys](Features.md).

## Points and shop administration

| Server/admin command | Behaviour |
| --- | --- |
| `sm_premium_points_adjust <player-or-Steam-ID> <signed-amount>` | Queue an additive adjustment from −1,000,000 to +1,000,000, excluding zero; balance clamps at zero/new-credit cap |
| `sm_premium_shop_reload` | Reload offer prices/durations and invalidate open confirmation quotes |

```text
sm_premium_points_adjust "Dane" 100
sm_premium_points_adjust "STEAM_1:0:123456" -25
sm_premium_shop_reload
```

Adjustments require the economy/storage to be enabled/ready. They do not cancel rentals or reset trial counters. Purchases remain self-only through the player's shop menu. A shop purchase is granted only after the purchase has saved.

## Balance, reload and storage

| Server/admin command | Behaviour |
| --- | --- |
| `sm_premium_rules_reload` | Reload class/map rules |
| `sm_premium_profile <cosmetic\|balanced\|fun> [next\|now]` | Queue for next round by default; `now` applies immediately |
| `sm_premium_profile status` | Show queued preset |
| `sm_premium_profile cancel` | Clear the queue |
| `sm_prem_reload` | Update live values/effects and reload online membership; **does not read or save the cfg file** |
| `sm_premium_database_create` | Reconnect and idempotently verify/create schema; not a database reset |
| `sm_premium_import_legacy [offset]` | Import one batch of up to 100 old `premium` table memberships; follow returned next offset |
| `sm_premium_diagnostics` | Version, storage readiness, access/trial counts, glow ownership and entity usage |

To apply the main file from console:

```text
exec sourcemod/premium_rebuilt.cfg
sm_prem_reload
```

Settle pending economy operations before changing storage backend settings. `sm_reload_translations` and `sm plugins list` are standard SourceMod commands, not Premium commands.

[Configuration](Configuration.md) · [Troubleshooting](Troubleshooting.md) · [Home](Home.md)

## Buildings and gifts (RC21)

| Command | What it does |
| --- | --- |
| `!premium_buildings` | Your placed buildings, with health, level and sapper status. Refresh to update. |
| `!premium_gift` | Choose a connected player and buy them an individual feature rental. |
| `!premium_giftpoints` | Choose a connected player and an amount of points to send. |
| `!premium_giftpoints "Player name" 25` | Open confirmation for an exact points amount; nothing moves until confirmed. |

Gifting has to be enabled by the owner. A feature gift gives only that feature, with no membership or chat tag. Points gifting is separate from the shop's purchase switch. See [RC21 setup](RC21-Additions.md).
