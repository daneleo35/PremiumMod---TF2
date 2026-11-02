# Configuration setup

The bundled main config has numbered sections, aligned values and short comments. Switches normally use `0` for off and `1` for on; exceptions are labelled. Keep quoted values and setting names intact.

You can also edit 105 numeric settings through `!premium_admin` → Server settings. Apply changes live, then explicitly Save to keep them. The saved block runs last in the main file; see [In-game menus](In-Game-Menus.md).

## Which file to edit

| Server path, relative to `tf/` | Purpose | Apply changes |
| --- | --- | --- |
| `cfg/sourcemod/premium_rebuilt.cfg` | Main switches, amounts, points earning, trial limits, chat, storage | Restart/change map, or execute file then update |
| `addons/sourcemod/configs/premium_features.cfg` | Public/free and individual Steam-ID grants | `sm_premium_features_reload` |
| `addons/sourcemod/configs/premium_shop.cfg` | Offer enable/name/feature/price/duration/unit | `sm_premium_shop_reload` |
| `addons/sourcemod/configs/premium_rules.cfg` | Per-class overrides and map restrictions | `sm_premium_rules_reload` |
| `cfg/sourcemod/premium_profiles/*.cfg` | Editable cosmetic/balanced/fun presets | Select with `sm_premium_profile` |

The source copies are in the repository's [configs directory](https://github.com/daneleo35/PremiumMod---TF2/tree/main/configs). They are not installed simply by putting the repository folder beside your server.

## Main-file sections

1. Main switches.
2. Access and free trials.
3. Points earning.
4. Points shop, rentals and purchased trials.
5. Engineer.
6. Health, ammo and respawn.
7. Movement and class speed.
8. Pyro flight and airblast jump.
9. Spy/Scout/Sniper/Medic.
10. Team swap and saved-location teleport.
11. Chat tags and cosmetics.
12. Adverts and announcements.
13. Storage.

The end contains **commented-out** optional preset examples. Leave all commented to use the values above. If selecting a persistent preset, uncomment only one `exec` at the end: it runs last and overrides matching main-file values.

## Common setups

### Premium members, with no free trials

```cfg
premium_enable "1"
premium_trial "0"
```

Give membership through the configured SourceMod permission or `sm_premadduser`. Disabling free trials also stops existing free-trial access, without resetting its saved deadline/eligibility.

### Selected features free to everyone

Keep `premium_enable "1"`. In `premium_features.cfg`, enable just the chosen Public keys, or use:

```text
sm_premium_feature_public airblast 1
sm_premium_feature_public glow 1
```

These grants are separate from membership. The corresponding feature settings still need to enable useful effects; a preset/map ban can disable them.

### Points earning and individual rentals only

```cfg
premium_points_enable "1"
premium_points_earning "1"
premium_points_interval_minutes "10"
premium_points_per_interval "5"
premium_points_shop "1"
premium_points_rentals "1"
premium_points_trials "0"
```

Edit the shop's individual feature offers. This does not grant membership or tags.

### Points-bought Premium trials, limited to three per account

```cfg
premium_trial "0"
premium_points_enable "1"
premium_points_shop "1"
premium_points_trials "1"
premium_points_trial_limit "3"
```

Add an enabled `feature "trial"` offer in the shop. The limit is lifetime across trial offers; `0` explicitly means unlimited. Free-trial records and purchased-trial counters are separate.

### Cosmetic gameplay policy

```text
sm_premium_profile cosmetic next
```

Or set `premium_gameplay_teams "0"` to block gameplay bonuses on both teams. Membership and configured cosmetics can remain. Presets do not rewrite membership or the free-feature grant file. See [Class/map rules](Class-and-Map-Rules.md).

## Custom chat tags

Choose the wording and colour for members and trials in section 11 of `cfg/sourcemod/premium_rebuilt.cfg`. For example:

```cfg
premium_chat_tags       "1"
premium_chat_tag        "{gold}[VIP]{default}"
premium_trial_chat_tag  "{aqua}[Guest]{default}"
```

Members will have a gold `[VIP]` tag. Players on a free or points-bought trial will have an aqua `[Guest]` tag. Change the text inside the brackets to whatever suits your server; the brackets are optional too. This only changes how the tag looks, and does not change anyone's access or membership.

Colours include `{green}`, `{gold}`, `{red}`, `{aqua}` and `{purple}`. You can mix colours, for example `"{gold}[{purple}Supporter{gold}]{default}"`. Keep `{default}` at the end so the tag colour stops there. Keep tags short so there is room for the player's name and message.

Players can normally choose team colour, gold or plain in their preferences. To keep everyone using your chosen colours:

```cfg
premium_tag_styles "0"
```

Use `"7"` to allow all three personal styles again. These choices keep your configured tag wording.

Run `exec sourcemod/premium_rebuilt.cfg` in the server console after saving. The next normal chat message uses the new settings; players do not need an activation command. If you previously saved settings from the admin menu, update any matching setting in its saved block at the end of the config as well: that block runs last. In particular, an old saved `premium_tag_styles` value can override the one in section 11.

`premium_tag` controls the plugin's own notices. Use `premium_chat_tag` and `premium_trial_chat_tag` for tags beside player names. These text values are edited in the config; the in-game numeric settings menu does not edit their wording.

## Applying the main file

From server console or as an admin with full access:

```text
exec sourcemod/premium_rebuilt.cfg
sm_prem_reload
```

Editing the file alone does not execute it. `sm_prem_reload` updates currently live cvars and membership; it does not read the cfg file. Spawn-only resources wait until respawn, not each reload.

Before changing database/backend settings, read [Storage/recovery](Storage-and-Recovery.md) and settle pending purchases. Keep customised values when upgrading instead of blindly copying new defaults.

[All setting defaults](Configuration-Reference.md) · [Home](Home.md)

See [RC21 additions](RC21-Additions.md) for the building overview, spawn effects, gifts, specials and team utility limits.
