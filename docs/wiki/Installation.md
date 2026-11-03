# Installation and upgrades

## Requirements

| Component | Required for |
| --- | --- |
| TF2 dedicated server with SourceMod | Running the plugin; Linux or Windows server |
| SDKTools, SDKHooks and Regex | Standard SourceMod runtime functionality |
| Client Preferences | Saved player options; keep the standard clientprefs extension/plugin available |
| [TF2Attributes 1.7.5.1](https://github.com/FlaminSarge/tf2attributes/releases/tag/v1.7.5.1) | Runtime attribute support and its matching gamedata |
| [More Colors](https://github.com/DoctorMcKay/sourcemod-plugins/blob/master/scripting/include/morecolors.inc) | Compile-time header only, embedded in the compiled plugin |

You need TF2Attributes even if you only want cosmetic features. If a required part is missing or cannot load, Premium stops and reports the problem. The [developer guide](Development-and-Testing.md) lists the exact build tools.

There is no separate runtime `morecolors.smx` to install. Copying `morecolors.inc` onto a running server cannot fix a missing chat tag.

## Obtain a build

This source repository does not bundle dependency headers/runtime binaries. Build the install ZIP as described in [Development/testing](Development-and-Testing.md), or use an install archive supplied by the maintainer for the same version. Do not assume the repository's **Code → Download ZIP** is a ready-to-extract server package.

The bundled automatic build is Windows/Python based. The resulting `premium.smx` can be installed on Linux or Windows servers. No TF2 game DLL/SO replacement is required.

## Install paths

Extract the **built install ZIP** into the server's `tf/` directory. Relevant files should end up here:

```text
tf/
  addons/sourcemod/
    plugins/
      premium.smx
      tf2attributes.smx            # install separately
    gamedata/
      tf2.attributes.txt           # install separately, matching TF2Attributes
    translations/
      premium_rebuilt.phrases.txt
    configs/
      premium_features.cfg
      premium_shop.cfg
      premium_rules.cfg
  cfg/sourcemod/
    premium_rebuilt.cfg
    premium_profiles/
      cosmetic.cfg
      balanced.cfg
      fun.cfg
```

The install ZIP contains only `addons/` and `cfg/`, ready to extract into `tf/`. Source code and build tools are in the separate source ZIP; guides stay on GitHub and the Wiki. Never run the old Premium plugin and this rewrite together: their command registrations overlap.

## First start

1. Install TF2Attributes and its matching gamedata separately.
2. Review `premium_rebuilt.cfg`. The points master and purchased-trial switch default **off**; free trials default **on**, lasting 900 seconds.
3. Review `premium_features.cfg`, `premium_shop.cfg` and `premium_rules.cfg`. Public/personal free grants default off.
4. Start a test map or restart the server.
5. In server console, run:

```text
sm plugins list
sm_premium_diagnostics
premium_version
```

Check that the version is `3.0.0-rc21` and there are no Premium or TF2Attributes errors. The player database may take a moment to finish loading.

Test with an ordinary player as well as a member. An admin with full access may automatically qualify for Premium, so testing only with your own admin account will not show what everyone else gets.

## SQLite or MySQL

Default: `premium_use_mysql "0"` uses SourceMod's `storage-local` SQLite entry. Tables are created automatically; no manual SQL import is required.

For a named backend, configure an entry in `addons/sourcemod/configs/databases.cfg`, set `premium_use_mysql "1"` and `premium_database "premium"` (or your entry's name). This switch selects a named config; the implementation supports MySQL or SQLite entries. MySQL economy tables require InnoDB.

Merge the [database example](https://github.com/daneleo35/PremiumMod---TF2/blob/main/configs/premium-databases.example.txt) into your existing `databases.cfg`; do not replace the whole file or publish its credentials. Read [Storage/recovery](Storage-and-Recovery.md) before changing a backend after points have been used.

## Upgrading

RC19 has built-in chat layouts, so older valid translation files no longer make normal chat tags disappear. The translation file itself is still required for the plugin's other messages. Keep your existing configs; replacing the plugin is enough for this fix when upgrading from RC18.

1. Back up the plugin, configuration, translations, database and any points outbox files together.
2. Let pending purchases settle before changing storage settings.
3. Unload the old binary and replace the plugin/translations with the matching version.
4. Add new companion files and merge new settings into your customised main config. Preserve your values rather than overwriting them with defaults.
5. Restart/change map and run the relevant acceptance checklist.

After a translation-only replacement, reload phrases with `sm_reload_translations`. A normal full upgrade should also reload the plugin or change map/restart. The conservative multiline translation layout is intentional; do not compress phrase sections onto one line.

[Next: Configuration](Configuration.md) · [Home](Home.md)

RC20 fixes repeated building-tooltip errors when looking at doors, ammo packs or props. Upgrading from RC19 needs only the plugin file; keep your existing configs and translations.
