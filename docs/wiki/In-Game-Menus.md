# Using the in-game menus

Type **`!premium`** in TF2 chat. You can use the plugin through the numbered menus without remembering the individual commands. Press the number beside an option; use Back to return or Exit to close it.

## Your home screen

The top shows whether you have Premium, a trial or individual features, followed by your points balance. Trial time is shown when a trial is active.

```text
Premium
Premium member
Your points: 125

1. My bonuses
2. My unlocked features
3. Appearance
4. Points shop & my purchases
5. Actions & free trial
6. Personal settings
7. Admin controls       (admins only)
```

The exact text changes with your account and the server's settings.

## What each screen does

| Screen | Use it for |
| --- | --- |
| My bonuses | See actual numbers for your current class, ammo and buildings |
| My unlocked features | Check each feature, whether you have it, and whether your current class/map/team can use it |
| Appearance | Toggle glow/dark tint, choose a colour or FOV, and open personal style settings |
| Points & purchases | Check your balance, remaining purchase time and trial-purchase usage |
| Actions & free trial | Swap teams, check a blocked swap, save/load a location, or start a free trial |
| Personal settings | Quiet mode, tag/style choices, glow style, particle visibility, reminders and building tooltip |

Appearance choices still require access to the feature. A free trial has a confirmation before starting its timer. Normal chat tags remain automatic.

## Buying something

Open the shop and choose Cosmetics, Gameplay features, Engineer features or Premium trials. Each offer shows its price and length. Choose it to see the full details.

The next screen shows the cost, duration, your balance and whether you are buying one feature or full Premium trial access. If you can buy it, it also shows what your balance will be afterwards. Confirm to buy or go back without spending anything.

If you cannot buy an offer, the reason appears on that screen. For example, you might already have it, need another class or need more points. An open old offer cannot charge you a changed price after the owner reloads the shop: you must choose the offer again.

Purchases can take a moment to save. Your receipt confirms when the purchase is ready. **Time counts while you are offline.** Check Points & purchases for time remaining; Refresh updates what is shown.

## Admin controls

Admins with permission see Admin controls on the home screen. **`!premium_admin`** opens it directly. It follows SourceMod permissions and target immunity; opening a menu does not bypass a command restriction.

| Admin screen | What you can do |
| --- | --- |
| Manage a player | Give/remove saved membership, give/reset a free trial, change individual free-feature grants, or add/remove points |
| Free features for everyone | Give/remove each public feature grant |
| Server settings | Browse grouped settings, choose numeric values and save them |
| Balance presets | Queue Cosmetic/Balanced/Fun for the next round, apply now, or cancel a queue |
| Reload files / check status | Reload grants, offers or class/map rules; refresh membership; view diagnostics |

Player changes use a selected connected player. The confirmation is checked again when you apply it. If the player left, a new player who takes their slot does not receive the old change. Offline accounts and custom points amounts remain available through the [admin commands](Commands.md).

Removing a grant changes only that source. A player may still get the feature through membership, another grant or a rental.

## Changing and saving server settings

Open Admin controls → Server settings → a category → a setting. Choose a new value, then confirm **Apply now**. The change takes effect immediately. The menus keep values within the plugin's allowed range.

Choose **SAVE current settings to config** to keep current numeric values across restarts. This saves a clearly marked block at the end of `cfg/sourcemod/premium_rebuilt.cfg`. It runs after earlier settings and preset lines. The old file is kept as `premium_rebuilt.cfg.menu-bak`.

The original config text and comments are kept. Later menu saves replace the menu's own block rather than adding another copy. Once saved, edit that final block or save through the menu again when changing those values; an earlier line with the same name is overridden by the final block. To return entirely to the original file values, remove the complete marked menu block and execute your config again.

If another setting changes while a confirmation is open, review the new value before applying/saving. Immediate changes can affect current bonuses or remove extra buildings when limits are reduced. Save captures the values that are live now, not a preset still waiting for the next round.

There are **105 numeric settings** available to edit. Membership flags and chat-tag text remain file edits. Storage connection settings also remain file edits; read [Backups and player data](Storage-and-Recovery.md) before changing them.

[Commands](Commands.md) · [Configuration](Configuration.md) · [Home](Home.md)

See [RC21 additions](RC21-Additions.md) for the building overview, spawn effects, gifts, specials and team utility limits.
