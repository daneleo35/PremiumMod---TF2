// Reserve capacity is attached to the player, as CTFPlayer::GetMaxAmmo expects.
// Both normal reserve pools are covered, including clipless flamethrower fuel.
// Metal and grenade/effect-bar resources have their own independent settings.
AttributeState g_AmmoCapacity[MAXPLAYERS + 1][2];
char g_AmmoAttributes[][] = {"maxammo primary increased", "maxammo secondary increased"};
char g_AmmoHooks[][] = {"mult_maxammo_primary", "mult_maxammo_secondary"};

void RestoreAmmoCapacity(int client)
{
    for (int pool; pool < 2; pool++) RestoreAttribute(g_AmmoCapacity[client][pool], g_AmmoAttributes[pool]);
}

void MaintainAmmoCapacity(int client)
{
    if (!GameplayFeatureAllowed(client, Feature_Ammo) || !IsPlayerAlive(client)) { RestoreAmmoCapacity(client); return; }
    for (int pool; pool < 2; pool++)
        ApplyAttribute(client, g_AmmoCapacity[client][pool], g_AmmoAttributes[pool], EffectiveAmmoMultiplier(client));
}

void GiveSpawnAmmo(int client)
{
    if (!GameplayFeatureAllowed(client, Feature_Ammo) || !IsPlayerAlive(client) || EffectiveAmmoMultiplier(client) <= 1.0) return;
    bool supplied[2];
    for (int slot; slot < 3; slot++)
    {
        int weapon = GetPlayerWeaponSlot(client, slot);
        if (!ValidWeapon(weapon) || !HasEntProp(weapon, Prop_Send, "m_iPrimaryAmmoType")) continue;
        int ammo = GetEntProp(weapon, Prop_Send, "m_iPrimaryAmmoType");
        int pool = PremiumReserveAmmoIndex(ammo, GetEntPropArraySize(client, Prop_Send, "m_iAmmo"));
        if (pool < 0 || supplied[pool] || !g_AmmoCapacity[client][pool].owned) continue;
        supplied[pool] = true;
        // This native clamps to TF2's actual maximum, respecting the class,
        // equipped item modifiers and our capacity. No class-ammo table, clip
        // writes, regeneration call or heartbeat refill is needed.
        GivePlayerAmmo(client, 1000000, ammo, true);
    }
}
