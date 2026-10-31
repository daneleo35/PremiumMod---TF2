// CTFPlayer::PreThink starts with UpdateTimers -> SharedThink -> UpdateCloakMeter.
// Pair SDKHooks around that call so activation fees in ItemPostFrame and ordinary
// recharge/pickups are never refunded. This also preserves other items' drain modifiers.
bool g_CloakSample[MAXPLAYERS + 1];
float g_CloakBefore[MAXPLAYERS + 1];
int g_CloakWatch[MAXPLAYERS + 1];

float CloakMultiplier(int client)
{
    if (!GameplayAllowed(client) || !cvCloak.BoolValue || !IsPlayerAlive(client)
        || TF2_GetPlayerClass(client) != TFClass_Spy) return 1.0;
    int watch = GetPlayerWeaponSlot(client, 4);
    if (!ValidWeapon(watch)) return 1.0;
    bool deadRinger = GetEntProp(watch, Prop_Send, "m_iItemDefinitionIndex") == 59;
    if (!GameplayFeatureAllowed(client, deadRinger ? Feature_DeadRinger : Feature_Cloak)) return 1.0;
    return deadRinger ? cvDeadRingerMultiplier.FloatValue : cvCloakMultiplier.FloatValue;
}

public void CaptureCloakDrain(int client)
{
    g_CloakSample[client] = CloakMultiplier(client) > 1.0 && TF2_IsPlayerInCondition(client, TFCond_Cloaked);
    if (!g_CloakSample[client]) return;
    g_CloakBefore[client] = GetEntPropFloat(client, Prop_Send, "m_flCloakMeter");
    g_CloakWatch[client] = EntIndexToEntRef(GetPlayerWeaponSlot(client, 4));
}

public void ScaleCloakDrain(int client)
{
    bool sampled = g_CloakSample[client];
    g_CloakSample[client] = false;
    if (!sampled || !TF2_IsPlayerInCondition(client, TFCond_Cloaked)) return;
    float multiplier = CloakMultiplier(client);
    int watch = GetPlayerWeaponSlot(client, 4);
    if (multiplier <= 1.0 || !ValidWeapon(watch) || EntIndexToEntRef(watch) != g_CloakWatch[client]) return;
    float after = GetEntPropFloat(client, Prop_Send, "m_flCloakMeter");
    float scaled = PremiumCloakAfterDrain(g_CloakBefore[client], after, multiplier);
    // Never re-add invisibility or postpone the native terminal fade. At most
    // the multiplier times one simulation tick is lost; the meter still declines to empty.
    if (scaled > after) SetEntPropFloat(client, Prop_Send, "m_flCloakMeter", scaled);
}
