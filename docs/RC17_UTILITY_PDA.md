# RC17: extra dispensers and teleporters in the normal build PDA

Corrects the report that extra sentries could be selected but dispensers/teleporters stayed “Already Built.”

## Cause and correction

The server's `CTFPlayer::GetObjectOfType` ignores `m_bDisposableBuilding`. The client's `C_TFPlayer::GetObjectOfType` does not; it only checks object type/mode and skips dormant/deleted objects. RC11–16's disposable-only workaround therefore opened the server-side limit without opening the native client menu.

RC17 keeps that server check and adds an owner-only SDKHooks transmit mask while the build-selection PDA is open. Eligible existing utilities become dormant for that owner, allowing the real client lookup to select another building. They are visible again during blueprint placement and outside the selection menu. Other clients keep normal visibility. No type/mode/builder/ownership-list rewriting or engine binary replacement is used.

Transmit fast-path flags are managed alongside the hook so it is called. Original flags are restored conditionally; later changes from another plugin are preserved. Hook failure does not leave a server-only mask behind. Existing wrench protection, native hauling, per-type limits, entity reserve and shared six slots remain in force.

## Upgrade

On RC15/RC16, replace `addons/sourcemod/plugins/premium.smx`, then reload it or change map. Keep translations/configs. Older installations also need the matching current translations for the RC15 chat fix.

## Live checks

With the dispenser/entrance/exit limits above one and fewer than six buildings, use the normal PDA to select a second dispenser, entrance and exit. Check models return when placing a blueprint, other players can always see them, native metal costs apply, and a six-slot layout refuses a seventh building. Verify upgrades, hauling and teleporter use afterwards.

Controller regression coverage: 91 building assertions, including the native client's lookup model, all utility directions, owner/other viewer separation, blueprint restoration, fast-path flags, hook failure, foreign policy changes and existing cap/ownership cases. The lookup model is a regression aid, not a running TF2 client. Python checks: 63 passed.

Sources: [Valve client lookup](https://github.com/ValveSoftware/source-sdk-2013/blob/master/src/game/client/tf/c_tf_player.cpp), [server lookup](https://github.com/ValveSoftware/source-sdk-2013/blob/master/src/game/server/tf/tf_player.cpp), [build-menu selection](https://github.com/ValveSoftware/source-sdk-2013/blob/master/src/game/client/tf/tf_hud_menu_engy_build.cpp).
