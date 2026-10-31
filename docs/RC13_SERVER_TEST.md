# RC13: optional play-time points, feature rentals and purchased trials

The entire economy is **off by default**. It uses the existing asynchronous SQLite/MySQL connection, separate wallet/rental/receipt tables, and a local pending-operation journal. No real-money purchases, engine DLL/SO, additional extension or new gamedata is involved.

## Install and enable

Replace the plugin and translations; install the new `addons/sourcemod/configs/premium_shop.cfg`. Keep your existing feature/rules/balance configs. Merge the points settings you want into `cfg/sourcemod/premium_rebuilt.cfg`, then change map/reload and check version `3.0.0-rc13`.

```cfg
premium_enable "1"
premium_points_enable "1"
premium_points_earning "1"
premium_points_interval_minutes "10"
premium_points_per_interval "5"
premium_points_shop "1"
premium_points_rentals "1"
premium_points_trials "1"
premium_points_trial_limit "3"

// Optional: turn off the separate one-time FREE trial.
premium_trial "0"
```

This example awards five points per ten eligible minutes and allows at most three successful points-bought trials per Steam account, across every trial offer. The distributed master economy and points-trial switches default to `0`, so enabling only the economy does not automatically enable purchased trials.

## Independent controls

| Setting | Default | Meaning |
| --- | --- | --- |
| `premium_points_enable` | 0 | Master economy and purchased-access switch |
| `premium_points_earning` | 1 | Count new eligible earning time |
| `premium_points_interval_minutes` | 10 | Minutes per completed award, 1–1440 |
| `premium_points_per_interval` | 5 | Points per award, 1–1,000,000 |
| `premium_points_cap` | 1,000,000 | Cap on new earnings/positive credits; lowering does not confiscate balances |
| `premium_points_min_players` | 1 | Minimum connected humans on counted teams |
| `premium_points_count_spectators` | 0 | Include spectators/unassigned humans in earning/population |
| `premium_points_count_dead` | 1 | Dead players on counted teams keep earning; separate from population count |
| `premium_points_afk_seconds` | 0 | Optional idle cutoff based on movement/fire/jump input; zero disables it |
| `premium_points_shop` | 1 | Accept new purchases |
| `premium_points_rentals` | 1 | Honor purchased individual features |
| `premium_points_trials` | 0 | Allow/honor points-bought Premium trials |
| `premium_points_trial_limit` | 1 | Lifetime successful purchased trials per Steam ID; zero explicitly means unlimited |
| `premium_points_renewals` | 1 | Extend an existing individual rental; does not permit buying while any Premium/trial membership is active |
| `premium_points_max_rental_days` | 365 | Maximum remaining individual rental duration including extensions |
| `premium_points_earn_notice` | 1 | Private chat award notices after committed earnings |
| `premium_points_purchase_notice` | 1 | Private successful-purchase receipts |
| `premium_points_expiry_notice` | 1 | Private expiry notices |

The existing quiet preference suppresses optional earn/expiry notices. Explicit balance requests and purchase errors remain visible. A full wallet stops accumulating additional earning time. Bots never earn. No offline or long server-stall catch-up is awarded.

Turning earning off stops new play-time accrual; already banked time/pending transactions can still settle. Turning the shop off blocks new purchases while allowing current rentals/trials to run. Turning rentals or paid trials off suspends that access without deleting its saved deadline/counter. The master off switch suspends both. **Expiry is always wall-clock time and continues while offline or disabled.** Accepted transactions retain their confirmed terms and may complete after a switch is turned off; a restart while disabled preserves them until the economy is enabled again.

Balance presets do not overwrite these economy settings. Numeric feature bonuses, class/map/team restrictions and the six-building limit continue to apply to purchased access.

## Set prices and durations

Edit `addons/sourcemod/configs/premium_shop.cfg`, then run root command `sm_premium_shop_reload`. Each enabled offer has a unique lowercase ID, display name, feature key, positive price, duration and unit:

```text
"PremiumShop"
{
    "Items"
    {
        "glow_hour"
        {
            "enabled" "1"
            "name" "Glow for an hour"
            "feature" "glow"
            "price" "10"
            "duration" "1"
            "unit" "hours"
        }
        "trial_day"
        {
            "enabled" "1"
            "name" "Premium trial for a day"
            "feature" "trial"
            "price" "100"
            "duration" "1"
            "unit" "days"
        }
    }
}
```

Units are exactly `minutes`, `hours` or `days`, up to one year per offer. Up to 64 enabled offers are supported; multiple prices/durations can refer to the same feature. The distributed catalog includes hourly glow/airblast, daily reserve ammo and an hourly Premium trial as editable examples.

All 29 RC12 feature keys are supported; the special key `trial` purchases full configured Premium trial access. Individual feature rentals **do not** grant membership, a chat tag or unrelated features. They expire independently and can be renewed if enabled, extending from their existing deadline up to the configured remaining-time cap. `glow` still needs `!glow`; spawn-only ammo/health/metal/uber rewards wait for respawn, without a purchase/refill exploit.

A purchased trial **does** grant the full configured Premium feature set and the Trial chat tag until expiry. It uses a separate saved expiry and lifetime counter. It does not change admin flags, permanent membership, free-trial usage or free-trial deadlines. `premium_trial 0` can disable free trials while `premium_points_trials 1` permits paid-in-points trials. `resettrial` resets the free-trial record, not the purchased-trial counter. Increasing the configured lifetime limit can allow additional purchases; disabling/re-enabling/reconnecting cannot reset the counter. Buying another trial while any Premium/trial membership is active is refused to avoid wasting points.

