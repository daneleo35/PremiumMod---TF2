# RC7: reserve ammunition, including Pyro fuel

The ammo perk now increases primary and secondary carrying capacity and the ammunition supplied on spawn. Magazine size stays native. Pyro fuel, Heavy minigun ammunition and other clipless normal-ammo weapons are included. The old magazine allowlist no longer limits this perk.

1. Replace `addons/sourcemod/plugins/premium.smx` and `addons/sourcemod/translations/premium_rebuilt.phrases.txt`. Keep your custom main cfg. If you use the supplied balance profiles, merge their renamed `premium_ammo_multiplier` line into your edited profiles. Reload translations and Premium from the server console:

   ```text
   sm_reload_translations
   sm plugins reload premium
   premium_gameplay_teams 3
   premium_ammo_multiplier 2
   ```

   Confirm `premium_version` reports **3.0.0-rc7**. Existing `premium_clip_multiplier` settings are accepted as an alias and synchronize with the new name. Use just one name in your saved cfg; prefer `premium_ammo_multiplier`. Both accept 1-10. A profile executed later can replace your value; Cosmetic/Balanced use 1, Fun uses 1.5.

2. Respawn as a premium Pyro with a stock flamethrower or Backburner. At 2x, a native 200 fuel capacity should become **400**. The default 1.25 gives 250. Test Degreaser, Phlog and Dragon's Fury too; their own native/item limits still combine with the reserve multiplier. Fire, stop, and wait away from pickups, dispensers and resupply: the fuel must stay spent. This is extra finite ammunition, with no automatic regeneration.

3. Test an ordinary rocket launcher, shotgun and pistol. The launcher should still load its normal magazine (for stock, 4), while the reserve increases (for stock Soldier, 20 to 40 at 2x). Shotgun/pistol magazines also stay normal; check the reserve number beside the magazine. Repeat with Heavy's clipless minigun (stock 200 to 400 at 2x), a flare gun, SMG and a weapon with an ammo-capacity penalty. Native penalties should still apply.

4. Changing the cvar while alive changes capacity without granting more ammo immediately. Respawn to receive the new reserve, or use native pickups/resupply. Run `!premium_weapons` to see the actual primary/secondary reserve counts, player capacity hooks and ownership. A stock loadout at 2x should report both capacity hooks at approximately 2.000 and `owned=1`. An equipped item or another plugin can change the combined hook value. If a pool stays at 1.000, send this report along with your exact loadout and active cfg/profile.

5. Ammo packs, dispensers and locker resupply should respect the expanded capacity through native TF2 rules. Check changing loadouts, death/respawn, class/team changes and a map change. Disabling the perk, setting multiplier 1, restricting the team, expiring premium access or unloading Premium must restore the capacity attributes owned by Premium. Ammo already supplied is not explicitly deleted; later native capacity reconciliation can clamp it. Other plugins' magazine/ammo changes should remain intact.

6. Verify that Engineer metal still uses only `premium_metal`, and Sandman/Jarate reserves still use their separate settings and one-at-a-time recharge. The reserve multiplier must not multiply these resources. Energy or meter-based weapons that have no normal reserve pool keep their native resource behavior; this does not create extra energy, cloak, rage or medigun charge.

The previous flight, finite cloak, spawn-only metal, PDA sentry, team/profile, chat and trial checks in `RC6_SERVER_TEST.md` still apply. Its older clip-capacity instructions are superseded by this guide.

Automated tests execute the production ammo controller and ownership helper with fake engine/provider boundaries. They verify both reserve attributes, clipless fuel, shared-pool deduplication, item penalties, live capacity changes, restoration and failed attribute writes. They do not execute a TF2 server. The implementation follows Valve's [player maximum-ammo hooks](https://github.com/ValveSoftware/source-sdk-2013/blob/master/src/game/shared/tf/tf_player_shared.cpp) and SourceMod's [native capped ammo supply](https://sm.alliedmods.net/new-api/sdktools_functions/GivePlayerAmmo).
