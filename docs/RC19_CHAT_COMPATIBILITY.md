# RC19: chat tags with older translation files

Addresses the report where RC18 showed bare player chat despite `premium=1 enabled=1 hidden=0`, with the native format recognised and prepared tags counted.

The prepared-prefix counter did not mean a tagged message had been sent. RC15's native renderer required the new `Chat Layout ...` translation phrases. An older installed/cached file caused layout creation to fail and the original untagged message to be sent instead.

RC19 includes built-in layouts for all eight supported TF2 chat contexts. Optional valid translations still take priority; missing phrases or invalid layout markers now use the built-in layout rather than silently dropping the tag. Colours stay in the layout and player text stays in the original message parameters, retaining the RC15 colour correction.

Membership, the server enable switch, personal hidden-tag choices and deliberately empty tag text still apply. Unknown native formats and insufficient packet budgets still preserve original chat.

## Upgrade

From RC18, replace `addons/sourcemod/plugins/premium.smx` and run `sm plugins reload premium`, or change map. Existing configs and valid translation files can stay. The core translation file is still needed for the plugin's other messages; this change removes the requirement for its new optional chat-layout phrases.

## Diagnostics

`!premium_chat` remains diagnostic only. It now includes:

- Prefixes prepared (the existing `tagged` field).
- Tagged sends to recipients.
- Built-in layout uses.
- Last preparation/send/skip result, including a packet-budget or message-creation failure.

Checks: 89 production chat assertions, including all eight contexts with layout translations unavailable, invalid optional layouts, RGB colour placement, original placeholders, disabled/hidden/nonmember behaviour, capacity limits and diagnostic reset. Python checks: 63 passed. Release compilation treats warnings as errors.
