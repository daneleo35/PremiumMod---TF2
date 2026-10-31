# RC10: native chat tags and turret tooltips

Replace `addons/sourcemod/plugins/premium.smx` with RC10. The current translations, rules and saved preferences remain compatible; the full install ZIP includes them. Preserve your custom cfg/rules/profiles. No new dependency, database change or client HUD installation is needed for this update.

## Chat tags

The missing native chat tags were caused by a format-filter bug: Premium accepted only `#TF_Chat_...`, but Valve's `CTFGameRules::GetChatFormat` returns keys such as `TF_Chat_All` without the `#`. RC10 accepts both forms and retains the original message key, text and recipient list.

1. Reload Premium and confirm `premium_version` is **3.0.0-rc10**. With `premium_chat_tags 1` and **My Premium tag: shown** in `!premium_options`, send ordinary chat. A flag/database member should display `[Premium] Dane: ...`; a trial-only account should display `[Trial]`.
2. Test all-chat, team-chat, dead-chat and spectator chat with another player where possible. The original team/dead/spectator audience must remain authoritative; Premium must not rebroadcast to everyone. Commands and plugin adverts should retain their original behavior.
3. Test hide/show tag, owner disable and approved colour presets. These choices still apply. Long name/packet limits may omit a tag to avoid truncating the original name/message. Repeating processing must not add the same tag twice.
4. If it still fails, send a normal chat message, then run `sm_premium_chat` **in your client console**. Save its line containing access, enabled/hidden state, style, processor presence, seen/tagged counters and last raw message format. `seen` counts native messages observed while the native fallback route is active; `tagged` counts successful name prefixes. Counters reset per connection/plugin load. The diagnostic is available only for your own player.

No additional chat processor is required. Existing optional processor integrations remain available, but the reported setup has only Premium and uses the corrected native route.

## Turret count and role

The always-on large white HUD overlay is removed. Aim at your own sentry within 600 units to see one short, native TF2 annotation attached above the gun:

```text
Primary | 2/3 sentries
Extra 1 (L1) | 2/3 sentries
```

Only you receive the tooltip event. Other players do not see your counter. It has no sound, ping particle, distance label or extra world entity. The team-wide extra-sentry budget remains on `!premium_benefits`; it is not added to the small tooltip.

This is the turret-attached tooltip selected during testing. The existing **Sentry Gun built by / health / Upgrade Progress** panel remains native: TF2 constructs its strings on the client from the object type, builder and upgrade state, and does not expose a server-supplied custom text field. RC10 does not rename builders or alter upgrade values to fake that panel. Font styling of the annotation ultimately follows the player's TF2 HUD.

Controls retain their existing names for configuration compatibility:

```text
premium_engineer_hud 1       // Native turret tooltip, no large overlay
premium_sentry_labels 1      // Include primary/extra role; 0 shows count only
```

The saved personal switch is now labelled **Turret tooltip** in `!premium_options`. A previously disabled counter stays disabled until the player turns this switch on.

Server checks:

1. After reload, confirm no permanent big text remains. Aim at a primary, an extra, another player's gun, a wall and empty space. Only an aimed-at own gun should have the tooltip; the standard health/upgrade panel should still work.
2. Switch between primary and extras and change the number of owned guns. Text and followed target should update without duplicate labels. Hold aim on the same gun for several seconds: one label should remain, with bounded refreshes.
3. Look away or behind a wall. The tooltip should disappear after the short native fade. Check gun destruction, hauling, class/team changes, death, expiry, disable, map change and plugin unload. The annotation also has a 2-second safety lifetime if a hide/update packet is lost.
4. Toggle owner/personal tooltip switches and the role-label setting. Team budgets, primary upgrade behavior, extra level-one caps, native health/upgrade display and normal building selection must be unchanged.
5. Test with two Engineer viewers, and high client slots if available. Events are sent directly to one client, using no client-index bit shift or 32-bit visibility mask. Check your actual HUD/theme/resolution because native annotation appearance is client-controlled.

Automated tests execute the production chat-format matcher, tag-prefix logic and tooltip state controller with fake native boundaries. They cover the actual unprefixed Valve keys, optional `#`, membership/hiding/styles/size limits, no duplicate tags, tooltip refresh throttling, target/count changes, failures, cleanup and high client slots. Rendering, native chat delivery and other engine interactions still require these live checks.

Source basis: Valve's [native chat-format selection](https://github.com/ValveSoftware/source-sdk-2013/blob/master/src/game/shared/tf/tf_gamerules.cpp), [building target-ID strings](https://github.com/ValveSoftware/source-sdk-2013/blob/master/src/game/client/tf/c_baseobject.cpp), and [native annotation handling](https://github.com/ValveSoftware/source-sdk-2013/blob/master/src/game/client/tf/tf_hud_annotationspanel.cpp).
