# Player guide

Start with **`!premium`** in normal game chat. Standard SourceMod chat triggers apply; console forms begin with `sm_`.

## Find out what you can use

- `!premium_status` — membership/trial status.
- `!premium_features` — your feature access, including independent grants/rentals.
- `!premium_benefits` — configured bonuses for your current class/team and current held resources.
- `!premium_options` — saved personal preferences.

An access grant can exist while a feature is disabled by the server's current balance/map/class settings. The server owner controls availability and amounts.

## Membership tags

Premium/Trial tags apply **automatically to normal chat** for qualifying members/trials. You do not need to run a command first. Free individual grants and individual rentals do not add a membership tag.

Use `!premium_options` to hide your tag or select an owner-approved style. `sm_premium_chat` only prints diagnostic information; it never activates a tag or membership.

## Cosmetics and preferences

```text
!glow
!glow on
!glow off
!colorme 255 100 100
!colorme reset
!dark
!premium_fov 100
!premium_fov reset
```

These require their own access and enable settings. Turning glow off always allows cleanup. Glow is hidden while the owner is cloaked/disguised. RGB tint and black tint do not make players invisible.

Preferences include quiet mode, tag visibility/style, glow style, Premium-particle visibility, trial reminders and the aimed building tooltip. Quiet mode hides optional adverts/reward/effect notices; it does not hide explicit errors or all purchase feedback. Particle visibility controls Premium-owned effects, not every particle in TF2.

## Pyro controls

**Primary-fire flight:** jump and hold primary fire with a supported flamethrower; WASD steers. Aim down to climb, level to hover and up to descend. Weapon fuel/state, movement restrictions and map rules still apply.

**Airblast jump:** right-click with a real, fuel-consuming airblast to receive recoil/lift. Backburner and other normal flamethrowers keep native airblast costs/cooldowns/reflection. Weapons without a valid native airblast do not gain one. Flight and airblast boost are separate features.

If testing fails, `!premium_flight` prints your weapon/state/access details to your console.

## Engineer controls

Use the normal build PDA and blueprint placement. No Premium spawning command is needed. The owner can allow up to three sentries, dispensers, entrances and exits, but there are **six shared total slots**. Each teleporter end occupies a slot.

Only extra sentries are restricted to level one. Ordinary primary sentries and utility buildings follow their normal upgrade rules. Aim at your own building for the small count tooltip. Read [Engineer buildings](Engineer-Buildings.md) for hauling, shared team budgets and the brief PDA pause near friendly wrench swings.

Extra metal is a spawn reserve/capacity bonus, not an automatic refill. The ammo multiplier changes reserve ammunition, including fuel, not magazine size.

## Team swap and location tools

```text
!premium_swapstatus
!swapteam
!save_loc
!load_loc
```

Swapping obeys round, team-balance and cooldown rules. Saved locations are tied to your current connection, map, life and team. Teleport destinations must be safe; objective carriers and blocked movement states cannot use them.

## Trials and points

`!trial` requests your one-time free trial when the owner enables it. The original expiry keeps counting offline.

If the optional economy is enabled, `!premium_points` shows your balance and `!premium_shop` opens the shop. `!points` / `!shop` may also be available. Purchases require confirmation and only activate after the purchase has saved. Read the offer: a feature rental gives only that feature; a **Premium trial** supplies the Premium package you have enabled and Trial tag. All expiry counts offline time.

[Complete commands](Commands.md) · [Home](Home.md)
