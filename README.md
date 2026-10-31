# Premium Mod for Team Fortress 2

Give players extra TF2 features, choose who gets them, and keep the balance in your hands.

Give members a Premium package, make a few features free for everyone, or let players earn points and spend them in a shop. You choose the bonuses, prices, time limits and restrictions. Points are optional.

**Current version: 3.0.0-rc20**

## What can it do?

- Give players more spare ammo, including Pyro fuel, extra health and more starting metal.
- Let Pyros fly while firing, or jump with a boost from right-click airblast.
- Let Engineers build extra sentries, dispensers and teleporters using the normal build menu.
- Change movement speed, gravity and cloak duration, with separate settings for each class.
- Add automatic Premium/Trial chat tags, glow, colours and a small building tooltip.
- Offer free trials, or let players buy a timed Premium trial with points.
- Award points for time played and sell individual features for hours or days.
- Turn features off on particular maps and switch balance presets between rounds.

**A few things to know:** Engineers have six building slots in total. A teleporter pair uses two. Extra sentries stay at level one. Extra metal is given on spawn; it does not refill itself. The ammo setting increases spare ammo and fuel, not the number of shots in a magazine.

## Read the guide

Start with the [Wiki](https://github.com/daneleo35/PremiumMod---TF2/wiki). A copy of every guide is also [saved in this repository](docs/wiki/Home.md).

| What you need | Guide |
| --- | --- |
| Get it running | [Installation](docs/wiki/Installation.md) |
| Understand who gets what | [How it works](docs/wiki/How-It-Works.md) |
| Know what players can type | [Player guide](docs/wiki/Player-Guide.md) |
| Use the player, shop and admin menus | [In-game menus](docs/wiki/In-Game-Menus.md) |
| Look up a command | [Commands](docs/wiki/Commands.md) |
| Change the settings | [Configuration](docs/wiki/Configuration.md) |
| Give free features or manage trials | [Access and trials](docs/wiki/Access-and-Trials.md) |
| Set up points, prices and purchase limits | [Points and shop](docs/wiki/Points-and-Shop.md) |
| See every available feature | [Features](docs/wiki/Features.md) |
| Understand the building limits | [Engineer buildings](docs/wiki/Engineer-Buildings.md) |
| Change rules by class or map | [Class and map settings](docs/wiki/Class-and-Map-Rules.md) |
| Find a setting's default or limit | [All settings](docs/wiki/Configuration-Reference.md) |
| Fix a problem | [Troubleshooting](docs/wiki/Troubleshooting.md) |
| Back up player data | [Storage and recovery](docs/wiki/Storage-and-Recovery.md) |
| Build or test the source | [Developer guide](docs/wiki/Development-and-Testing.md) |

## Install it

You need a TF2 server running SourceMod, plus [TF2Attributes and its matching gamedata](https://github.com/FlaminSarge/tf2attributes/releases/tag/v1.7.5.1). Install TF2Attributes separately.

This repository contains the source. GitHub's **Download ZIP** button does not give you a ready-to-install plugin. To build an install ZIP on Windows, install Git and Python 3.10 or newer, then run:

```powershell
git clone https://github.com/daneleo35/PremiumMod---TF2.git
cd PremiumMod---TF2
python tools/bootstrap.py
python build.py
```

The first script gets the tools needed to build it. The second creates the plugin and install ZIP in `dist/`. The resulting plugin works with supported Linux or Windows TF2 servers; the supplied build script runs on Windows.

Extract the **built install ZIP** into your server's `tf/` folder. Remove the old Premium plugin first. Open `tf/cfg/sourcemod/premium_rebuilt.cfg`, choose your settings, then restart the server or change map. Keep your existing player data and customised settings when upgrading.

The [installation guide](docs/wiki/Installation.md) shows where each file goes and what to check on first start. [More Colors](https://github.com/DoctorMcKay/sourcemod-plugins/blob/master/scripting/include/morecolors.inc) is needed to compile the plugin; the first script downloads it for you. You do not need to load it as a separate plugin on the server.

## A few useful examples

### Give someone Premium

In the server console, or as an admin with full access:

```text
sm_premadduser "Dane"
```

Use a player name or Steam ID. To remove that saved membership, use `sm_premremuser`. Someone who also gets Premium through their admin flags or an active trial may still have it.

### Make one feature free

```text
sm_premium_feature_public airblast 1
sm_premium_feature "Dane" glow 1
```

The first gives everyone airblast boost. The second gives Dane glow. They get those features without becoming Premium or getting a Premium tag. The feature must also be switched on in the server settings. Change `1` to `0` to remove that grant.

### Give five points every ten minutes

In `tf/cfg/sourcemod/premium_rebuilt.cfg`:

```cfg
premium_points_enable "1"
premium_points_earning "1"
premium_points_interval_minutes "10"
premium_points_per_interval "5"
premium_points_shop "1"
premium_points_rentals "1"
```

Set the offers, prices and lengths in `premium_shop.cfg`. Players type `!premium_points` to check their points and `!premium_shop` to browse. Short `!points` and `!shop` commands also work if another plugin has not already taken them.

To sell Premium trials too, add:

```cfg
premium_points_trials "1"
premium_points_trial_limit "3"
```

That allows three trial purchases per player in total. Set `premium_trial "0"` if you want to stop the separate free trial. **Time keeps counting while players are offline.** Buying glow gives only glow; buying a Premium trial gives the whole configured Premium package and a Trial tag.

Points and points-bought trials start **off**. Free trials start **on**, lasting 15 minutes. Read [how to apply config changes](docs/wiki/Configuration.md) after editing.

## For players

Type `!premium` to open the menu. It shows your status and points, with separate screens for bonuses, unlocked features, appearance, purchases, actions and personal settings. Admins can open `!premium_admin` to manage players and server settings through the menus.

The shop groups offers by category and shows the price, length, remaining balance and any reason you cannot buy. Admin changes have confirmation screens; settings can be applied live and explicitly saved to the main config with a backup.

Premium and Trial tags appear automatically in normal chat.

To give them your own names and colours, edit `cfg/sourcemod/premium_rebuilt.cfg`:

```cfg
premium_chat_tag       "{gold}[VIP]{default}"
premium_trial_chat_tag "{aqua}[Guest]{default}"
```

Use `premium_tag_styles "0"` if everyone should keep your chosen colours. Apply the file with `exec sourcemod/premium_rebuilt.cfg`. See [custom chat tags](docs/wiki/Configuration.md#custom-chat-tags) for more examples and the saved-menu settings note.

## Credits and licence

Original concept: **daneleo and noodleboy347**. See [LICENSE.txt](LICENSE.txt) and [THIRD-PARTY.txt](THIRD-PARTY.txt) for the licence and dependency credits. TF2Attributes and More Colors come from their original authors and are not bundled as separate downloads in this repository.
