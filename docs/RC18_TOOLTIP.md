# RC18: stable building tooltip

Corrects the building tooltip repeatedly moving from the left of the screen back above the building.

The old controller re-sent `show_annotation` every 0.75 seconds. TF2 calls `CTFAnnotationsPanelCallout::Touch()` on every show event, which resets the position to the screen's upper-left region and restarts its slide-in. The 2.5-second interpolation could not settle before the next refresh.

The tooltip now uses the native persistent lifetime and sends a show event only when its target or displayed text changes. The client already follows the building every frame. Looking away, losing access, death, disconnect, map/plugin cleanup and HUD round resets still release/reset ownership explicitly.

Labels are shorter and split across two lines. Example:

```text
Extra 1 (L1)
Sentries 2/3 | Total 3/6
```

`Total` counts all owned buildings against the shared six slots. The existing tooltip/label settings and player preference still apply.

## Upgrade

On RC17, replace `addons/sourcemod/plugins/premium.smx` and reload the plugin or change map. Keep configs/translations. Older installations still need the matching translations introduced by RC15.

Checks: 24 tooltip assertions, including 240 consecutive unchanged heartbeats without a repeated show, target/text changes, cleanup, event failure/recovery, high client slots and compact labels; 63 Python checks; warnings-as-errors release compilation. Actual HUD drawing remains a client/server check.

Source: [Valve's annotation controller](https://github.com/ValveSoftware/source-sdk-2013/blob/master/src/game/client/tf/tf_hud_annotationspanel.cpp).
