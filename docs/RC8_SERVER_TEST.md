> RC10 replaces the large Engineer HUD below with a short turret-attached tooltip and fixes native chat tags. See `RC10_SERVER_TEST.md`; the other feature tests remain relevant.

# RC8: counters, balance rules and personal options

Replace `addons/sourcemod/plugins/premium.smx` and `addons/sourcemod/translations/premium_rebuilt.phrases.txt`. **Also install `addons/sourcemod/configs/premium_rules.cfg`** from the package. Preserve your edited main cfg, profiles and rules file when upgrading; merge the new cvars instead of overwriting personal settings. SourceMod's standard **Client Preferences** extension/plugin must be enabled for saved choices. TF2Attributes still installs separately.

Run `sm_reload_translations`, `sm plugins reload premium`, and `sm plugins info premium` from server console. Confirm **3.0.0-rc8**, storage/dependency readiness, Client Preferences readiness and no errors. A missing or malformed rules file blocks flight and teleport until corrected and reloaded; it does not execute cfg commands or change permissions.

| Requested feature | Included behavior / control |
| --- | --- |
| Engineer counter | Owner-only small HUD: `Sentries: 2/3` and shared team extras. `premium_engineer_hud`, personal HUD switch |
| Sentry labels | Aim at your own gun within 600 units: primary / extra 1 / extra 2, with level-one restriction. No world-label entities or through-wall labels. `premium_sentry_labels` |
| Team sentry cap | `premium_team_extra_sentries`, default 4 per RED/BLU. Primaries excluded; 0 disables extras. Newest extras removed when lowering the budget |
| Swap availability | `!premium_swapstatus` or Premium menu: ready, remaining cooldown, team-size restriction, access, mode or round restriction. The swap command uses the same check |
| Per-class settings | `configs/premium_rules.cfg`: ammo, speed, gravity, held jump/gain/cap, follow-up jump, Pyro flight handling and teleport |
| Map settings | Same rules file: exact map name or trailing wildcard; independently ban flight/teleport. Workshop base names supported |
| Trial reminders | Optional threshold notices at 5 min, 1 min, 10 sec. No replay of already-passed thresholds on reconnect; no notices for permanent/flag members. Owner and personal switches |
| Actual bonuses | `!premium_benefits` or benefits page: effective class settings, held ammo/metal, sentry limits, movement/cloak values and map gates. Viewing it grants nothing |
| Between-round presets | `sm_premium_profile fun` queues for next round start. `status`, `cancel`, or explicit `fun now`. No mid-round change by default |
| Quiet mode | Saved personal switch suppresses optional Premium adverts, join notices, reward/fake-kill messages and trial reminders. Errors, command replies and final trial expiry remain visible. No new cosmetic sounds are emitted |
| Hide tag | Saved switch hides the Premium/Trial chat prefix without changing access |
| Tag presets | Server default, team colour, gold, plain. Server bitmask controls choices; presets only recolour the configured membership tag |
| Personal glow | Server default, classic (4 emitters), team (2), subtle team (1). `premium_personal_glow` lets owners require the server style |
| Round celebration | One short team-coloured emitter per eligible winning member after round win, default 2 seconds and at most 8 emitters. No sound; hidden for cloaked/disguised players |
| Cosmetic visibility | Any player can hide Premium glow/celebration particles using `!premium_options`. Applies per viewer; other viewers keep their own choice. It does not hide TF2's native effects or other plugins' cosmetics |

Saved options are in SourceMod clientprefs, separate from membership/trial storage. A choice made before the asynchronous cookie read finishes takes priority over the old saved value. Invalid cookie data resets to validated defaults. Styles do not grant membership or bypass owner feature switches.

Useful commands:

```text
!premium
!premium_options
!premium_benefits
!premium_swapstatus
!premium_weapons

sm_premium_rules_reload
sm_premium_profile fun
sm_premium_profile status
sm_premium_profile cancel
sm_premium_profile balanced now
```

`sm_premium_rules_reload` and profile commands require root/server-console access. Ordinary preferences, status and benefits are available to any in-game player. Normal sentry building still uses the Engineer PDA; metal is still spawn-only; ammo is still finite reserve capacity with normal magazines.

Test on the server:

