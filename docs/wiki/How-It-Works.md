# How it works

You choose **who gets a feature**, **whether it is switched on**, and **how much it changes the game**. These are separate choices.

For example, giving someone extra ammo lets them use the ammo bonus. The multiplier in your settings decides how much they get. If you set it to `1`, they still have the normal amount.

## Three ways to give players something

| Choice | What the player gets | Chat tag? |
| --- | --- | --- |
| Premium membership | All the features you have enabled for members | Premium |
| A free feature or points-bought feature | Just that feature | No |
| A free or points-bought Premium trial | Your Premium package until the trial ends | Trial |

Players can hide their tag, and owners can turn tags off. Hiding a tag does not remove the player's features.

### Give someone Premium

Use `sm_premadduser` with their name or Steam ID. You can also use SourceMod admin flags; the default is the `o` flag. Details are in [Access and trials](Access-and-Trials.md).

### Give only selected features

Use `sm_premium_feature_public` to give a feature to everyone, or `sm_premium_feature` to give it to one player. This is useful if you want free airblast jumps or want to reward someone with glow without giving them everything else.

### Let players earn their features

Turn on the optional points system. Players earn points for time played, then choose an offer in the shop. You set the price and how long it lasts. See [Points and shop](Points-and-Shop.md).

## Server rules still apply

A player only gets a feature when the server settings allow it. Giving access does not ignore map bans, team/class restrictions, cooldowns or building limits.

- A Pyro cannot use flight on a map where you have banned it.
- Extra dispensers still share the Engineer's six building slots.
- A player with a glow rental still needs to type `!glow` to turn it on.
- Buying extra ammo does not refill a weapon immediately. They get the starting bonus on their next normal spawn.

Use `!premium_features` to see which features a player has access to. Use `!premium_benefits` to see the bonuses their current class actually gets.

## When do bonuses appear?

| Bonus | When it happens |
| --- | --- |
| Extra starting ammo, health, metal or Medic charge | On a normal spawn |
| Speed, gravity, capacity and display settings | Updated as the player's settings/access change |
| Health for kills or assists | After a qualifying kill or assist |
| Ball and jar recharge | One at a time, through TF2's normal recharge |
| Glow or colour | When the player chooses it |
| Points or a shop purchase | After the save succeeds; a purchase may take a moment |

There is no constant ammo or metal refill.

## What happens when time runs out?

Trials and rentals use real time, including time offline. Reconnecting or changing maps does not start them again. Disabling the system does not pause their clock.

Players can get the same feature in more than one way. If a glow rental expires but you have also given them free glow, they keep it. Removing saved Premium membership also does not remove Premium they still get from an admin flag or active trial.

Saved membership, trials, points and purchases survive ordinary restarts. Personal display options are saved too. Saved teleport locations are temporary and are lost when the player changes life, team, map or connection.

[Set up access](Access-and-Trials.md) · [Change the settings](Configuration.md) · [Home](Home.md)
