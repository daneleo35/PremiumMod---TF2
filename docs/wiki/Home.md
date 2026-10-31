# Premium Mod for TF2

Premium Mod lets you choose extra features for your TF2 server and decide who can use them. Give members a Premium package, make selected features free, or let players earn points and buy time with a feature.

You decide how generous it is. Keep it mostly cosmetic, add small bonuses, or turn up the settings for a fun server.

This guide covers **3.0.0-rc15**.

## Where to start

| I want to… | Open this |
| --- | --- |
| Install it on my server | [Installation](Installation.md) |
| Understand who gets which features | [How it works](How-It-Works.md) |
| Find out what to type as a player | [Player guide](Player-Guide.md) |
| Use the player, shop and admin menus | [In-game menus](In-Game-Menus.md) |
| Look up player and admin commands | [Commands](Commands.md) |
| Change the settings | [Configuration](Configuration.md) |
| Give free features or set up trials | [Access and trials](Access-and-Trials.md) |
| Set up points and shop prices | [Points and shop](Points-and-Shop.md) |
| See the full feature list | [Features](Features.md) |
| Set up extra Engineer buildings | [Engineer buildings](Engineer-Buildings.md) |
| Change settings for a class or map | [Class and map settings](Class-and-Map-Rules.md) |
| Fix something that is not working | [Troubleshooting](Troubleshooting.md) |

For a particular setting, use [All settings](Configuration-Reference.md). Server owners can also read [Backing up player data](Storage-and-Recovery.md). Building the source and detailed test information live in the [Developer guide](Development-and-Testing.md).

## The bits that are easy to mix up

- **Premium** gives the player the whole package you have enabled, plus a Premium chat tag.
- **A free feature or feature rental** gives only that feature. It does not make the player Premium.
- **A Premium trial** gives the package and a Trial tag until its time runs out.
- **Free trials and points-bought trials are separate.** You can turn either one off.
- **Engineers can own six buildings in total.** A teleporter pair takes two slots. Extra sentries stay at level one.
- **Extra ammo means more spare ammo and fuel.** Magazines stay the same size. Extra metal is starting metal, not an automatic refill.
- **Trial and rental time keeps counting offline.** Turning a feature off does not pause the clock.

Start players with `!premium`; use `!premium_benefits` to see their actual bonuses. Chat tags work automatically for members and active Premium trials.

[Source and downloads information](https://github.com/daneleo35/PremiumMod---TF2) · [Report a problem](https://github.com/daneleo35/PremiumMod---TF2/issues)
