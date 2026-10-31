# RC11: extra dispensers and teleporters, six shared building slots

This is a SourceMod plugin update for stock TF2 on Linux or Windows. No engine DLL/SO replacement, memory patch, new extension, or new gamedata is supplied or required. The native client/server interaction still needs live server acceptance; automated tests do not run TF2.

## Upgrade and configure

Replace `addons/sourcemod/plugins/premium.smx` and `addons/sourcemod/translations/premium_rebuilt.phrases.txt`. Keep your customised config and merge these settings into `cfg/sourcemod/premium_rebuilt.cfg`:

```cfg
premium_sentry_limit "3"
premium_dispenser_limit "3"
premium_teleporter_entrance_limit "3"
premium_teleporter_exit_limit "3"
```

Each accepts **1–3**, including the ordinary building of that type. `1` disables that type's extras. Entrance and exit caps are independent. **All types share six slots per Engineer. Each teleporter end costs one slot.** This limit is fixed, not a cvar that can accidentally exceed the native networked array.

Examples that fit: three sentries plus three dispensers; one sentry, three dispensers and one teleporter pair; two dispensers and two pairs; or three complete pairs with no sentry/dispenser. The shared team-extra-sentry budget still applies, and only the two extra sentries are restricted to level one. Dispensers and teleporters use normal upgrade levels.

Cosmetic/Balanced profiles set the three new caps to `1`; Fun uses `3`. Profile application overwrites the corresponding cvars. Feature access can also be restricted through `premium_feature_dispenser` and `premium_feature_teleporter`; these overrides do not grant Premium membership. Extras remain disabled in Arena/MvM.

Reload translations and the plugin, or change map. Verify `premium_version` reports `3.0.0-rc11`. Build using the ordinary construction PDA. No Premium spawn command is needed. Ordinary metal cost, placement, construction and resupply apply. There is no metal refill.

## Native PDA behaviour

Existing utilities are temporarily omitted from native PDA ownership counts using the existing disposable-building flag while another of that type can be placed. Their real object type, builder, team and entrance/exit mode stay intact. The plugin claims only flags it changes and restores them on weapon exit, death, round end, full capacity and cleanup. Carried objects stay visible to native redeployment checks.

The native flag also affects upgrade checks. Before a nearby friendly wrench/Gunslinger swing, the plugin restores utility flags and pauses masking for half a second through its delayed impact. The build PDA may briefly show **Already Built** during that pause; select again after the swing finishes. This is a stock-client workaround and needs the friendly-upgrade checks below. No invalid teleporter object mode or custom teleporter transport logic is used.

Native TF2 pairs available unmatched entrances/exits. Build an entrance and exit, test that pair, then add the next pair. Matching, shared upgrade state, recharge, sappers and transport remain native. This does not add numbered, manually selected destinations; an unmatched end may match a different available opposite end after destruction.

## Server checks

1. With Premium active, build three dispensers via the PDA. Switch to wrench and upgrade all three to level three. Try a fourth with free total slots: it must be rejected without spending metal.
2. Add one sentry and one complete teleporter pair. Confirm six total. Try another sentry, dispenser, entrance and exit. All must be rejected. Check metal before/after each denied attempt.
3. Haul/re-place a dispenser, each teleporter direction and a sentry at the six-slot cap. This must not create another object or charge for a new one. Check the PDA after leaving and re-entering it.
4. Aim at your own buildings. The small native tooltip should show the type count and `Buildings: n/6`; no large white overlay should appear. `!premium_benefits` shows all counts and effective caps. Another player's buildings must not receive your tooltip.
5. Destroy enough buildings to leave room. Build two pairs at separated locations and test each in both teams. Upgrade each pair and verify its own shared levels/recharge. Destroy and rebuild one end; check that other complete pairs remain functional. Repeat with three pairs and no sentries/dispensers.
6. While the owner holds the build PDA/blueprint, have a teammate repair and upgrade each dispenser and teleporter, using stock wrench, Jag and Gunslinger. Repeat during teleporter recharge and after hauling. Expect only the brief PDA pause described above, never lost upgrade metal or a permanently disposable building.
7. Lower a cap live, for example `premium_dispenser_limit 1`. Only tracked extra dispensers should be removed; the ordinary dispenser and other building types remain. Repeat separately for entrances/exits. Raise it again and build normally.
8. Test access expiry/removal, team/class change, plugin disable/reload and map change. Tracked extras must be removed and temporary flags restored. On ordinary player death, buildings remain under native rules but PDA masks clear. Check a non-Premium Engineer still has stock limits.
9. Repeat the PDA, hauling, pairing and friendly-upgrade checks on a Windows test server before claiming Windows runtime acceptance. Linux and Windows use the same SourcePawn code and standard SourceMod APIs; neither has been run here.

Check the SourceMod error log throughout. Also retest RC10's chat tags (`!premium_chat`) and RC9's airblast jump. RC11 does not claim that automated fake-entity tests verify client prediction, native teleporter matching, or rendering.
