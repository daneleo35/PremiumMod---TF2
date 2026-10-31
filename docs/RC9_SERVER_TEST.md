# RC9: Pyro airblast jump

Right-click with an airblast-capable flamethrower to add a rocket-jump-style impulse. It pushes away from the direction you aim, with upward lift. Aim down for a mainly vertical launch, or behind you for forward travel. It can take off from the ground; no extra command or jump key is required. Primary-fire flight remains a separate feature.

The boost requires a real fuel-consuming airblast: the weapon must advance its native secondary cooldown and spend fuel between the before/after weapon-frame samples. A held button or secondary animation alone does not qualify. Holding right-click may boost again when the weapon actually fires its next native airblast, subject to the additional owner cooldown. Native fuel cost, cooldown, reflection/extinguishing and weapon behavior are unchanged.

Replace the plugin with RC9 and merge these settings into your main cfg. No database change or new third-party dependency is required. Existing RC8 translations/rules remain compatible; the full package includes the current files.

```text
premium_airblast_enabled "1"
premium_airblast_force "400"
premium_airblast_lift "350"
premium_airblast_speed_cap "900"
premium_airblast_cooldown "0.75"
```

| Setting | Meaning / range |
| --- | --- |
| `premium_airblast_enabled` | 0/1 feature switch, independent of primary-fire flight |
| `premium_airblast_force` | Recoil opposite aim, 0-1500 |
| `premium_airblast_lift` | Minimum upward impulse added to current velocity, 0-1000 |
| `premium_airblast_speed_cap` | Horizontal and upward speed limit for this boost, 100-2000; pre-existing faster momentum is preserved without further acceleration |
| `premium_airblast_cooldown` | Minimum seconds between boosts, 0.1-10; native airblast timing still applies |

Cosmetic/Balanced profiles disable the boost; Fun enables it with the values above. Merge those new fields into any profiles you have edited. As with other explicit class settings, optional Pyro `"airblast" "0"` or `"airblast" "1"` in `configs/premium_rules.cfg` overrides the global feature toggle; `-1` inherits it. The existing map **flight ban blocks both continuous flight and airblast jumps**. Global disable/access/team restrictions still apply. No boost while frozen, taunting, stunned, charging, in deep water or using another movement type.

The Phlog cannot airblast and does not receive a boost from rage activation. Stock, Backburner, Degreaser and airblast-capable reskins use their own native costs. Dragon's Fury is recognized by its separate weapon class. Energy/meter modifications or custom charged-airblast releases are not treated as normal fuel-consuming right-click blasts. Normal falling velocity and fall damage remain relevant; the perk adds movement rather than granting invulnerability.

Server tests:

1. Confirm `premium_version` is **3.0.0-rc9**. On a premium Pyro with stock flamethrower, aim down and right-click from standing: one upward launch. Aim horizontally: backward recoil plus lift. Try aiming behind while moving forward. Compare with the ordinary primary-fire flight and ensure right-click does not produce continuous per-frame thrust.
2. Hold right-click. Boosts should occur only when native airblasts actually fire, with native fuel consumption. Try Backburner and its higher cost, Degreaser, a reskin and Dragon's Fury. Use the last complete fuel charge: it should work; insufficient fuel should not. Phlog rage and shotgun/melee secondary input must not boost.
3. Set cooldown to 2 seconds. Native blasts between boost intervals must still work, without granting an extra impulse or queuing one for later. Set both force and lift to 0: no added movement. Raise/lower force/lift/cap and verify direction, takeoff and bounded repeated boosts. Existing faster rocket-jump/knockback momentum should not be cut down to the lower configured cap.
4. Test class/team change, death/respawn, weapon switching, trial expiry, disable and map change. No stale or delayed impulse. Repeat while taunting, frozen/stunned and in deep water. Test from moving platforms and with the server's actual gravity settings.
5. Disable the global feature, then test a Pyro class override. Ban flight on the current map: both Pyro movement perks must stop even when their class switches are enabled. Check Cosmetic/Balanced/Fun behavior and queued profile changes.
6. Check native reflection, pushing enemies and extinguishing teammates still work. Confirm the plugin does not add fuel, remove extra fuel, change weapon fire timers, heal or add resistance conditions. Test near dispensers/pickups too; detection conservatively requires an observed net fuel decrease, so an ammo gain in that exact simulation frame can suppress that boost.
7. `!premium_benefits` shows the current boost settings. `sm_premium_flight` in your client console prints enabled state, force, lift, cap, cooldown and next boost time alongside the existing firing diagnostics. Save that output, loadout, cvars and error log for a failure.

Automated tests execute the production boost controller and impulse/detection helpers with fake native boundaries. They cover one impulse per sample, native rejection, higher fuel cost, last fuel charge, wrong input/class/weapon, permission/map loss, cooldown, takeoff, direction and caps. They do not prove live TF2 weapon-frame ordering, networking or movement prediction. Continue the RC8 and earlier server checks.
