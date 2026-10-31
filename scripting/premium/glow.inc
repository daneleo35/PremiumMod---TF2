// Particle ownership/controller. Host operations live in glow_host.inc so this
// exact state machine can be executed with fault-injected engine boundaries.
#define PREMIUM_GLOW_PARTICLES 4

enum struct PremiumGlowState
{
    bool requested;
    int references[PREMIUM_GLOW_PARTICLES];
    int team;
    int style;
    float retryAfter;
}

PremiumGlowState g_Glow[MAXPLAYERS + 1];
char g_GlowEffects[PREMIUM_GLOW_PARTICLES][] = {
    "player_recent_teleport_blue", "player_recent_teleport_red",
    "critical_grenade_blue", "critical_grenade_red"
};

void StopGlow(int client)
{
    for (int slot; slot < PREMIUM_GLOW_PARTICLES; slot++)
    {
        int reference = g_Glow[client].references[slot];
        // Clear ownership before an engine operation can invoke another callback.
        g_Glow[client].references[slot] = INVALID_ENT_REFERENCE;
        GlowHostDestroy(reference);
    }
    g_Glow[client].team = 0;
    g_Glow[client].style = -1;
}

void ResetGlow(int client)
{
    g_Glow[client].requested = false;
    g_Glow[client].retryAfter = 0.0;
    StopGlow(client);
}

bool RefreshGlow(int client)
{
    if (!g_Glow[client].requested)
    {
        // Usually all references are already empty; this also repairs stale state.
        StopGlow(client);
        return false;
    }
    if (!GlowHostAllowed(client))
    {
        ResetGlow(client);
        return false;
    }
    if (!GlowHostVisible(client))
    {
        StopGlow(client); // Keep preference while dead, cloaked, or disguised.
        return false;
    }
    int style = GlowHostStyle(client);
    int team = GlowHostTeam(client);
    int expected = style == 0 ? PREMIUM_GLOW_PARTICLES : (style == 2 ? 1 : 2);
    bool complete = g_Glow[client].style == style && g_Glow[client].team == team;
    for (int slot; slot < PREMIUM_GLOW_PARTICLES; slot++)
    {
        bool exists = GlowHostExists(g_Glow[client].references[slot]);
        if (exists != (slot < expected)) complete = false;
    }
    if (complete) return true; // No duplicate emitters on inventory/heartbeat.
    StopGlow(client);
    float now = GlowHostClock();
    if (now < g_Glow[client].retryAfter) return false;
    for (int slot; slot < expected; slot++)
    {
        int effect = style == 0 ? slot : slot * 2 + (team == 2 ? 1 : 0);
        int reference = GlowHostCreate(client, g_GlowEffects[effect], slot);
        if (reference == INVALID_ENT_REFERENCE || !GlowHostExists(reference))
        {
            // Also clean the returned reference if creation became invalid.
            GlowHostDestroy(reference);
            StopGlow(client); // Roll back every earlier successful emitter.
            g_Glow[client].retryAfter = now + 5.0;
            GlowHostFailure(client);
            return false;
        }
        g_Glow[client].references[slot] = reference;
    }
    g_Glow[client].team = team;
    g_Glow[client].style = style;
    g_Glow[client].retryAfter = 0.0;
    return true;
}

void RequestGlow(int client, bool requested)
{
    g_Glow[client].requested = requested;
    if (!requested) ResetGlow(client);
    else RefreshGlow(client);
}
