> RC7 changes the ammo perk to reserve capacity and spawn ammo, including Pyro fuel. See `RC7_SERVER_TEST.md`; the other RC6 checks below still apply.

# RC6 server checks

This release changes cloak, metal, sentry placement, flight and balance controls. Automated checks execute SourcePawn policies/controllers and SQLite; they do not run TF2. These server checks remain necessary.

1. Back up your custom cfg. Replace `addons/sourcemod/plugins/premium.smx` and `addons/sourcemod/translations/premium_rebuilt.phrases.txt`. Copy the three supplied `cfg/sourcemod/premium_profiles/*.cfg` files. Merge the new settings below into your existing `premium_rebuilt.cfg`; do not overwrite your custom values. Remove obsolete `premium_deadringer_infinite`, `premium_metal_cooldown` and `premium_sentry_cost` lines. The old `!metal` and `!sentry` commands are removed. TF2Attributes and its gamedata still install separately.

   ```text
   premium_gameplay_teams "3"
   premium_cloak "1"
   premium_cloak_multiplier "2"
   premium_deadringer_multiplier "2"
   premium_metal "600"
   premium_sentry_limit "3"
   premium_pyrofly_enabled "1"
   premium_pyrofly_speed "400"
   premium_pyrofly_lift "200"
   premium_pyrofly_acceleration "600"
   premium_swapteam_cooldown "60"
   premium_swapteam_max_difference "1"
   ```

2. From the server console, run `sm_reload_translations`, `sm plugins reload premium`, `sm plugins info premium`, and `sm_premium_diagnostics`. Confirm **3.0.0-rc6**, dependency/storage readiness and a clean error log. Use a premium test player. The example above deliberately enables the requested server tests; the Balanced profile disables flight.

3. **Backburner flight:** choose Backburner, jump, and hold primary fire in clear space. Aim down to climb, level to hover, up to descend; hold W/A/S/D to steer. Continue for at least 10 seconds. It must not start sinking or building up reverse recoil. Repeat with stock, Degreaser, Phlog and a reskin. Repeat with Dragon's Fury after actually firing a primary shot. Check landing, release, empty fuel, airblast, switching to shotgun/melee, taunts, water, stun/freeze, death, and access loss: no residual flight. If you use unusual server gravity, repeat at that gravity. Do not expect flight while standing on the ground; jump first.

   For a failure, execute `sm_premium_flight` from your **client console while still holding primary fire** and save its two console lines. It reports enabled/access/team, class, grounded/blocked state, weapon classname/item, state, ammo, firing, buttons, pitch, velocity and configured handling. `!premium_flight` also prints this information, but opening chat releases your firing input, so use the console form during diagnosis.

4. **Finite cloak:** with ordinary cloak, measure native duration at multiplier 1, then repeat at 2 and 3 from the same starting meter. It should approximately double/triple, decline to empty, then end normally. Repeat while moving with Cloak and Dagger, and separately with Dead Ringer using `premium_deadringer_multiplier`. Dead Ringer still needs its normal activation meter, consumes its native activation fee, and recharges normally; damage resistance must end on its native schedule. Fake death must not trigger respawn or healing. Stationary Cloak and Dagger still recharges naturally. Changing multiplier to 1 or disabling the perk while cloaked must restore normal drain immediately. Native terminal fade is never reversed; timing can differ by less than multiplier × one server tick.

5. **Spawn-only metal:** spawn as Engineer with 600. Build and repair, use Widowmaker, then wait at least a minute away from pickups/dispensers/resupply. Metal must remain spent. Changing the cvar or running `prem_reload` must not refill the current life. Respawning should grant the configured reserve. Native ammo packs, dispensers and locker resupply may replenish normally. Try 200 and 1000; larger capacity must persist while spending. There is no refill command or timed top-up.

6. **Normal PDA sentries:** build the first sentry from the usual construction PDA. Upgrade it to level 3. Open the same PDA and select sentry again, then place a second and third using the native blueprint. Check ordinary placement rejection, construction time and metal cost. Second and third must stay level 1 even when teammates wrench them. Attempt a fourth: it must be blocked. Repeat with a native build bind. With Gunslinger, native mini-sentry behavior remains applicable.

7. **Sentry lifecycle:** haul/redeploy the primary and each extra, including when already at the three-sentry cap. Try repairing and refilling each; sap each with an enemy Spy and remove its sapper. Destroy the primary while extras survive, then build an upgradeable replacement from the PDA. Destroy an extra and replace it; it must remain level 1 while a primary exists. Check Wrangler and Rescue Ranger with multiple targets. Native HUD/destroy-menu selection may target only one gun at a time; record which gun is selected. Lower the cap from 3 to 2 to 1; newest tracked extras should be removed and the primary preserved. Repeat disable, membership expiry, class/team change, disconnect and plugin reload: extras must disappear, modes restore, and ordinary construction work. Check another Engineer's guns are unaffected. Test ordinary maps; extra sentries remain disabled in Arena/MvM.

8. **Balance controls:** run `sm_premium_profile cosmetic`, `balanced`, and `fun` from server console/root access. Review the values in `docs/BALANCE.md`. Cosmetic must remove gameplay perks and tracked extras while retaining membership/chat/glow/colour/FOV. Trial time and database/flags must remain unchanged. `premium_gameplay_teams 1` allows RED gameplay only; 2 allows BLU only; 3 both. Test a permitted and a restricted member at the same time. Normal metal/health spawn grants wait for the next spawn. Team swaps must respect the configured cooldown and size limit, and mode/health/attribute cleanup must follow live team/profile changes.

9. Repeat the older throwable one-at-a-time recharge, clip capacity/reload, chat privacy, trial/storage and glow checks in `SERVER_TEST.md` / `ACCEPTANCE.md`. Check the error log after a map change and a normal play session.

Keep the exact loadout, cvars, server/TF2Attributes versions, reproduction and log output for any remaining failure. Compilation and fake-engine controller tests are not a claim that these native engine interactions have already passed on your server.
