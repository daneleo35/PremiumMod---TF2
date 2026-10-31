# Points and the feature shop

Players earn points by playing, then spend them on extra features. You choose how often they earn points, how much things cost and how long purchases last. This uses in-game points; it does not take real-money payments.

## Enable it

Points and bought trials are **off by default**. Put these values in `tf/cfg/sourcemod/premium_rebuilt.cfg`:

```cfg
premium_points_enable "1"
premium_points_earning "1"
premium_points_interval_minutes "10"
premium_points_per_interval "5"
premium_points_shop "1"
premium_points_rentals "1"
premium_points_trials "1"
premium_points_trial_limit "3"
```

This awards five points per ten eligible minutes and permits three bought trials per account over its lifetime. Set `premium_trial "0"` separately if free trials should be disabled. Apply the main file as described in [Configuration](Configuration.md).

Players type `!premium_points` to check their balance and `!premium_shop` to browse. Short `!points` and `!shop` commands work too, unless another plugin already uses those names. Players see a confirmation before they buy. The purchase may take a moment to save before the feature becomes available.

## Earning rules

| Setting | Default | Meaning |
| --- | --- | --- |
| `premium_points_interval_minutes` | 10 | Eligible minutes per award |
| `premium_points_per_interval` | 5 | Points per completed interval |
| `premium_points_cap` | 1000000 | Cap on new credits; lowering it preserves existing balances |
| `premium_points_min_players` | 1 | Required counted humans |
| `premium_points_count_spectators` | 0 | Count spectator/unassigned humans |
| `premium_points_count_dead` | 1 | Count dead humans on playing teams |
| `premium_points_afk_seconds` | 0 | Input inactivity cutoff; zero disables the filter |

Bots never earn points. If you enable the idle check, movement, firing and jumping count as activity. Offline time does not earn anything. Players stop building up more earning time when their wallet is full. Progress is saved when they leave normally or the map changes; a server crash can lose some unfinished progress toward the next award.

Award, purchase and expiry notices have separate switches. A player's quiet preference can suppress optional notices.

## Configure offers

Edit `tf/addons/sourcemod/configs/premium_shop.cfg`. Example complete file:

```text
"PremiumShop"
{
    "Items"
    {
        "glow_hour"
        {
            "enabled"  "1"
            "name"     "Glow for an hour"
            "feature"  "glow"
            "price"    "10"
            "duration" "1"
            "unit"     "hours"
        }
        "trial_day"
        {
            "enabled"  "1"
            "name"     "Premium for a day"
            "feature"  "trial"
            "price"    "100"
            "duration" "1"
            "unit"     "days"
        }
    }
}
```

Then run `sm_premium_shop_reload`. Players with an old shop menu open must reopen it and confirm the new price. A purchase already accepted keeps the price and time the player agreed to.

You can copy an offer to add another price or length. Give each one a different name, such as `glow_hour` or `glow_day`: use lowercase letters, numbers and underscores, up to 30 characters. You can have up to 64 enabled offers. Use whole numbers for price and duration, a price from 1 to 1,000,000, and a length no longer than one year. Units must be exactly `minutes`, `hours` or `days`. The `feature` value is a name from the [feature list](Features.md), or `trial` for the full Premium package.

The supplied catalog contains glow: 10 points/hour; airblast: 10 points/hour; ammo: 50 points/day; Premium trial: 30 points/hour. Category switches still control availability.

## Rental versus trial

| Purchase | Grants | Tag | Limit/renewal |
| --- | --- | --- | --- |
| Individual feature | That feature only | None | Renewal can extend the remaining rental |
| `trial` | Full Premium package you have enabled | Trial tag | Shared lifetime purchase limit; cannot stack active Premium/trials |

`premium_points_trial_limit 0` means unlimited lifetime purchases, not an unlimited-duration trial. All trial offers share the counter. It is separate from the free-trial record; `sm_resettrial` does not reset it.

With `premium_points_renewals 1`, buying the same feature again adds time to what is left. `premium_points_max_rental_days` limits how much time they can have left, starting at 365 days. Time keeps counting offline, even if you temporarily turn the feature off. There are no automatic refunds. Players still type `!glow` to turn glow on, and starting ammo/metal wait until their next spawn.

Players cannot buy something they already get free or through Premium. They also cannot buy a feature their current class, map, team or server settings would prevent them from using. Later changes to your rules can still stop a feature while its purchased time runs down.

## Independent switches

| Switch set to zero | Effect |
| --- | --- |
| `premium_points_enable` | Turns points and bought features off; their time still runs down |
| `premium_points_earning` | Stops earning more play time; progress already saved can still be paid |
| `premium_points_shop` | Stops new purchases; existing bought access remains |
| `premium_points_rentals` | Turns bought individual features off |
| `premium_points_trials` | Stops trial purchases and turns existing bought trials off |

A purchase already accepted may finish saving after you change a switch. If the server restarts while points are off, it can wait until you turn points back on. Turning the system off does not cancel accepted purchases or give players extra time.

## Administration and storage

```text
sm_premium_points_adjust "Dane" 100
sm_premium_points_adjust "STEAM_1:0:123456" -25
sm_premium_shop_reload
```

The first example adds 100 points; the second takes away 25. Use a name or Steam ID. You can add or remove up to 1,000,000 per command. This does not cancel purchases or reset how many trials someone has bought. Read [Backing up player data](Storage-and-Recovery.md) before moving or restoring the database.

[Commands](Commands.md) · [Access and trials](Access-and-Trials.md) · [Home](Home.md)
