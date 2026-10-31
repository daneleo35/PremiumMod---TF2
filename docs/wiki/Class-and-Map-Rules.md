# Class rules, map bans and presets

Use the main cfg for general values, `premium_rules.cfg` for per-class overrides and map restrictions, and editable preset cfgs for grouped balance changes. Rules never create player access.

## Per-class settings

`tf/addons/sourcemod/configs/premium_rules.cfg` contains all nine class sections. Change values inside an existing section; do not add a second duplicate class block. Example Pyro section:

```text
"pyro"
{
    "ammo_multiplier"     "2.0"
    "speed_multiplier"    "1.0"
    "gravity_multiplier"  "1.0"
    "bhop"                "0"
    "bhop_gain"           "0"
    "bhop_cap"            "0"
    "doublejump"          "0"
    "flight"              "1"
    "airblast"            "1"
    "flight_speed"        "350"
    "flight_lift"         "150"
    "flight_acceleration" "500"
    "teleport"            "-1"
}
```

Numeric `0` inherits the corresponding global setting. Toggle values use `-1` to inherit, `0` to disable and `1` to enable. To choose normal ammo/speed/gravity explicitly, use `1.0`, not zero. The example doubles native reserve capacity, disables bhop/follow-up jumps and specifies Pyro flight values. Fuel is supplied on spawn, not continuously refilled.

## Map restrictions

Within the existing `Maps` block:

```text
"jump_*"
{
    "flight"   "0"
    "teleport" "0"
}
"cp_example"
{
    "flight"   "0"
}
```

Names match exactly or by a trailing `*` prefix pattern. Workshop base names are supported. A matching flight ban blocks both flame flight and airblast boost. `teleport` here concerns saved locations, not Engineer buildings. Matching bans win over class/global settings.

Reload with `sm_premium_rules_reload`. Missing/invalid rules fail closed for flight/saved-location teleport; last good class settings are retained. Correct the file and reload rather than ignoring the error.

## Balance presets

The editable cfgs live in `tf/cfg/sourcemod/premium_profiles/`:

| Preset | Intended use |
| --- | --- |
| `cosmetic` | Disable gameplay advantages while retaining configured cosmetics |
| `balanced` | Restrained gameplay values and a smaller extra-sentry allowance |
| `fun` | Stronger movement/resources and a larger extra-sentry allowance |

Read/edit the actual cfg before selecting a preset; its contents define the values. Select one with:

```text
sm_premium_profile balanced next
sm_premium_profile status
sm_premium_profile cancel
sm_premium_profile fun now
```

Omitting `next`/`now` queues the change for the next round. `now` applies immediately and can update existing effects/buildings. Presets do not rewrite membership, grants, economy or storage. Class overrides and map bans remain applicable.

For persistence across map/server restarts, uncomment exactly one preset `exec` at the **end** of `premium_rebuilt.cfg`. Otherwise use its normal values. A runtime selection does not rewrite your main config.

[Configuration](Configuration.md) · [All defaults](Configuration-Reference.md) · [Home](Home.md)
