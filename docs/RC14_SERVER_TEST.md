# RC14: in-game player, shop and admin menus

Player entry point: `!premium`. Admin entry point: `!premium_admin` (root by default, normal command overrides apply). Install the new binary and keep the existing translations/configs. No new runtime dependency or database schema is introduced.

## Changes

- Short home menu with live membership/trial status and points balance.
- Separate bonuses, feature details, appearance, wallet/purchases, actions and personal options.
- Colour/FOV selectors, free-trial confirmation and clearer unavailable-feature information.
- Shop categories and offer details with price, duration, balance afterwards or refusal reason.
- Back/cancel navigation, human-readable durations and preserved accepted-price/receipt safeguards.
- Admin player picker, membership/free-trial changes, individual/public grants and points adjustments.
- Preset/reload controls and grouped server settings: 95 listed, 91 numeric editors.
- Explicit live-apply confirmations and explicit persistence to a marked config block with backup.
- Serial-bound targets and rechecked admin/individual command permissions; no server-console impersonation.

## Acceptance sequence

1. Open `!premium` as an ordinary player, member and free/purchased trial user. Verify titles and remaining time, points off/loading/ready states and that only authorised admins see controls.
2. Walk every submenu and Back/Exit path. Toggle appearance and preferences, select FOV/colour, and try features without access. Confirm no automatic unlocks or refills.
3. Open each shop category, an affordable offer, an unaffordable one and one already supplied by another source. Verify readable refusal reasons and correct balance projection. Cancel/back must never charge.
4. Reload the catalog with an offer/confirmation open; old quotes must be rejected. Confirm one purchase and rapidly try another; existing purchase locking/receipts must prevent double debit. Check wallet expiry and balances after completion.
5. Cancel a free-trial confirmation, then claim one. Repeated claims and disabled trials must remain blocked.
6. From the admin picker, confirm a membership/grant/points action. Disconnect the target before confirming, then fill the slot with another player: no action may affect the replacement. Remove the actor's permission while a menu is open and retry. Test target immunity and command-specific overrides.
7. Change a numeric setting and confirm. Change it externally before confirming another value: the stale choice must be refused. Test both bounds, booleans and restoration of defaults.
8. Back up the main cfg, Save from the menu, restart/change map and verify the final saved values. Confirm original comments/text remain, the managed block occurs once and `.menu-bak` contains the previous file. Verify saving again replaces the block. Try a read-only destination and malformed managed markers: the original cfg must remain recoverable.
9. Queue/apply/cancel presets and reload each companion file through the admin menu. Verify command-specific denies still apply and live changes have the same effect as commands.

The SourcePawn UI harness executes production ID parsing, target/permission guards, stale numeric quote checks, config-block parsing and duration formatting. Menu drawing/input and native file replacement still require a TF2/SourceMod host; these are separate from policy coverage.
