# Buildings, spawn effects, gifts and shop specials

All settings below go in `cfg/sourcemod/premium_rebuilt.cfg`. Save the file, then run `exec sourcemod/premium_rebuilt.cfg` in the server console. If you have saved settings through the admin menu, check the saved block at the end of the file too: it runs last.

## Building overview

Open `!premium` as Engineer and choose **My buildings**, or type `!premium_buildings`. It lists your placed sentries, dispensers and teleporter ends with current health, level and whether they are sapped. Choose **Refresh status** to update it. This is an owner-only snapshot, not a screen overlay; it cannot control or destroy buildings.

```cfg
premium_building_overview "1"
```

This menu is available to ordinary Engineers too. Set it to `0` to hide/disable it. Your six shared building slots and per-type limits still apply.

## Member spawn effects

```cfg
premium_spawn_effect "1"
premium_spawn_effect_style "0"
premium_spawn_effect_seconds "1.5"
premium_spawn_effect_budget "8"
```

Effects start **off**. Style `0` is a short team-coloured teleport shimmer; `1` is a team-coloured critical sparkle. The owner selects the style. The budget limits how many can be active at once, and the duration can be 0.25–5 seconds. These use built-in TF2 particles, with no extra sound or downloads.

Only members and active full trials get this effect. An individual feature grant or rental does not grant it. Effects stop on death, disguise/cloak, team/class changes, round end, access loss, map change and unload. Players who hide Premium particle effects do not see them. Resupply and inventory changes do not trigger another spawn effect.

## Gift a feature rental

Enable points and feature gifting:

```cfg
premium_points_enable "1"
premium_points_shop "1"
premium_points_rentals "1"
premium_points_gifts "1"
```

Open the points shop and choose **Gift a feature rental**, or type `!premium_gift`. Choose a connected player, then an offer. The confirmation names the recipient, price and length. Your points pay for the recipient's rental; it does not give you the feature or give them Premium membership/a tag. Premium trials cannot be gifted.

The recipient must have loaded their account and be able to use the feature with their current class, team, permissions, map and server settings. Existing free/member access prevents duplicate purchases. Renewal and maximum-duration settings apply to the recipient's rental. If they disconnect or the price changes before confirmation, choose again. Once accepted for saving, a gift stays tied to that Steam ID even if either player leaves. Offline time counts as usual.

Gifts use the same saved transaction queue and receipts as ordinary purchases. Failed/declined purchases do not debit points, and retrying the same saved gift cannot charge or grant it twice.

## Gift points

```cfg
premium_points_enable "1"
premium_points_transfers "1"
premium_points_transfer_max "1000"
premium_points_transfer_daily "5000"
```

Open **Points & my purchases → Gift points**, or type `!premium_giftpoints`. Pick a connected player and amount, then confirm. For an exact amount, use:

```text
!premium_giftpoints "Player name" 25
```

You can only give points you already have. The minimum is one point. The per-gift maximum defaults to 1,000; the daily maximum defaults to 5,000 and is recorded against the sender's Steam ID. Reconnecting/reloading does not reset it. Days change at midnight **UTC**, including when the server uses another timezone. Set the daily limit to `0` for unlimited daily sending. The recipient's normal points cap still applies.

Both balances are saved together. If the gift would exceed a limit or there are not enough points when it settles, neither balance changes. Transfers grant no features or membership. They have their own switch, so they can remain available while shop purchases are disabled. Points earning is independent too.

## Scheduled specials and rotating offers

```cfg
premium_shop_specials "1"
```

Edit offers in `addons/sourcemod/configs/premium_shop.cfg`. Add any of these optional keys **inside an existing offer**:

```text
"available_from"   "0"
"available_until"  "0"
"sale_from"        "0"
"sale_until"       "0"
"discount_percent" "0"
```

Times are Unix timestamps in UTC: whole seconds since 1 January 1970. `0` means no boundary. An offer is available at its start time and stops at its end time. A discount must have a positive `sale_until`; change the percentage from `0` to 1–99. Discounts round up to whole points, with a minimum price of one point. The offer's ordinary `price` is still its base price.

For example, these sale times mean **1–8 December 2026, midnight UTC**, with 25% off:

```text
"sale_from"        "1796083200"
"sale_until"       "1796688000"
"discount_percent" "25"
```

Rotate offers by making separate offers with different availability windows. These are fixed schedules, not an automatic daily rotation. Reload with `sm_premium_shop_reload`. Windows and prices are then evaluated automatically as time passes; no timed reload is needed. Outside an availability window, an offer can still be inspected but cannot be purchased or gifted.

Both self-purchases and feature gifts use specials. A changed price invalidates an open confirmation; the player chooses again. A purchase already accepted into the saved queue keeps its agreed price and duration. Set `premium_shop_specials "0"` to ignore schedules/discounts and use base offers again. Existing shop files work unchanged.

## Pause purchases on restricted maps

```cfg
premium_shop_pause_unavailable "1"
```

On by default. Individual flight/airblast purchases and gifts are blocked when map rules disable flight. Saved-location teleport purchases/gifts are blocked when the map disables teleport. The shop explains why. Full Premium trials are bundles and are still available; their perks follow the map rules.

Set this to `0` if you want to allow buying those rentals for later maps. This does not turn the perk on in a restricted map. Class, team, permission and server-feature checks still apply. Existing rentals keep counting down while unavailable; this setting pauses **new purchases**, not rental clocks, and does not issue refunds.

## Team limits for extra utility buildings

```cfg
premium_team_extra_dispensers "4"
premium_team_extra_entrances "4"
premium_team_extra_exits "4"
```

Each limit is separate for RED and BLU. Only extras count: each Engineer's ordinary dispenser, entrance and exit are excluded. `-1` means unlimited team extras and is the default; `0` prevents extras of that kind. Limits range up to 128. Your existing per-player limits and six shared slots still apply.

The normal PDA and build binds respect the limits. Carrying/redeploying an existing building is allowed at capacity. If someone takes the last team slot while a blueprint is open, placement is blocked before metal is spent; a final placement check also catches races. Lowering a team cap removes the newest tracked extras first, leaving ordinary buildings alone. Teleporter entrances and exits have separate budgets; these caps do not create numbered teleporter routes.

## Upgrading from RC20

Replace `addons/sourcemod/plugins/premium.smx`. Add only the new settings you want from the supplied config, keeping your existing values. Existing shop files, translations, memberships, points, rentals and database tables work without replacement. Do not delete the points outbox or receipt tables. The new gifting operations extend the existing journal format; settle its queue before downgrading to an older plugin that does not understand gifts.

[Commands](Commands.md) · [Configuration](Configuration.md) · [Points](Points-and-Shop.md) · [Home](Home.md)
