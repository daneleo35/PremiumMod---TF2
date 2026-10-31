# TF2 server acceptance checklist

Release candidate 3.0.0-rc5 has compilation, policy/dependency/glow-controller VM, and SQLite integration coverage. The checks below require a real TF2 server and have **not** been reported as passed.

Use a private test server with the old plugin unloaded. Retain original logs/config/database backups. Start with the packaged defaults, then try your intended configuration and other plugins.

| Area | Verify |
| --- | --- |
| Startup | Premium, SDKHooks, SDKTools and TF2Attributes load; no gamedata/native/translation errors; SQLite tables appear automatically |
| Missing dependencies | With TF2Attributes absent, Premium logs its named dependency warning, download link and installation paths, then fails startup before applying effects; repeat with incompatible/unready TF2Attributes. More Colors is checked during compilation, not by looking for a server-side include file. |
| Slots | Premium players at 1, 31, 32, and higher supported slots can spawn, move, and run commands without bounds errors |
| Classes | All nine classes; wearable primary replacements; weapon stripping; class changes; resupply and loadout switching |
| Heavy | Ordinary/minigun firing and spin transitions; no speed override while disabled; timer/handle counts remain stable during a long session |
| Attributes | Weapon/item speed modifiers still operate; changing/turning off speed restores the original runtime value; test other plugins that own the same attributes |
| Health | Spawn bonus exactly once; correct max health; no stacking from inventory events/admin trial renewals; kill/assist reward caps and recipients |
| Weapons/ammo | Primary/secondary reserve capacity and spawn supply, including Pyro fuel and Heavy minigun; native magazine sizes, item penalties and energy/meter behavior; metal and throwables stay separate; no automatic refill |
| Throwables/Medics | Sandman, Wrap Assassin, supported Jarate variants; stock/variant mediguns; no invalid weapon access |
| Deaths | World damage, suicide, teamkill, domination/revenge and first blood, real Spy death, actual Dead Ringer feign |
| Respawn | Dead eligible player respawns once; no spectator, already-alive, Arena, MvM, sudden-death or round-end respawn; settings/team/access changes cancel the pending timer |
| Timer identity | Disconnect before respawn, immediately reuse that slot, and verify the new occupant is unaffected |
| Trials | One claim, two overlapping users, countdown correctness, reconnect/map/restart persistence, no new trial on reconnect, disable at expiry, admin finite grant/reset |
| Permissions | Root/admin flags remain byte-for-byte unchanged; database add/remove does not alter flags; SourceMod command and feature overrides work |
| Storage recovery | Missing config, unreachable MySQL, connection loss, slow SQL, retries, backend switch during a pending callback; flag members retain access |
| MySQL | Schema initialization, both INSERT IGNORE paths, REPLACE trial grant/reset, duplicate add, delete, remote persistence, concurrent claim across servers, legacy import rollback |
| Migration | Mixed Steam2/Steam3/Steam64 records, duplicates, malformed IDs, more than 100 old rows; old table remains untouched |
| Late load | Load the plugin with existing players; existing flags/memberships are recognized; cleanup on unload; unload Premium before changing TF2Attributes |
| Rendering/FOV | Default/custom/dark transitions; Spies not made globally invisible; FOV 75/120/reset; disabling each feature; locker/spawn interactions; another render plugin; More Colors tags in the prefix and translations render in chat and strip correctly in console replies |
| Glow | All four legacy particles visible; on/on does not duplicate; off removes every emitter; repeat toggles; death/respawn; cloak/disguise; team/class changes; expiry, feature/global disable, disconnect, map change and unload; partial creation/low-edict rollback; verify `sm_premium_diagnostics` emitter count returns to zero. Team style uses exactly two team-coloured emitters. |
| Teleport | Load before save, walls/occupied space, crouching/scaled players, cooldown, objective carriers, team/life/map changes, finished rounds |
| Movement | Bunnyhop/trimp, optional follow-up jump including zero horizontal speed, water/taunt/stun/charge; preserve normal Scout jump behavior; Pyro needs ammo; continuous flight between flame packets, no flight on airblast; frozen inputs must block boosts; compare intended server tick rates |
| Disable/unload | Every continuously applied perk stops and owned state restores; no old trial timer can regrant or remove membership |
| Soak | Observe handles, memory, tick time, and error logs with representative player counts and repeated class/death/reconnect/inventory activity |

If a check fails, capture plugin versions, the active config, the smallest reproduction, and the relevant SourceMod error log. Mark acceptance per actual server build rather than assuming compilation proves engine behavior.

Current ammunition correction: follow `RC7_SERVER_TEST.md` for reserve capacity, spawn supply and unchanged magazines. Follow `RC6_SERVER_TEST.md` for finite cloak/Dead Ringer duration, spawn-only metal, normal PDA sentries, hauling at cap, Backburner/other flame flight, and balance profiles. The rc5 command/refill/unlimited-cloak instructions are superseded. Continue to test sequential throwable recharge, normal primary/secondary reserve pools, chat privacy and dependency/storage behavior.

RC8: follow `RC8_SERVER_TEST.md` for counters/labels, competing team sentry slots and budget reduction, class/map overrides, queued profiles, saved options/cookie races, quiet/chat privacy, per-viewer particle suppression, bounded celebrations, reminders and the numeric benefits page. Native HUD/network/particle behavior and other-plugin compatibility still require live acceptance.

RC9: follow `RC9_SERVER_TEST.md` for real-airblast detection, ground takeoff, aiming recoil, native fuel costs, held-input behavior, additional cooldown, caps and class/map disable. Native weapon-frame ordering and prediction still require the target TF2 server.

RC10: follow `RC10_SERVER_TEST.md` for native chat-tag appearance and team/dead/spec recipient privacy, plus turret tooltip attachment, count/role changes, owner-only delivery, cleanup and high client slots. The large overlay is removed; native health/upgrade UI and gameplay must remain unchanged.

For the RC11 shared six-building limit, extra dispensers/teleporters, hauling and friendly wrench/PDA interaction, complete `RC11_SERVER_TEST.md` on both server operating systems before publication. No TF2 engine runtime was exercised by the local tests.

For RC12, also complete `RC12_SERVER_TEST.md`: isolate each free/user grant, verify membership and chat remain inactive, persist/revoke grants, and check trial disable never extends original deadlines. The configuration backup/rename path needs native SourceMod server verification.

For RC13, complete `RC13_SERVER_TEST.md`, especially rapid/stale purchases, insufficient balance, purchased-trial lifetime limits, restart/journal recovery, DB outage and the separate off switches. Real SQLite tests do not substitute for SourceMod callback/filesystem or MySQL/InnoDB runtime acceptance.
