# Troubleshooting

Start with `sm plugins list`, `sm_premium_diagnostics` and the latest SourceMod error log. Confirm version `3.0.0-rc14` and remove older duplicate Premium binaries. Use an ordinary authenticated test account as well as an administrator: root/flag access can hide permission and shop problems.

## Plugin will not load

Install TF2Attributes and its matching gamedata separately, then check its own loading errors. The build warns about missing `morecolors.inc` / `tf2attributes.inc`; these headers are for compilation. More Colors has no separate runtime plugin to load. Use the [installation guide](Installation.md) and current dependency download links there.

For a translation parsing error, replace `addons/sourcemod/translations/premium_rebuilt.phrases.txt` with the file from this same release, keeping its braces/quotes intact. Do not mix the old legacy translation file with the rewrite. Reload translations/plugin or restart after correction and check the log again.

## No tag in ordinary chat

Tags are automatic for members and active full trials. `sm_premium_chat` is a **diagnostic command**, not an activation step.

1. Check `!premium_status` and actual membership; a free feature grant or individual rental does not give a tag.
2. Check `premium_enable`, `premium_chat_tags`, `premium_chat_tag` and `premium_trial_chat_tag`.
3. In `!premium_options`, enable your tag and select a permitted style.
4. Run `sm_premium_chat`, send ordinary all/team chat, then collect the output again. Check the processor/format/counter details.
5. If another chat processor is installed later, include its name/version when reporting interference.

`premium_tag` changes the plugin's system-message prefix, not the player's membership tag.

## Ammo, fuel, balls or jars look wrong

Use `sm_premium_weapons` and `!premium_benefits` after a normal respawn. Check the `ammo` grant, global multiplier and class override. The old `premium_clip_multiplier` name is a synchronized compatibility alias for **reserve ammo**, not magazine size. Pyro fuel and Heavy ammo are included; item-specific native modifiers also apply.

Spawn bonuses do not continually refill depleted ammo. Scout balls and supported jars recharge **one at a time** using native regeneration. Their configured reserve of one is intentionally stock-like; raise it if more are wanted. Record the weapon name/item definition and held/max reserve before spawn, after use and after a full recharge cycle.

## Pyro flight or airblast boost fails

Check `flight`/`airblast` access, Pyro class, fuel, `premium_pyrofly_enabled`/`premium_airblast_enabled`, class rules and map bans. Run `sm_premium_flight` while reproducing. Jump and hold primary fire for flight; WASD steers, looking down climbs. Backburner is supported. Right-click boost requires a real native fuel-consuming airblast. Phlog does not gain airblast.

A flight map ban blocks both movements. State/fuel/round restrictions can stop movement. Report whether the effect fails immediately, only after several seconds, or after resupply/weapon changes, with the diagnostic output.

## Cannot place or upgrade extra buildings

Count **all** owned buildings: the fixed shared limit is six, and both teleporter ends count separately. Check per-type access/limits and the team extra-sentry cap. Extra sentries are deliberately level one; utilities have normal upgrades. A carried existing building can be redeployed at capacity.

Use the ordinary PDA. Friendly wrench work can briefly pause its temporary building-count adjustment and show “Already Built.” Read [Engineer buildings](Engineer-Buildings.md) for pairing and native restrictions. For tooltip issues, check owner/aim/range, `premium_engineer_hud`, `premium_sentry_labels` and your tooltip preference.

## Points/shop unavailable or not awarding

Points and points-bought trials are off by default. Check the separate master, earning, shop, rental and bought-trial switches, then database readiness. Count eligibility, minimum humans, AFK/dead/spectator rules, full-wallet cap and a complete eligible interval. Quiet mode can hide the notice while the balance still changes.

The shop refuses purchases on accounts already receiving that benefit, including root/flag members. Test with a non-Premium account and use an admin adjustment to fund it. Check class/map/team availability, enabled offers, prices and the lifetime trial counter. Reloading the catalog invalidates open quotes; reopen the menu.

For persistence errors, preserve the SQL database and outbox before attempting recovery. Follow [Storage and recovery](Storage-and-Recovery.md); deleting receipts or the outbox can destroy the evidence needed for safe replay.

## Edits have no effect

Main cfg: run `exec sourcemod/premium_rebuilt.cfg`, then `sm_prem_reload`. Companion files have separate reload commands. A preset executed at the end can override earlier main settings; class overrides and map bans can override globals. Resource bonuses apply on spawn. `!premium_features` shows access, while `!premium_benefits` helps inspect actual current effects.

## Reporting a reproducible issue

Include version, server OS, SourceMod/TF2Attributes versions, exact weapon/class/map, relevant config snippets, diagnostics and a short sequence with expected versus actual behaviour. Attach the relevant error-log excerpt. Remove passwords and private database credentials. Use the repository [issue tracker](https://github.com/daneleo35/PremiumMod---TF2/issues).

[Commands](Commands.md) · [Server testing](Development-and-Testing.md) · [Home](Home.md)
