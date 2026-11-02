int g_SpawnRefs[MAXPLAYERS + 1];
float g_SpawnUntil[MAXPLAYERS + 1];

void StopSpawnEffect(int client)
{
    int reference = g_SpawnRefs[client];
    g_SpawnRefs[client] = INVALID_ENT_REFERENCE; g_SpawnUntil[client] = 0.0;
    GlowHostDestroy(reference);
}

void StopSpawnEffects()
{
    for (int client = 1; client <= MaxClients; client++) StopSpawnEffect(client);
}

void RefreshSpawnEffects()
{
    int count;
    for (int client = 1; client <= MaxClients; client++)
    {
        if (!GlowHostExists(g_SpawnRefs[client])) continue;
        if (!cvSpawnEffect.BoolValue || !g_RoundActive || !HasPremium(client)
            || !CosmeticOwnerVisible(client) || GetEngineTime() >= g_SpawnUntil[client]
            || count >= cvSpawnBudget.IntValue) StopSpawnEffect(client);
        else count++;
    }
}

public void FrameMemberSpawnEffect(any serial)
{
    int client = GetClientFromSerial(serial);
    if (!ValidClient(client)) return;
    StopSpawnEffect(client);
    if (!cvSpawnEffect.BoolValue || !g_RoundActive || !HasPremium(client) || !CosmeticOwnerVisible(client)) return;
    RefreshSpawnEffects();
    int count;
    for (int viewer = 1; viewer <= MaxClients; viewer++) if (GlowHostExists(g_SpawnRefs[viewer])) count++;
    if (count >= cvSpawnBudget.IntValue) return;
    bool red = GetClientTeam(client) == 2;
    char effect[64];
    if (cvSpawnStyle.IntValue == 1) strcopy(effect, sizeof(effect), red ? "critical_grenade_red" : "critical_grenade_blue");
    else strcopy(effect, sizeof(effect), red ? "player_recent_teleport_red" : "player_recent_teleport_blue");
    g_SpawnRefs[client] = GlowHostCreate(client, effect, 5);
    g_SpawnUntil[client] = GetEngineTime() + cvSpawnSeconds.FloatValue;
}