1. **Two Engineers on the same team:** set per-player limit 3, shared extra budget 1, spawn metal 600. Each builds a primary; only one additional gun should be allowed between them. Open two extra blueprints while a slot is free and race to place them. The loser must be blocked before spending metal where its normal build input is used; a final placement guard removes any excess gun produced through another path. Try dispenser/teleporter placement at the cap: those must work. Haul/redeploy any existing gun at the cap. Destroy and replace a primary at the cap. Move one Engineer to the other team and confirm independent budgets. Lower budget to 0: remove extras, keep primaries. Follow the earlier level-one/sapper tests too.
2. **HUD/labels:** verify own counts after building, hauling, destruction, class/team changes and access loss. Aim at primary and extras, then someone else's gun and a wall. Only your visible aimed-at gun should be labelled. Disable the HUD personally and globally; check cleanup after death/unload. Check positioning alongside any existing HUD plugins.
3. **Swap status:** compare status with actual `!swapteam` results at equal/unequal team sizes, during cooldown, in excluded mode/round states, as spectator and without access. Cooldown must report seconds and reset on map change, without locking players for the old map's elapsed time.
4. **Class rules:** set Pyro `ammo_multiplier` to 2, Scout to 3, leave Soldier at 0 (inherit). Reload rules. Respawn and verify each loadout; existing ammo is not refilled by reload. Test gravity, held jump, gain/cap and follow-up jump separately on two classes. Check the benefits/weapon report uses effective class values. Set Pyro `flight` 0 and confirm flight stops. Return fields to inherited defaults after testing.
5. **Map rules:** ban flight and teleport for the current map, reload, and check both commands/movement plus benefits page. Try exact, prefix and workshop base-name matching. Change to an unlisted map and confirm the old ban does not leak. A map ban wins over a class enable. Invalid values such as `nan`, out-of-range multipliers or invalid toggles must reject the file and log an error; correct and reload it to restore the configured gates.
6. **Queued presets:** note the current values mid-round, queue Fun, check `status`, and confirm values stay unchanged until the next round starts. Check replacing a queued choice and cancelling it. `now` should apply immediately only when explicitly requested by root. Queued choices are cleared on map change/plugin reload, and selecting one does not modify saved cfg files, permissions or trials. Persistence still uses an `exec` at the end of the main cfg.
7. **Preferences:** toggle quiet, tag hiding/style, glow style, particle visibility, reminders and HUD. Reconnect and reload the plugin; choices should survive. Check the default/team/gold/plain tags on paid/flag and trial accounts with team-chat privacy preserved. Restrict style bitmask or personal-glow permission and confirm server rules win. Hidden tags must stay hidden.
8. **Two viewers:** one hides Premium particles and one keeps them. With another player's aura already active, toggle visibility off/on. Existing particles should disappear for the opted-out viewer and remain visible for the other. Try all three personal glow styles, Spy cloak/disguise, death, map change and expiry. No duplicate or permanent emitters. Visibility preference changes rebuild Premium particles, with a 2-second toggle cooldown.
9. **Celebration:** win a round with eligible players on both teams. Only alive, visible winners within the global emitter budget receive the short effect; it must end after its deadline and clear at next round, disable, cloak, disconnect, map change and unload. Turn off Premium particles for one viewer. No combat-time celebration and no sound. Test budget 0 and 1, and owner disable while the effect is active.
10. **Reminders/quiet:** use a non-flag, non-member trial account. Grant a trial long enough to cross each threshold. Expect one notice at each crossed threshold, none each heartbeat. Reconnect below a threshold: do not replay it. Quiet/reminder-off suppress notices without extending the trial; final expiry remains authoritative. Verify quiet recipients get no optional adverts/reward messages while non-quiet recipients still do, and actual health rewards still apply.
11. **Benefits page:** compare class rule values with cfg, current held ammo/fuel and metal with the HUD, and team/player sentry counts with placed guns. Current held ammo is labelled separately from the configured multiplier; it is not an inferred maximum for custom class scripts. Repeatedly opening this page must not refill ammo/metal, heal, create effects or consume a trial.

Run the existing RC7 ammo and RC6 cloak/flight/PDA acceptance checks after these additions. Automated tests exercise preference encoding, reminder thresholds, rule matching/inheritance, team-budget ownership and celebration cleanup with fake engine boundaries. They do not validate live TF2 rendering, networking, build prediction, native HUD placement or other-plugin interactions.
