# RC20: safe building tooltip targets

Fixed repeated SourceMod errors when an Engineer looked at ammo packs, spawn-room barriers, doors or map props:

```text
Property "m_hBuilder" not found
OwnedUtilityKind -> TooltipHostTarget -> RefreshTurretTooltip -> Heartbeat
```

The tooltip checked ownership before checking whether its aim target was a dispenser or teleporter. Those unrelated entities have no building ownership property. The utility lookup now checks the entity reference and class first, then verifies the required ownership and teleporter-mode properties before reading them. Non-buildings are ignored. The utility transmit callback uses the same guarded lookup, and an invalid utility kind cannot match a non-building.

Regression coverage uses a strict property host that throws on missing building properties. Before the fix it reproduced the reported ammo-pack exception; after the fix all 119 building assertions pass. Coverage includes all four reported classes, own and foreign utilities, entrance/exit modes, missing properties, invalid references and an edict reused by an ammo pack. The existing 24 tooltip assertions and 63 Python checks also pass. Release compilation treats warnings as errors.

The separate `prop_dynamic` / `m_vecNormal` DataTable warning at the end of the supplied log has no Premium stack trace. Premium neither creates `prop_dynamic` entities nor writes `m_vecNormal`; this release does not claim to resolve that separate warning.

## Upgrade

Replace only `addons/sourcemod/plugins/premium.smx`, then run `sm plugins reload premium` or change map. Existing configuration, translations, points and membership data can stay.
