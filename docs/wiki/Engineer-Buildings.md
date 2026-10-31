# Engineer buildings

Engineers place extra buildings using the **ordinary build PDA**. There is no extra-building spawn command and no custom engine DLL/SO requirement. This release keeps TF2's native **six total building slots per Engineer**.

## Limits

| Setting | Default | Meaning |
| --- | --- | --- |
| `premium_sentry_limit` | 3 | Total sentries, including primary |
| `premium_dispenser_limit` | 3 | Total dispensers |
| `premium_teleporter_entrance_limit` | 3 | Entrance ends |
| `premium_teleporter_exit_limit` | 3 | Exit ends |
| `premium_team_extra_sentries` | 4 | Extra sentries shared across one team; primary excluded |

Each type limit accepts 1–3; one disables that type's extras. Every teleporter end consumes one slot, so a pair consumes two. These limits do not increase the shared six-slot maximum. Examples:

| Valid six-slot layout | Used slots |
| --- | --- |
| Three sentries + three dispensers | 6 |
| One sentry + three dispensers + one teleporter pair | 6 |
| Two dispensers + two teleporter pairs | 6 |
| Three teleporter pairs | 6 |

Three sentries, three dispensers and three pairs together would require twelve slots and are unsupported. Team cap zero blocks extra sentries. Access to one extra building type does not grant the other types.

## Levels, metal and normal use

The primary sentry upgrades normally. The two additional sentries are restricted to **level one**. Gunslinger mini-sentry behaviour remains native. Dispensers and teleporters retain normal upgrade levels.

`premium_metal` sets spawn metal and capacity when the `metal` feature is allowed. It is not automatic regeneration. Building costs still apply; normal pickups/resupply remain available. There is no extra-metal command.

The plugin keeps real builder/team ownership and the native building list. Carrying/redeploying an existing building is allowed at capacity. Lowering limits or losing access updates tracked extras; cleanup preserves an untracked ordinary primary building. Extra-building access is restricted in modes such as Arena/MvM.

## Teleporter pairing

Teleporters use native available unmatched ends. They are not three numbered selectable routes. Build and test a pair before adding the next. Destroying an end can cause a remaining unmatched end to bind another free end; do not depend on permanent numbered destinations.

## Small building tooltip

With `premium_engineer_hud 1`, aiming at your building within range shows a small native annotation. `premium_sentry_labels 1` includes primary/extra sentry labels. Player tooltip preferences are in `!premium_options`.

RC18 keeps that annotation attached without repeatedly restarting its slide-in. Labels use shorter lines; `Total 3/6` means three of your six shared building slots are occupied.

The existing native health/upgrade panel cannot accept a custom text line from this server plugin. The annotation is separate; the large screen overlay is not used.

## Why the build menu sometimes pauses

The plugin makes the normal build menu allow another building when you have room. If a friendly Engineer is working on a building nearby, it briefly pauses that change so their upgrades and hauling still work. You may see “Already Built” for a moment; try again after the pause.

RC17 corrects a client-side block that prevented extra dispensers and teleporters from being selected. Existing eligible utility buildings are temporarily hidden from their owner while the build-selection PDA is open. They are shown again when you select a blueprint or close the menu. Other players keep seeing them, and their real ownership, direction and server behaviour stay unchanged. The shared six-building limit still applies.

Hauling, friendly upgrades and teleporter pairing still need checking on a real server, especially with custom maps or other plugins. See the [server test plan](https://github.com/daneleo35/PremiumMod---TF2/blob/main/docs/RC10_SERVER_TEST.md) and [testing guide](Development-and-Testing.md).

[Configuration](Configuration.md) · [Features](Features.md) · [Home](Home.md)
