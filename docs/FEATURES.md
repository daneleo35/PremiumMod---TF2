# Original feature coverage â€” 3.0.0-rc10

Every command registered by the supplied original file is represented below, including the formerly commented-out glow command. Feature presence has been reviewed against the source; the table is not a claim that every historical forum symptom has been reproduced and passed on a TF2 server.

| Original feature | Rewrite implementation / deliberate policy |
| --- | --- |
| Premium flag and root access | `premium_flags` / `premium_access`; existing admin rights are preserved |
| MySQL membership | Async MySQL/SQLite, automatic schema, add/remove, normalized Steam IDs, bounded legacy import |
| Player trial | Atomic once-per-account claim, persisted deadline, reconnect/map/restart continuity |
| Administrator create/reset trial | Finite grant/reset using the same persistent ledger; no admin-flag mutations |
| `prem_reload` | Live effect/membership reconciliation; cvars also have change hooks |
| `premium_database_create` | Explicit reconnect and idempotent automatic schema verification |
| Health on spawn | SDKHooks max health plus one grant per life |
| Health on kill/assist | Living recipients, real enemy deaths, health cap, correct recipient feedback |
| Gravity | Configurable multiplier with owned-state restoration |
| Class speed / Heavy | Class speed attributes; game preserves minigun slows; no timer multiplication |
| Extra ammunition | Player primary/secondary reserve capacity and spawn supply, including clipless Pyro fuel; magazines stay native |
| Jarate / Sandman balls | Checked native grenade capacity; spawn reserves and one-at-a-time native recharge |
| Medigun charge | Checked medigun property and bounded minimum spawn charge |
| Engineer metal | Owned metal capacity (default 200, configurable to 2000) and initial spawn reserve only; no refill command or automatic top-up |
| FOV | Validated 75â€“120 or reset, feature-disable support and restoration |
| Default/custom colour | One final render state, checked RGB values, More Colors chat support |
| Dark | Toggleable full-alpha black tint; no automatic invisible player/Spy alpha |
| Glow | `!glow [on/off]`, four original particles or a two-particle team preset, tracked ownership and lifecycle cleanup |
| Team swap | Checked RED/BLU swap, configurable cooldown and team-size limit; spectator and MvM exclusions |
| Respawn | One serial-bound timer, real-death flags, life/team/mode/round/access revalidation; sudden death excluded |
| Dead Ringer detection | Actual death flag, with fake deaths excluded from rewards/respawn |
| Cloak duration | Finite configurable duration multiplier, separate Dead Ringer value; native activation fees, recharge and resistance duration |
| Bunnyhop / trimp | Held jump rearmed on the ground; bounded speed calculation without zero-length normalization; native airborne jump inputs preserved |
| Follow-up jump | Explicit optional post-trimp jump rather than the old impossible Unknown-class branch |
| Pyro flight | Post-movement flight for all flame variants including Backburner and Dragon Fury; gravity compensation, aim-controlled climb, WASD steering |
| Lit bow arrows | Checked compound-bow entity/property, including bow variants by classname |
| Save/load location | Session/life/map/team validity, hull check, objective restriction and cooldown |
| Advertisement / welcome / join message | Configurable, live scheduling, translated coloured chat; public announcement defaults off |
| Player chat tags | Configurable Premium/Trial name tags; original chat recipient list preserved |
| Extra sentries | Normal PDA/blueprint construction, maximum three total; one upgradeable primary and two level-one-only extras; native ownership and tracked cleanup |
| Weapon diagnostics | !premium_weapons reports reserve ammo pools, capacity hooks and ownership; !premium_flight reports firing/movement state |
| Feature display | `!premium`, a passive-benefits page, and access/countdown status |

`premium_extra_clip`, absolute speed cvars and alpha-zero invisibility are intentionally replaced by the policies above. rc6 replaces unlimited cloak with finite duration multipliers, makes metal spawn-only, and replaces sentry commands with native PDA placement. Editable balance profiles and RED/BLU gameplay controls are supplied. These are documented behavior changes rather than omitted feature categories. The original broken particle glow is now implemented. Live engine behavior, MySQL and other-plugin interactions remain part of server acceptance testing.

Balance controls: `docs/BALANCE.md`. Current ammunition checks: `docs/RC7_SERVER_TEST.md`; other engine checks: `docs/RC6_SERVER_TEST.md`.

rc7 replaces the magazine bonus with reserve ammunition as clarified during server testing. `premium_ammo_multiplier` is the new setting; `premium_clip_multiplier` remains an alias.

RC8 adds every selected control/presentation feature; see the 15-row table and acceptance sequence in `RC8_SERVER_TEST.md`. Primary sentries remain excluded from the shared budget; extra guns remain level 1. No new combat reward, metal refill, unlimited cloak or magazine boost is added.

RC9 adds configurable Pyro recoil jumps on completed native airblasts, with fuel/cooldown validation, impulse caps, class override and map-flight restrictions. See `RC9_SERVER_TEST.md`.

RC10 fixes native chat-key matching and replaces the large Engineer counter with an owner-only tooltip attached to the aimed-at turret. The standard health/upgrade panel remains native. See `RC10_SERVER_TEST.md`.

RC11 adds normal-PDA extra dispensers and teleporter ends with separate 1–3 caps, a fixed shared six-building limit, lifecycle cleanup and compact aimed building counts. No modified game binaries or additional native-call dependency is required. Native pairing/upgrade behaviour and the temporary PDA flag workaround require the live checks in `RC11_SERVER_TEST.md`.

RC12 adds independent public/Steam-ID grants for all 29 feature keys, explicit feature permission grants, a paged access menu and trial disable that also blocks existing trial entitlement. Membership and chat tags remain separate. See `RC12_SERVER_TEST.md`.

RC13 adds optional points earning, private reward chat, durable wallets, configurable feature rental offers and separately limited Premium trial purchases. All switches, time units, shop/root commands and recovery/live checks are in `RC13_SERVER_TEST.md`.
