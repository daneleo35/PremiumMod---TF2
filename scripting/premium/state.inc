int g_BaseMaxHealth[MAXPLAYERS + 1];

bool ValidClient(int client)
{
    return client > 0 && client <= MaxClients && IsClientInGame(client);
}

bool ValidWeapon(int weapon)
{
    return weapon > MaxClients && IsValidEntity(weapon)
        && HasEntProp(weapon, Prop_Send, "m_iItemDefinitionIndex");
}

void Tell(int client, const char[] format, any ...)
{
    if (client < 0 || (client && !ValidClient(client))) return;
    char message[512], tag[64];
    SetGlobalTransTarget(client ? client : LANG_SERVER);
    VFormat(message, sizeof(message), format, 3);
    cvTag.GetString(tag, sizeof(tag));
    CReplyToCommand(client, "%s %s", tag, message);
}

int ActorSerial(int client)
{
    return client ? GetClientSerial(client) : 0;
}

int ResolveActor(int serial)
{
    if (!serial) return 0;
    int client = GetClientFromSerial(serial);
    return ValidClient(client) ? client : -1;
}

bool FlagPremium(int client)
{
    if (!ValidClient(client) || IsFakeClient(client)) return false;
    char flags[32];
    cvFlags.GetString(flags, sizeof(flags));
    int defaults = ReadFlagString(flags);
    // Empty flag configuration does not silently make every player premium.
    if (!defaults) defaults = ADMFLAG_CUSTOM1;
    return CheckCommandAccess(client, "premium_access", defaults, true);
}

bool HasPremium(int client)
{
    if (g_Stopping || !g_DependencyReady || !cvEnable.BoolValue || !ValidClient(client) || IsFakeClient(client)) return false;
    return PremiumMembership(FlagPremium(client), g_Player[client].loaded, g_Player[client].member,
        cvTrial.BoolValue, g_Player[client].trialUntil, GetTime()) || EconomyTrialActive(client);
}

bool GameplayAllowed(int client)
{
    return PerksAvailable(client) && (g_ClientFeatures[client] & PremiumGameplayFeatures()) != 0
        && PremiumTeamAllowed(GetClientTeam(client), cvGameplayTeams.IntValue);
}

bool GameplayCommandAccess(int client, const char[] overrideName)
{
    if (!FeatureAccess(client, overrideName)) return false;
    if (GameplayAllowed(client)) return true;
    Tell(client, "%t", "Gameplay Team Disabled");
    return false;
}

bool FeatureAccess(int client, const char[] overrideName)
{
    if (!ValidClient(client))
    {
        Tell(client, "%t", "In Game Only");
        return false;
    }
    int feature = StrContains(overrideName, "premium_feature_") == 0 ? PremiumFeatureIndex(overrideName[16]) : -1;
    if (feature < 0 || !HasFeature(client, view_as<PremiumFeature>(feature)))
    {
        Tell(client, "%t", "No Access");
        return false;
    }
    return true;
}

void ResetMovement(int client)
{
    g_AirblastSample[client] = false;
    g_Player[client].previousButtons = 0;
    g_Player[client].wasGrounded = false;
    g_Player[client].trimpJump = false;
    g_Player[client].flightTick = -1;
    g_Player[client].flightButtons = 0;
    for (int i; i < 3; i++) g_Player[client].previousVelocity[i] = 0.0;
}

void CancelRespawn(int client)
{
    Handle timer = g_Player[client].respawn;
    g_Player[client].respawn = null;
    // Map-scoped timers can already have been destroyed by the engine when a
    // lifecycle callback arrives. Clear ownership before cancelling anything.
    if (timer != null && IsValidHandle(timer)) delete timer;
}

void ClearClient(int client)
{
    ClearEconomyClient(client);
    g_ClientFeatures[client] = 0;
    ResetOptions(client);
    ResetChatDiagnostics(client);
    ResetPresentation(client);
    ResetGlow(client);
    PlayerState clean;
    AttributeState empty;
    g_Player[client] = clean;
    g_Speed[client] = empty;
    for (int pool; pool < 2; pool++) g_AmmoCapacity[client][pool] = empty;
    g_MetalCapacity[client] = empty;
    g_ThrowableCapacity[client] = empty;
    g_CloakSample[client] = false;
    g_FireballShot[client] = -10.0;
    g_FireballWeapon[client] = INVALID_ENT_REFERENCE;
    g_NextAirblast[client] = 0.0;
    g_BaseMaxHealth[client] = 0;
    for (int i; i < 3; i++) g_Player[client].rgb[i] = 255;
}

void InitializeClient(int client)
{
    ClearClient(client);
    SDKHook(client, SDKHook_GetMaxHealth, HookMaxHealth);
    SDKHook(client, SDKHook_PreThink, CaptureCloakDrain);
    SDKHook(client, SDKHook_PreThinkPost, ScaleCloakDrain);
    SDKHook(client, SDKHook_PreThinkPost, CaptureAirblast);
    SDKHook(client, SDKHook_PostThinkPost, ApplyAirblastBoost);
    SDKHook(client, SDKHook_PostThinkPost, ApplyFlameFlight);
    SDKHook(client, SDKHook_WeaponSwitchPost, SentryWeaponSwitched);
    ResetMovement(client);
    if (AreClientCookiesCached(client)) OnClientCookiesCached(client);
}

public Action HookMaxHealth(int client, int &maximum)
{
    if (!ValidClient(client)) return Plugin_Continue;
    g_BaseMaxHealth[client] = maximum;
    if (GameplayFeatureAllowed(client, Feature_Health) && cvHealth.IntValue > 0)
    {
        maximum += cvHealth.IntValue;
        return Plugin_Changed;
    }
    return Plugin_Continue;
}

int PlayerMaximum(int client)
{
    int maximum = g_BaseMaxHealth[client];
    if (maximum <= 0) maximum = GetEntProp(client, Prop_Data, "m_iMaxHealth");
    if (GameplayFeatureAllowed(client, Feature_Health)) maximum += cvHealth.IntValue;
    return maximum > 0 ? maximum : 1;
}

void RefreshEverybody()
{
    for (int client = 1; client <= MaxClients; client++)
        if (ValidClient(client)) RefreshClient(client);
}

void RefreshClient(int client)
{
    bool entitled = HasPremium(client);
    int features = ResolveFeatureMask(client);
    if (entitled == g_Player[client].effective && features == g_ClientFeatures[client]) return;
    g_Player[client].effective = entitled;
    g_ClientFeatures[client] = features;
    RestoreEffects(client);
    if (!HasFeature(client, Feature_Glow)) ResetGlow(client);
    if (!GameplayFeatureAllowed(client, Feature_Respawn)) CancelRespawn(client);
    if (!GameplayFeatureAllowed(client, Feature_Teleport)) g_Player[client].saved = false;
    if (!HasFeature(client, Feature_Dark)) g_Player[client].dark = false;
    MaintainExtraBuildings(client);
    ResetMovement(client);
    QueueEffects(client);
}

void QueueEffects(int client, bool spawn = false)
{
    if (!ValidClient(client)) return;
    g_Player[client].spawnPending = g_Player[client].spawnPending || spawn;
    if (g_Player[client].framePending) return;
    g_Player[client].framePending = true;
    RequestFrame(FrameEffects, GetClientSerial(client));
}

public void FrameEffects(any serial)
{
    int client = GetClientFromSerial(serial);
    if (!ValidClient(client)) return;
    g_Player[client].framePending = false;
    bool spawn = g_Player[client].spawnPending;
    g_Player[client].spawnPending = false;
    if (IsPlayerAlive(client)) ApplyEffects(client, spawn);
}
