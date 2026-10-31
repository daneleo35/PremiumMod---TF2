# Membership, individual grants and trials

## Permanent membership

The default membership permission is `premium_access`, using `premium_flags "o"` unless overridden. The plugin checks access; it never grants/replaces a player's SourceMod admin flags.

Membership can also be stored in the database:

```text
sm_premadduser "Dane"
sm_premremuser "STEAM_1:0:123456"
```

Removing database membership removes only that source. Flags or an unexpired trial can still make the user a member. Use a clean ordinary account when testing who gets access; root accounts generally pass SourceMod permission checks.

## Individual access without membership

Use `premium_features.cfg` or server/admin commands:

```text
sm_premium_feature_public airblast 1
sm_premium_feature "STEAM_1:0:123456" glow 1
sm_premium_feature "STEAM_1:0:123456" glow 0
```

The user gains only the selected feature. No membership, player-chat tag, join/welcome announcement or unrelated bonus is created. Grant amounts/enables remain controlled by the main configuration and class/map/team rules.

Manual example:

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
        }
    }
}
```

Reload manual changes with `sm_premium_features_reload`. All omitted keys default to no grant. `0` removes that source's grant; it is not an absolute denial of membership/public/rental/override access. Commands rewrite the file in a standard layout and retain a backup, so reload manual edits before using them.

Steam2, Steam3 and Steam64 IDs are accepted and converted to Steam3. At most 1024 personal records are accepted. Malformed/missing files disable file grants and log an error; membership and explicit SourceMod overrides remain separate.

## Feature permission overrides

Each key has a `premium_feature_<key>` permission check. For example, `premium_feature_glow` corresponds to `glow`.

Premium members normally pass undefined feature checks. An undefined override's default allow **does not** grant free features to a non-member. An explicitly defined global override can independently authorise a user who passes it, while also restricting Premium members who fail it.

For example, an owner can define `premium_feature_glow` with an appropriate SourceMod flag in `admin_overrides.cfg`; passing that explicit permission supplies glow access without changing membership. Remember that existing membership flags may independently make those same admins Premium. Group-only allows should be paired with an explicitly defined global override, or use the simpler grant file/commands.

Feature permission denials still restrict public/personal/rental sources. Read the [feature-key list](Features.md) before configuring permissions; `teleporter` and `teleport` are different features.

## Free trials

```cfg
premium_trial "1"
premium_trial_timelimit "900"
```

Players request their one-time free trial with `!trial`. The database stores used status and an saved expiry time. Offline time counts. Reconnects, map changes and reloads do not reset it.

`premium_trial "0"` blocks new claims and stops existing free-trial access without deleting its history. Re-enabling before the original expiry restores only the remaining time; an expired trial cannot be revived by toggling it.

Server/admin commands `sm_createtrial` and `sm_resettrial` manage the free-trial record. Creating one while free trials are disabled stores a record but does not bypass the disable switch. Resetting it does not change points-bought trial counts.

## Points-bought trials

These have their own switch, expiry and lifetime purchase counter:

```cfg
premium_points_enable "1"
premium_points_trials "1"
premium_points_trial_limit "3"
```

An enabled shop offer with `feature "trial"` sells a timed Premium bundle and Trial tag. The limit covers successful purchases per account across all trial offers. `0` means unlimited. Active Premium/trial users cannot buy another trial while that membership is active.

Free trials may remain disabled while purchased trials are enabled. Points purchases do not reset free-trial eligibility. Reconnect/reload/`resettrial` cannot reset purchased-trial history. Individual feature rentals remain non-membership access.

[Points and shop](Points-and-Shop.md) · [How it works](How-It-Works.md) · [Home](Home.md)
