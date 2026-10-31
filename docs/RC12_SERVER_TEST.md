# RC12: independent feature grants and trial disable

Feature access and Premium membership are separate. Public/user grants and explicit feature overrides can authorize individual features without granting membership, a Premium/Trial player-chat tag, join/welcome announcements, or any other feature. Premium members retain the full configured feature set, subject to feature permission overrides. No membership/trial database schema or admin flags are changed.

## Install

Replace `premium.smx` and `premium_rebuilt.phrases.txt`, and install the new `addons/sourcemod/configs/premium_features.cfg`. Keep your customised `premium_rebuilt.cfg` and rules/profiles. Restart/change map, or reload translations and the plugin. Check `premium_version` is `3.0.0-rc12`. All public/user grants default off.

The existing master `premium_enable` must remain `1` to use any features. TF2Attributes and the existing dependencies remain required. No engine DLL/SO, additional extension, or new gamedata is required.

## Disable trials independently

Set this in `cfg/sourcemod/premium_rebuilt.cfg`:

```cfg
premium_trial "0"
```

This blocks new trial claims **and stops existing trials from granting access**. It does not revoke flag/database memberships or separately granted features. Saved deadlines and used-trial records remain unchanged; time continues to pass. Re-enabling trials restores an unexpired trial only until its original deadline, never extending or restarting it. Disabled-trial adverts no longer suggest `!trial`; reminders/tags are suppressed when no other membership source applies. An admin-created trial record also grants no trial access while this setting is off.

## Make a feature public

Run in the server console (root admin command):

```text
sm_premium_feature_public airblast 1
sm_premium_feature_public glow 1
```

Or edit the `Public` section of `addons/sourcemod/configs/premium_features.cfg`, setting selected feature values to `1`, then run `sm_premium_features_reload`. Missing feature keys mean `0`.

Access does not override its enable/balance settings. For example, `airblast` still needs `premium_airblast_enabled 1`; Balanced/Cosmetic profiles turn that setting off. Public glow enables use of `!glow`, without automatically turning the player's aura on. Extra dispensers use the normal PDA; ammo/metal/health/uber spawn bonuses wait until the next spawn. Granting or reloading access never refills ammo/metal/health. Class, map, team, round, cooldown and shared six-building limits still apply.

## Give a specific user a feature

Root admin commands accept an online target/name or an offline Steam ID:

```text
sm_premium_feature "Dane" glow 1
sm_premium_feature "STEAM_1:0:123456" dispenser 1
sm_premium_feature "STEAM_1:0:123456" ammo 1
sm_premium_feature "STEAM_1:0:123456" ammo 0
```

Steam2, Steam3 and Steam64 IDs use the existing strict normalizer; saved keys are canonical Steam3 IDs. Grants persist across disconnects, map changes and restarts until revoked. Online target groups use SourceMod targeting/immunity checks; a supplied Steam ID addresses that account directly. Authenticated human players are supported, not bots. At most 1024 user records are accepted.

Example file (the distributed file lists every public key at zero):

```text
"PremiumFeatures"
{
    "Public"
    {
        "airblast" "1"
    }
    "Users"
    {
        "STEAM_1:0:123456"
        {
            "glow" "1"
            "dispenser" "1"
        }
    }
}
```

`0` removes **that grant source**. It does not deny access supplied by Premium membership, another public/user grant, or an explicit permission override. To restrict feature permissions, use SourceMod feature overrides. An explicitly defined global `premium_feature_<key>` override authorizes matching users independently of membership; an undefined override's default allow does not. The same override can restrict Premium members. Group-only allows should be paired with an explicitly defined global override, or use the grant commands/file instead.

Admin changes rewrite this file in canonical form, saving its previous contents as `.bak`. A staged `.new` file is renamed into place; failed persistence does not apply the staged access. Reload manual edits before using grant commands, which operate on the loaded configuration. Check file/directory write permission if a save fails. Malformed/missing files disable file-based public/user grants and log an error; membership and explicit SourceMod overrides remain separate. Keep the `.bak` for recovery after interrupted file replacement.

Players can use `!premium_features` / `sm_premium_features` for a paged access menu. The server console lists all keys/public status. `!premium_benefits` shows only bonuses their access permits. The menu reports permission, while individual feature enable/balance settings still determine availability. No player command is needed to activate automatic bonuses or normal-chat membership tags.

## Feature keys

| Keys | Effect |
| --- | --- |
| `speed`, `gravity`, `health`, `ammo` | Class speed, gravity, spawn/max health, reserve ammunition |
| `kill_health`, `assist_health` | Separate kill and assist health rewards |
| `color`, `dark`, `glow`, `fov` | Individual cosmetic/FOV commands and settings |
| `balls`, `jarate`, `uber` | Class-appropriate spawn reserves/medigun charge |
| `respawn`, `swap` | Controlled early respawn/team swap |
| `cloak`, `deadringer` | Separate finite duration multipliers; both obey `premium_cloak` |
| `metal`, `sentry`, `dispenser`, `teleporter` | Spawn metal/capacity and separate extra-building types |
| `firearrows` | Supported bow fire arrows |
| `bhop`, `doublejump` | Held-jump/trimp and its optional follow-up jump; follow-up needs bhop/trimp too |
| `flight`, `airblast` | Separate continuous Pyro flight and right-click boost |
| `teleport` | Save/load location commands; distinct from Engineer `teleporter` buildings |
| `celebration`, `fakekill` | Optional round-win effect / actual Dead Ringer feign notification |

There is no `premium`, `all`, or chat-tag grant key. Membership remains managed through the existing membership flags/database commands and optional trials.

## Server acceptance

1. Use an authenticated account with no admin flags, database membership or trial. Keep every public grant at zero. Verify no bonuses, no chat tag, and normal class/building limits.
2. Turn trials off. Grant that user only `glow`; verify `!glow` works, membership stays inactive, and normal chat has no Premium/Trial tag. Verify unchanged ammo, health, speed, gravity, cloak, metal and buildings.
3. Revoke glow. Its effect must stop. Grant only `airblast`; right-click boost works when enabled, without continuous primary-fire flight. Grant only `dispenser`; extra dispensers work, sentry cap/metal stay ordinary, and total buildings never exceed six.
4. Test each spawn grant independently (`ammo`, `health`, `metal`, `balls`, `jarate`, `uber`). Grant/reload while alive must not top up current resources. Respawn to receive the configured bonus. Revoke it and check owned capacities/effects are restored. Revoke an unrelated feature first, then health, to check health ownership is retained until health access ends.
5. Make one feature public, then add the same user grant. Remove public access: that user keeps the feature and others lose it. Remove their user grant: it stops unless membership/explicit override supplies it. Test a second account to check grants do not leak.
6. Disconnect/reconnect, change map and restart with a user grant. It must persist. Inspect the config and backup; test a non-writable config directory on a test server and confirm a failed save does not change live grants. Restore write permission afterwards.
7. Start a trial, retain an independently granted feature, then turn trials off. The Trial tag/full trial bonuses disappear; the independent feature remains. Re-enable before the original deadline and check only original remaining time is available. After expiry, toggling trials must not revive it. Flag/database members stay members throughout.
8. Test global disable, dependency failure, feature disable, gameplay team restriction and a map flight/teleport ban against free grants. None should bypass those controls. Check root-only grant commands reject ordinary users, and explicit feature overrides respect their permission flags.
9. Repeat relevant RC11 hauling, pairing, nearby friendly wrench and cleanup checks. Native client/server prediction, rendering and TF2 feature execution still need live verification on Linux and Windows; the local VM tests do not run the game or SourceMod's native config/file implementation.
