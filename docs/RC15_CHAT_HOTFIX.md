# RC15: colour codes appearing before chat tags

Fixes the report where normal chat showed `3EFF3E[Premium]` instead of a coloured tag. The native fallback was placing More Colors control bytes inside TF2's player-name parameter; the client filters that field. The prefix now belongs to the chat layout instead.

## Install

Replace both files from the RC15 install ZIP:

- `addons/sourcemod/plugins/premium.smx`
- `addons/sourcemod/translations/premium_rebuilt.phrases.txt`

Keep your configs/database. Reload translations and the plugin, or restart/change map. If upgrading hot, use `sm_reload_translations`, then `sm plugins reload premium`.

Chat tags remain automatic. No new command or setting is required. Test ordinary and team chat, dead/spectator chat and the available tag styles.

## Behaviour

- Tag/name colours are in the layout; original player name, message and location parameters are unchanged.
- Keeps the original speaker, chat flag, recipient list and reliability.
- Layout labels use the recipient's translation language. English is provided; additional languages can translate the eight `Chat Layout ...` phrases, retaining `@NAME@`, `@TEXT@` and, where present, `@LOCATION@`.
- Unknown chat formats, missing layout phrases or an insufficient message budget fall back to the unchanged original message.
- Existing chat-processor integrations continue through their existing forwards.

Checks: release compilation with warnings as errors, 67 production chat-controller/layout assertions and 63 Python checks. The colour mock now matches More Colors' real `green` expansion rather than hiding RGB behaviour behind a one-byte colour.

Reference: [Valve's chat parameter filtering and layout parsing](https://github.com/ValveSoftware/source-sdk-2013/blob/master/src/game/client/hud_basechat.cpp).
