> Historical rc5 checklist. See `RC6_SERVER_TEST.md` for the current release: unlimited cloak and the !metal/!sentry commands below have been replaced.

# rc5: server feedback fixes and new Engineer/chat perks

Replace **both** `addons/sourcemod/plugins/premium.smx` and `addons/sourcemod/translations/premium_rebuilt.phrases.txt`. rc5 adds translation keys for the new commands/menu. Keep your existing configured values rather than overwriting the config with packaged defaults. Then run:

```text
sm_reload_translations
sm plugins reload premium
sm_premium_diagnostics
```

Confirm version **3.0.0-rc5** and successful dependency/storage readiness. TF2Attributes is still installed separately; no additional external plugin is required by these features. This project has not run a TF2 server: the checks below require your server.

For a focused test, add/set these in `cfg/sourcemod/premium_rebuilt.cfg` or temporarily in the server console. Restore your preferred values after testing:

```text
premium_stunballs 15
premium_jarates 15
premium_clip_multiplier 2
premium_pyrofly_enabled 1
premium_pyrofly_speed 400
premium_cloak 1
premium_deadringer_infinite 1
premium_chat_tags 1
premium_chat_tag "{green}[Premium]{default}"
premium_trial_chat_tag "{lightgreen}[Trial]{default}"
premium_sentry_limit 3
premium_sentry_cost 130
premium_metal 800
premium_metal_cooldown 30
```

## Sandman, Wrap Assassin and Jarate

- Spawn with the supported item and the configured initial reserve.
- Use several charges and wait. The native recharge must return **one item per ordinary recharge interval**, continuing until the configured capacity. It must not stop permanently at one and must not restore all 15 at once.
- Repeat after depleting the reserve completely. A reserve of three is quicker to test before repeating with 15.
- Check that your other weapons' ammunition is unchanged.
- Visit resupply, change class/loadout, disable the setting and expire a trial. Owned capacity must restore. Existing ammunition is not retroactively removed.

The implementation sets `maxammo grenades1 increased` on the player. Valve's native `EffectBarRegenFinished` gives one item and restarts while below `GetMaxAmmo`; Premium does not add ammo in its heartbeat. Reference: [Valve weapon recharge implementation](https://github.com/ValveSoftware/source-sdk-2013/blob/master/src/game/shared/tf/tf_weaponbase.cpp).

## Clip capacity

- With `premium_clip_multiplier 2`, test supported primary **and secondary** magazines: stock Scattergun/pistol, Soldier rocket launcher/shotgun, Engineer shotgun/pistol, Medic syringe gun, Sniper SMG, Spy revolver, Demoman grenade/sticky launchers.
- Fully reload after spawning. Capacity is an engine attribute, not an immediate clip refill.
- Visit resupply, change loadout, die/respawn and reload again.
- Run `!premium_weapons`. It reports slot, item/classname, current clip, support status, effective `mult_clipsize` hook and owned-attribute state. Record this output if the capacity still fails. Other item attributes can combine with the multiplier.
- Exclusions remain: Beggar's Bazooka, Widowmaker, Cow Mangler, Loose Cannon, clipless weapons and classes outside the allowlist. Reserve ammo is not a magazine.
- Set multiplier 1, disable Premium or expire a trial; the original owned attributes should restore. Do not expect already-loaded bonus rounds to disappear immediately.

rc5 maintains ownership separately for each slot, repairs missing runtime attributes and clears the weapon owner's cache when a capacity modifier changes. The executable attribute harness verifies those operations with fake provider/entity boundaries; it does not simulate actual reloading.

## Pyro flight

- Jump, aim down and hold primary fire with a supported flamethrower and fuel.
- Use W/A/S/D to steer. Sustained flight must approach the requested horizontal motion instead of accumulating reverse recoil. With no movement input, horizontal flight should slow toward zero.
- Hold for at least 15 seconds, steer/reverse, then release fire. Flight assistance must stop on release, empty fuel, weapon change, taunt/stun/freeze, access loss or disabling.
- Test airblast separately; it must not create the primary-fire flight effect.
- Repeat as a flag member and a trial user. Test at your server's intended tick rate.