The shop also refuses individual features already supplied by membership, a public/user grant or a permission override. Disabled/non-beneficial features, unsuitable classes and current map/team bans prevent purchases. The shop explains failed availability checks; later server rule changes can still suppress a purchased feature while its clock runs. No automatic refunds are implemented. Catalog errors disable new purchases, not existing rentals or earning. Existing menus/confirmation quotes are invalidated on catalog reload; accepted transactions use their saved terms.

## Commands

| Command | Access |
| --- | --- |
| `!premium_points` / `sm_premium_points` | Own balance and purchased-trial count |
| `!premium_shop` / `sm_premium_shop` | Browse and confirm a purchase |
| `!points`, `!shop` | Convenience aliases, registered only if unused at plugin startup |
| `sm_premium_shop_reload` | Root: reload offers and invalidate open quotes |
| `sm_premium_points_adjust <player\|Steam ID> <signed amount>` | Root: queue an additive adjustment, clamped at zero/cap; up to ±1,000,000 |

An admin adjustment is useful for testing with a separate non-Premium account. Root/flag/permanent members already have features and normally cannot buy them. Adjusting points does not revoke existing rentals or reset trial purchase counts. Offline Steam IDs are accepted; balances are keyed to the existing canonical Steam3 identity.

## Persistence and recovery

The optional tables are `premium_v3_wallets`, `premium_v3_rentals` and `premium_v3_receipts`; MySQL uses InnoDB. Do not change the table engines. Balance changes, rental/trial grants and their receipt are one transaction. A receipt is recorded for declined requests too, so replaying an old declined request cannot unexpectedly spend points after the user earns more. Retrying a committed request never repeats its debit, reward or duration extension.

`addons/sourcemod/data/premium_points_outbox.cfg` stores accepted pending operations with unique tokens. `.new` and `.bak` support interrupted replacement. The journal is written before settlement and cleaned after commit; if cleanup fails, replay is safe through the DB receipt. Invalid journals pause the economy without overwriting the damaged file. Preserve it and its backup, correct/recover it, then reload. Do not edit operation tokens, delete receipts, or restore only half the journal/database pair. Back up the database and points journal together.

Database errors pause the economy, retain queued intents and retry after 30 seconds; regular flag/free-feature access stays separate. The same backend must be restored to settle its pending operations. Before changing `premium_use_mysql` / `premium_database`, let queued purchases finish. A queue tied to an old backend is preserved and blocks new economy operations until that backend is restored and settled. Balances are not silently moved between unrelated databases.

Whole eligible seconds are saved on ordinary disconnect, map change and plugin unload. A hard crash can lose the currently unfinished, unjournaled play-time interval; committed points and journaled operations remain recoverable. Rental timestamps are calculated when their transaction attempt is dispatched using the server's clock; asynchronous DB delay/offline time counts. Synchronize clocks when sharing MySQL across servers.

## Server acceptance

1. Upgrade with the master at `0`: confirm no earning/shop/purchased access and normal RC12 grants/membership. Enable the example settings with free trials off. Check all optional tables initialise without errors.
2. On an ordinary authenticated non-Premium account, set interval to one minute/reward ten for testing. Verify no award before a full eligible minute, then one private chat message and ten committed points. Check another account's balance stays separate.
3. Test dead/spectator/population/AFK switches independently, plus earn/notice off, quiet mode and balance cap. Changing map/reconnecting should preserve banked whole seconds/points without offline catch-up. Restore your intended production rates afterwards.
4. Use root `sm_premium_points_adjust "Dane" 100` against the test account. Open the shop, inspect the quote, cancel once, then confirm glow. Verify one debit, one rental, no Premium tag/extra health/ammo, and that `!glow` works. Check spawn-only bonuses wait for respawn.
5. Configure a one-minute rental. Verify expiry removes only that purchase's access; a separate public/user grant keeps access. Test renewal on/off and maximum remaining duration. Buying a feature already supplied free/by membership must not charge.
6. Enable paid trials, set the lifetime limit to two, and use one-minute trial offers. Buy, verify the Trial tag/full configured perks, and wait for expiry. Buy a second; a third must be refused. Reconnect/change map/reload between attempts, test alternate trial offers and free `resettrial`: none should reset the purchased counter. Test `0` unlimited only if desired.
7. Open a confirmation, reload/change its price/duration, then confirm the stale menu: it must reject the old quote. Try rapid clicks and insufficient balance; no double charges or negative balances. Purchases must remain self-only and point adjustments root-only.
8. In a disposable server/database copy, test disconnect/reload during settlement, DB outage/recovery, non-writable data directory and restart with pending intents. Verify one receipt/debit/grant per token, preserved declined requests, correct partial-time progress and no operation replay into a different backend.
9. Disable earning/shop/rentals/paid trials/master separately while rentals exist. Confirm the documented distinction between blocking purchases, suspending access and retaining elapsed expiry. Re-enable before/after expiry and check that time is never extended automatically.
10. Repeat on a Linux/Windows server and, if used, real MySQL/InnoDB. Local tests execute production-generated SQL against SQLite and compiled SourcePawn policies; SourceMod filesystem/callback recovery, game rendering/prediction and real MySQL have not been run here.
