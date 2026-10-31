# Backing up player data

Premium saves membership, trials, points and purchases so they survive normal restarts. Keep backups before upgrading, moving the server or changing its database.

## Do I need to set up a database?

For a normal install, no. Leave `premium_use_mysql "0"` and the plugin uses SourceMod's local database. It creates the parts it needs automatically.

If you want to use a separate database, set up an entry in SourceMod's `databases.cfg`, then choose it with `premium_use_mysql "1"` and `premium_database`. The [example file](https://github.com/daneleo35/PremiumMod---TF2/blob/main/configs/premium-databases.example.txt) shows the layout. Keep passwords private. MySQL points tables must use InnoDB.

## What should I back up?

Stop the server first so nothing changes halfway through the backup. Keep these together:

- The database used by Premium.
- `addons/sourcemod/data/premium_points_outbox.cfg`, plus its `.new` and `.bak` files if present. These hold purchases and points changes that are still waiting to finish saving.
- Your Premium configuration files, especially `premium_features.cfg`, which stores free player grants.
- SourceMod's ClientPrefs data if you want to keep players' saved display options.

The database and waiting-purchase files belong to the same backup. Restoring only one can cause problems. Saved teleport locations are temporary and are not included.

## If the database or disk stops working

The points system pauses instead of pretending a purchase succeeded. It keeps waiting changes and tries again after 30 seconds. A damaged waiting-purchase file also stops points from running, without overwriting that file.

Keep the files and error logs. Fix the disk/database problem before restarting points. Do not delete the waiting file or purchase records just to make an error disappear: they are needed to check which purchases have already been paid for.

Normal disconnects and map changes save progress toward the next points award. A hard server crash can lose some unfinished progress. Trial and rental time keeps counting while the server or player is offline.

## Moving to another database

Let purchases and points changes finish saving first. Take a backup, then move the data and update the connection settings.

The plugin does not move balances to a new database for you. If waiting purchases belong to the old database, it keeps them and blocks new ones until you reconnect the old database and let them finish. Keep server clocks correct if several servers share a database.

## Useful admin commands

| Command | What it does |
| --- | --- |
| `sm_premium_diagnostics` | Shows whether Premium and its storage are ready |
| `sm_premium_database_create` | Reconnects and checks/creates the required tables; does not erase player data |
| `sm_prem_reload` | Refreshes online membership and effects; does not read your cfg file |
| `sm_premium_import_legacy [offset]` | Copies up to 100 memberships from the old `premium` table in the connected database |

For a legacy import, back up first and follow the command's next-offset message until it finishes. It copies memberships, not old settings or preferences.

## For developers: how purchases are protected

The economy uses `premium_v3_wallets`, `premium_v3_rentals` and `premium_v3_receipts`. Each request has a unique receipt. Its points change and feature grant are saved together, so repeating a saved request does not charge or reward twice. Refused requests are recorded too.

Do not edit request tokens, delete receipts or change table engines. The automated tests check real SQLite purchase/retry behaviour. A real MySQL server and interrupted SourceMod disk/database operations still need live testing; see the [developer guide](Development-and-Testing.md).

[Installation](Installation.md) · [Points](Points-and-Shop.md) · [Home](Home.md)
