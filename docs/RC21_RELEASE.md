# RC21 additions

Added the six requested features plus points gifting:

- Owner-only building overview showing health, level and sapper status, with a refresh button.
- Short member spawn effects with two owner-approved styles, duration and simultaneous-effect budget.
- Feature rental gifts: sender pays, recipient gets only the selected feature; no membership or tag.
- Scheduled percentage discounts and offer availability windows for owner-controlled rotations.
- Optional purchase pause for movement features disabled by current map rules.
- Separate RED/BLU extra-dispenser, entrance and exit caps, preserving ordinary buildings and six total slots per Engineer.
- Points gifting with per-transfer and persistent UTC daily limits, recipient capacity checks and atomic updates to both wallets.

New commands: `!premium_buildings`, `!premium_gift`, `!premium_giftpoints`, and `!premium_giftpoints "Player name" 25`. They are also available through the player menus.

Spawn effects, rental gifting, points gifting and specials default off. Building overview and map purchase pause default on. Utility team caps default to -1 (unlimited); zero disables extras. All fourteen new numeric controls are also in the admin settings menu, now with 105 numeric editors.

Existing shop files and database tables remain compatible. New accepted gift/transfer intents preserve the beneficiary, quoted amount and relevant limits in the existing durable outbox. Receipts prevent replaying debits, grants or credits; declines are recorded too. Pending accepted intents retain their agreed terms. No third-party runtime files or custom engine binaries are added.

Upgrade from RC20: replace `addons/sourcemod/plugins/premium.smx`, add the new config settings you want while retaining existing values, and reload the plugin. No translation or database replacement is needed. Preserve the outbox and receipt tables, and settle pending gifting operations before downgrading.

See [the complete setup guide](wiki/RC21-Additions.md) for every switch, menu, command, schedule field and limit.

Validation: compiled production helpers cover team utility caps, spawn ownership/deadlines, sale windows/pricing, quote invalidation, map purchase gates and durable intent validation. Real SQLite integration covers gift debits/grants, renewal limits, replay/decline handling, concurrent transfers, recipient caps, daily boundaries and rollback after debit. MySQL query structure is checked; a live TF2 or MySQL server is not exercised by these checks.
