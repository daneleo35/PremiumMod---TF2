# Core server test sequence — 3.0.0-rc10

For native chat tags and the small turret-attached tooltip replacing the large overlay, start with **`RC10_SERVER_TEST.md`**. For the Pyro airblast jump, use **`RC9_SERVER_TEST.md`**. For the 15 controls and preferences, start with **`RC8_SERVER_TEST.md`** and install the new rules file. For the reserve-ammo correction, start with **`RC7_SERVER_TEST.md`**. For cloak/metal/PDA changes, all-flamethrower flight, and balance settings, start with **`RC6_SERVER_TEST.md`**. Replace the plugin and translations, install the new profile cfg files, and merge the new settings into your existing config. `RC5_SERVER_TEST.md` is a historical checklist; its unlimited-cloak, `!metal` and `!sentry` instructions are superseded.

For the rc4 gameplay fixes, replace `addons/sourcemod/plugins/premium.smx` and reload the plugin. Keep your custom config. If your server still has the rc2 translation file, also replace `addons/sourcemod/translations/premium_rebuilt.phrases.txt` and run `sm_reload_translations` before reloading Premium. rc3 already supplied the corrected translations. No database migration is needed. `FORUM_REVIEW.md` includes the new sudden-death, continuous-flight and Scout-ball checks.

1. Unload the old Premium plugin. Install TF2Attributes and matching gamedata separately if needed. Copy the new `premium.smx` and `premium_rebuilt.phrases.txt`. Keep your existing configured values when upgrading rc1, adding these new options:

   ```text
   premium_glow "1"
   premium_glow_style "0"
   ```

2. Load Premium and run `sm plugins info premium` and `sm_premium_diagnostics` in the server console. Confirm version **3.0.0-rc10**, dependency readiness, and database readiness. The initial owned-emitter count should be zero.
3. Join with a premium account or use `sm_premadduser "player name"`. Try `!premium`, its benefits page, and `!glow on`. With one glowing player, diagnostics should report **four owned emitters**. Repeating `!glow on` must keep that number at four. `!glow off` must return it to zero. Client particles can take a brief render frame/tail to disappear; they must not remain permanently or survive as owned emitters.
4. Repeat on/off several times, then test death/respawn, team/class changes, Spy cloak/disguise, disconnect, map change, and plugin unload. The player's other game effects should remain intact. Requested glow should return after ordinary death/respawn or temporary hiding, provided premium access remains valid.
5. Test `premium_glow 0` and `premium_enable 0`. The owned-emitter count must return to zero. Re-enable the settings and use `!glow on` again. Try style 1: diagnostics should report **two** emitters per glowing player and the particles should match RED/BLU.
6. Test trial expiry using an ordinary account without a premium/admin flag. A root account already has premium access and cannot test self-service trial eligibility. Temporarily use a short `premium_trial_timelimit`, grant a trial or use `!trial`, enable glow, and let the trial expire. Perks and glow should stop. Reconnect must not create another trial. Restore your normal duration afterward.
7. Hold jump through several landings with bunnyhop enabled; test from a standing start and while moving. Check normal Scout air-jumps and the B.A.S.E. Jumper parachute separately; these airborne inputs should work normally. Then smoke-test the remaining features listed in `FEATURES.md`: all classes/loadouts, reward recipients, Heavy spin, cloak, special weapons, resupply, teleport and colour/FOV commands. Test SQLite and your MySQL setup if you use it. The expanded `ACCEPTANCE.md` covers the edge cases and soak test.

If anything fails, save the output of `sm plugins list`, `sm_premium_diagnostics`, the relevant SourceMod error log, and the exact commands/settings/loadout used. This project has not connected to or modified your live server; these are the tests for you to run there.