## Unlimited Dead Ringer

- With both `premium_cloak 1` and `premium_deadringer_infinite 1`, equip Dead Ringer, arm it and trigger a real feign.
- Cloak resource should stay replenished and return to full for repeated use. Manual decloak and weapon switching must still work normally.
- The game still owns activation, decloak/weapon delays and the ordinary resistance-condition duration. Premium must not re-add resistance conditions or respawn you on a fake-death event.
- Repeat several times, then test an ordinary watch and Cloak and Dagger.
- Set `premium_deadringer_infinite 0`: Dead Ringer must use its ordinary meter while ordinary-watch unlimited cloak can remain active. `premium_cloak 0` disables the unlimited resource behavior for every watch.

## Player chat tags

- A flag/database member should show `[Premium]`; an account entitled only by a trial should show `[Trial]`.
- Test public chat, team chat, dead chat and spectator chat. The engine's original recipients must be preserved; the opposing team must not receive private team chat.
- Check long names, Unicode, percent signs and literal colour-looking text in messages. The original message text must be preserved. A tag may be omitted when it cannot fit without truncating the name or exceeding the message packet limit.
- Trial expiry/revocation should remove that tag. Disable `premium_chat_tags` to leave another chat formatter in charge.
- If Simple Chat Processor or Chat-Processor is present, test for duplicate prefixes and compatibility with its mute/format rules. Premium uses their optional forwards instead of requiring them. Custom chat systems still need compatibility testing.

## Engineer: one normal sentry plus two level-one extras

- Build your ordinary sentry with the PDA and upgrade it to level 3.
- Use `!metal` to refill to the configured reserve. Repeated successful refills must respect the cooldown. Setting above 200 raises native metal capacity; spending metal still works normally, including Widowmaker spending/refunds. No per-tick metal replenishment occurs.
- Aim at flat clear ground within 256 units and use `!sentry` twice. The limit includes the ordinary sentry: a fourth is denied. Default cost is 130 metal per command-created sentry.
- Both additional sentries must stay **level 1**, including owner and teammate wrench hits. Repairs and ammunition refill should remain possible. The original ordinary sentry remains upgradeable.
- Damage and enemy sappers must work against every sentry. Check kill credit, team skin, pickup/redeploy, destruction and the ownership HUD. Multi-Wrangler interaction is an explicit compatibility test; no simultaneous multi-Wrangler feature is claimed.
- Destroy the primary and use `!sentry`: it should create a new upgradeable primary while the surviving two extras remain level-one-only. Without a primary, the first command-created sentry takes that primary role.
- Test `!sentry clear`: it removes command-created sentries, including a primary created through the command. It leaves an ordinary PDA-created sentry alone.
- Check limits with a dispenser and teleporter pair. Premium never adds an object when six native ownership slots are already occupied.
- Try a wall, steep slope, occupied hull, spawn/no-build volume, out-of-range ground, carrying a building, Arena, MvM and a stopped round: placement must be refused and metal must not be charged.
- Revoke access, disable Premium, change class/team, disconnect, change map and unload: command-created sentries must be removed. Ordinary death alone does not remove them.

The native object activation and `SetBuilder` input register ownership and team membership. Bonus sentries keep the native map-placed non-upgradeable flag; they do not pretend to be sappers. Reference: [Valve base object lifecycle](https://github.com/ValveSoftware/source-sdk-2013/blob/master/src/game/server/tf/tf_obj.cpp), [sentry upgrade eligibility](https://github.com/ValveSoftware/source-sdk-2013/blob/master/src/game/server/tf/tf_obj_sentrygun.cpp).

If any check fails, capture `sm plugins list`, `sm_premium_diagnostics`, `!premium_weapons` where relevant, the SourceMod error log, the exact settings and loadout, and the smallest reproduction. State whether a competing ammo, building or chat plugin was enabled.
