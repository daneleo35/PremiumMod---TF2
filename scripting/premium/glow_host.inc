float g_NextGlowWarning;

bool GlowHostAllowed(int client)
{
    return ValidClient(client) && HasFeature(client, Feature_Glow) && cvGlow.BoolValue
        && CheckCommandAccess(client, "premium_feature_glow", 0, true);
}

bool GlowHiddenCondition(TFCond condition)
{
    return condition == TFCond_Cloaked || condition == TFCond_CloakFlicker
        || condition == TFCond_DeadRingered || condition == TFCond_Disguising
        || condition == TFCond_Disguised || condition == TFCond_Stealthed
        || condition == TFCond_StealthedUserBuffFade;
}

bool GlowHostVisible(int client)
{
    return g_RoundActive && CosmeticOwnerVisible(client);
}

bool CosmeticOwnerVisible(int client)
{
    if (!ValidClient(client) || !IsPlayerAlive(client) || GetClientTeam(client) < 2) return false;
    TFCond conditions[] = {TFCond_Cloaked, TFCond_CloakFlicker, TFCond_DeadRingered,
        TFCond_Disguising, TFCond_Disguised, TFCond_Stealthed, TFCond_StealthedUserBuffFade};
    for (int i; i < sizeof(conditions); i++)
        if (TF2_IsPlayerInCondition(client, conditions[i])) return false;
    return true;
}

int GlowHostStyle(int client) { return cvPersonalGlow.BoolValue && g_Options[client].glowStyle > 0 ? g_Options[client].glowStyle - 1 : cvGlowStyle.IntValue; }
int GlowHostTeam(int client) { return GetClientTeam(client); }
float GlowHostClock() { return GetEngineTime(); }

bool GlowHostExists(int reference)
{
    if (reference == INVALID_ENT_REFERENCE) return false;
    int entity = EntRefToEntIndex(reference);
    if (entity <= MaxClients || !IsValidEntity(entity)) return false;
    char classname[48];
    GetEntityClassname(entity, classname, sizeof(classname));
    return StrEqual(classname, "info_particle_system");
}

void GlowHostDestroy(int reference)
{
    if (!GlowHostExists(reference)) return;
    int entity = EntRefToEntIndex(reference);
    // Only our own emitter is stopped, never the player's other game particles.
    AcceptEntityInput(entity, "Stop");
    RemoveEntity(entity);
}

int GlowHostCreate(int client, const char[] effect, int slot)
{
    // Preserve room for gameplay entities, and fail atomically if capacity is low.
    if (GetEntityCount() >= GetMaxEntities() - 32) return INVALID_ENT_REFERENCE;
    int entity = CreateEntityByName("info_particle_system");
    if (entity <= MaxClients || !IsValidEntity(entity)) return INVALID_ENT_REFERENCE;
    int reference = EntIndexToEntRef(entity);
    char name[64];
    FormatEx(name, sizeof(name), "premium_glow_%d_%d", GetClientSerial(client), slot);
    DispatchKeyValue(entity, "targetname", name); // Never rename the player.
    DispatchKeyValue(entity, "effect_name", effect);
    DispatchKeyValue(entity, "start_active", "0");
    if (!DispatchSpawn(entity))
    {
        GlowHostDestroy(reference);
        return INVALID_ENT_REFERENCE;
    }
    // info_particle_system::Spawn precaches its effect for the current map.
    float origin[3];
    GetClientAbsOrigin(client, origin);
    TeleportEntity(entity, origin, NULL_VECTOR, NULL_VECTOR);
    if (HasEntProp(entity, Prop_Send, "m_hOwnerEntity"))
        SetEntPropEnt(entity, Prop_Send, "m_hOwnerEntity", client);
    SetVariantString("!activator");
    if (!AcceptEntityInput(entity, "SetParent", client, entity))
    {
        GlowHostDestroy(reference);
        return INVALID_ENT_REFERENCE;
    }
    ActivateEntity(entity);
    SDKHook(entity, SDKHook_SetTransmit, PremiumCosmeticTransmit);
    if (!AcceptEntityInput(entity, "Start"))
    {
        GlowHostDestroy(reference);
        return INVALID_ENT_REFERENCE;
    }
    return reference;
}

void GlowHostFailure(int client)
{
    float now = GetEngineTime();
    if (now < g_NextGlowWarning) return;
    g_NextGlowWarning = now + 30.0;
    LogError("Premium glow could not create all emitters for client %d. Created emitters were removed; retry is delayed. Check available edicts and server particle errors.", client);
}

public void TF2_OnConditionAdded(int client, TFCond condition)
{
    if (client > 0 && client <= MaxClients && GlowHiddenCondition(condition)) { StopGlow(client); StopCelebration(client); StopSpawnEffect(client); }
}

public void TF2_OnConditionRemoved(int client, TFCond condition)
{
    if (ValidClient(client) && GlowHiddenCondition(condition)) QueueEffects(client);
}
