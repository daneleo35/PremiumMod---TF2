# RC16: downward-only airblast jump

The Premium airblast jump now requires looking down. Level or upward aim performs normal airblast without added jump/recoil impulse.

The angle is checked at capture and again before applying the boost. Turning down after a level blast cannot create a delayed boost. Looking down means below the horizon (positive Source eye pitch); no new setting is needed.

Normal airblast fuel costs, reflection and weapon cooldowns remain unchanged. Rejected angles do not consume the Premium boost cooldown or detach a grounded player. Primary-fire flight is unchanged.

Upgrade an existing RC15 installation by replacing `addons/sourcemod/plugins/premium.smx`. Keep its translations and your configs. Earlier versions still need RC15's matching translation update for coloured chat tags.

Verification: warnings-as-errors release compile, 35 airblast controller/policy assertions and 63 Python checks.
