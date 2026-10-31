# Configuration quick guide

The bundled configuration uses numbered sections, aligned values and short comments. Setting names and defaults are unchanged. Keep your own edited values when updating.

| File | Edit here for… |
| --- | --- |
| `cfg/sourcemod/premium_rebuilt.cfg` | Main switches, feature amounts, balance, points earning, trial limits, chat and storage |
| `addons/sourcemod/configs/premium_features.cfg` | Public/free features and individual Steam-ID grants |
| `addons/sourcemod/configs/premium_shop.cfg` | Shop offers, prices, rental durations and purchased-trial offers |
| `addons/sourcemod/configs/premium_rules.cfg` | Per-class overrides and map bans; most servers can leave it unchanged |

## Start with the main switches

The first section contains the plugin, free-trial, points, earning, shop, rental and purchased-trial switches. `0` means off and `1` means on unless the comment specifies another meaning. The points system and points-bought trials remain off by default.

For points-only trials, enable the points system and purchased trials, and disable the separate free trial. Set the reward amount/interval in section 03, and the lifetime purchased-trial limit in section 04. Prices and durations belong in the shop file.

Other sections group Engineer settings, health/ammo, movement/class speed, Pyro, class bonuses, team/location tools, chat/cosmetics, adverts and storage. Comments label seconds, minutes, days, percentages and multipliers.

## Free access and shop offers

In `premium_features.cfg`, a Public value of `1` gives everyone that feature's access. Personal Steam-ID entries grant only their listed features. Zero removes that source's grant; other access sources can still apply. Reload with `sm_premium_features_reload`.

In `premium_shop.cfg`, edit each offer's `enabled`, `feature`, `price`, `duration` and `unit`. Units are `minutes`, `hours` or `days`. `trial` is the full Premium trial; other feature keys rent only that feature. Reload with `sm_premium_shop_reload`.

Access does not bypass the main enable/balance settings. Granting access does not automatically refill resources. Offline/disabled time still counts toward trial and rental expiry.

## Class rules and presets

In `premium_rules.cfg`, numeric `0` inherits global settings; toggle `-1` inherits, `0` disables and `1` enables. Map bans take priority. Reload with `sm_premium_rules_reload`.

The main file ends with three **commented-out** balance preset examples. Uncomment at most one if wanted. It executes last and overrides matching values above; leaving all three commented uses your main-file values.

Changing these layout/comments requires no new plugin binary. Copy configuration files into their existing locations, keeping your custom values, and restart/change map to apply the main file. See `RC13_SERVER_TEST.md` for detailed points/trial options and recovery checks.
