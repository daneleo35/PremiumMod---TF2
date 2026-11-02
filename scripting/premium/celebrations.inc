int g_CelebrationRefs[MAXPLAYERS + 1];
int g_WinningTeam;
float g_CelebrationUntil;

void StopCelebration(int client)
{
    int reference = g_CelebrationRefs[client]; g_CelebrationRefs[client] = INVALID_ENT_REFERENCE;
    GlowHostDestroy(reference);
}

void StopCelebrations()
{
    for (int client = 1; client <= MaxClients; client++) StopCelebration(client);
    g_CelebrationUntil = 0.0; g_WinningTeam = 0;
}

void BeginCelebration(int team)
{
    if (team < 2 || team > 3 || !cvCelebration.BoolValue || g_CelebrationUntil > GetEngineTime()) return;
    g_WinningTeam = team;
    g_CelebrationUntil = GetEngineTime() + cvCelebrationSeconds.FloatValue;
}

void RefreshCelebrations()
{
    bool active = !g_RoundActive && cvCelebration.BoolValue && GetEngineTime() < g_CelebrationUntil;
    int count;
    for (int client = 1; client <= MaxClients; client++)
    {
        if (!active || !HasFeature(client, Feature_Celebration) || GetClientTeam(client) != g_WinningTeam || !CosmeticOwnerVisible(client)
            || count >= cvCelebrationBudget.IntValue)
        { StopCelebration(client); continue; }
        if (!GlowHostExists(g_CelebrationRefs[client]))
            g_CelebrationRefs[client] = GlowHostCreate(client, g_WinningTeam == 2 ? "player_recent_teleport_red" : "player_recent_teleport_blue", 4);
        if (GlowHostExists(g_CelebrationRefs[client])) count++;
    }
}

void RebuildPremiumCosmetics()
{
    // Delete existing instances so opting out also stops already-running client
    // emitters. The shared heartbeat recreates permitted emitters with transmit filtering.
    for (int client = 1; client <= MaxClients; client++) { StopGlow(client); StopCelebration(client); StopSpawnEffect(client); }
}
