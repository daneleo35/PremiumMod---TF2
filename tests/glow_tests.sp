#pragma semicolon 1
#pragma newdecls required
#include <sourcemod>

// This tests the production glow controller. These are only engine-boundary
// fakes: references include generations, slots can be reused, and allocation can
// fail at any emitter. No TF2 rendering or entity native is simulated here.
bool g_Allowed[MAXPLAYERS + 1], g_Visible[MAXPLAYERS + 1];
int g_Team[MAXPLAYERS + 1], g_LastEffects[MAXPLAYERS + 1][4];
int g_Style, g_CreateCalls, g_DestroyCalls, g_FailAt, g_InvalidAt, g_Failures;
int g_Generation, g_Capacity = 128, g_Checks;
float g_Clock = 100.0;
bool g_Alive[256], g_Owned[256];
int g_Reference[256], g_Owner[256];

#include "../scripting/premium/glow.inc"

bool GlowHostAllowed(int client) { return g_Allowed[client]; }
bool GlowHostVisible(int client) { return g_Visible[client]; }
int GlowHostStyle(int client) { return client > 0 ? g_Style : 0; }
int GlowHostTeam(int client) { return g_Team[client]; }
float GlowHostClock() { return g_Clock; }
void GlowHostFailure(int client) { if (client > 0) g_Failures++; }

bool GlowHostExists(int reference)
{
    if (reference <= 0) return false;
    int slot = reference & 255;
    return g_Alive[slot] && g_Reference[slot] == reference;
}

void GlowHostDestroy(int reference)
{
    if (!GlowHostExists(reference)) return;
    int slot = reference & 255;
    g_Alive[slot] = false;
    g_Owned[slot] = false;
    g_DestroyCalls++;
}

int Owned(int client = 0)
{
    int count;
    for (int slot; slot < 256; slot++)
        if (g_Alive[slot] && g_Owned[slot] && (client == 0 || g_Owner[slot] == client)) count++;
    return count;
}

int GlowHostCreate(int client, const char[] effect, int effectSlot)
{
    g_CreateCalls++;
    if (g_CreateCalls == g_FailAt || Owned() >= g_Capacity) return INVALID_ENT_REFERENCE;
    int slot;
    while (slot < 256 && g_Alive[slot]) slot++;
    if (slot == 256) return INVALID_ENT_REFERENCE;
    int reference = ++g_Generation * 256 + slot;
    g_Alive[slot] = g_CreateCalls != g_InvalidAt;
    g_Owned[slot] = true;
    g_Reference[slot] = reference;
    g_Owner[slot] = client;
    for (int index; index < 4; index++)
        if (StrEqual(effect, g_GlowEffects[index])) g_LastEffects[client][effectSlot] = index;
    return reference;
}

void Check(bool condition, const char[] label)
{
    g_Checks++;
    if (!condition) ThrowError("FAIL: %s", label);
}

void World()
{
    for (int client = 1; client <= MAXPLAYERS; client++)
    {
        ResetGlow(client);
        g_Allowed[client] = true;
        g_Visible[client] = true;
        g_Team[client] = 2;
    }
    for (int slot; slot < 256; slot++)
    {
        g_Alive[slot] = false;
        g_Owned[slot] = false;
    }
    g_CreateCalls = 0; g_DestroyCalls = 0; g_FailAt = 0; g_InvalidAt = 0;
    g_Failures = 0; g_Style = 0; g_Clock = 100.0; g_Capacity = 128;
}

public int main()
{
    int client = 32;
    World();
    RequestGlow(client, true);
    Check(Owned(client) == 4, "slot 32 owns all four legacy particles");
    Check(g_CreateCalls == 4, "exactly four particles created");
    for (int slot; slot < 4; slot++)
    {
        Check(GlowHostExists(g_Glow[client].references[slot]), "each particle reference is retained");
        Check(g_LastEffects[client][slot] == slot, "original four effects retained");
    }
    for (int repeat; repeat < 100; repeat++) RefreshGlow(client);
    Check(g_CreateCalls == 4 && Owned() == 4, "heartbeat/inventory reconciliation does not duplicate emitters");
    RequestGlow(client, false);
    Check(Owned() == 0 && g_DestroyCalls == 4, "toggle off removes all four, not only the last");
    Check(!g_Glow[client].requested, "toggle off clears preference");
    for (int slot; slot < 4; slot++)
        Check(g_Glow[client].references[slot] == INVALID_ENT_REFERENCE, "ownership cleared after removal");
    RequestGlow(client, false);
    Check(g_DestroyCalls == 4, "repeated cleanup is idempotent");

    World();
    RequestGlow(client, true);
    g_Visible[client] = false;
    StopGlow(client); // Death, cloak/disguise addition, and round end use this.
    Check(Owned() == 0 && g_Glow[client].requested, "hidden/dead state keeps preference without particles");
    Check(!RefreshGlow(client) && Owned() == 0, "hidden state does not recreate emitters");
    g_Visible[client] = true;
    Check(RefreshGlow(client) && Owned() == 4, "respawn/uncloak restores requested glow once");
    g_Allowed[client] = false;
    RefreshGlow(client);
    Check(Owned() == 0 && !g_Glow[client].requested, "expiry/disable/access loss removes and revokes glow");
    g_Allowed[client] = true;
    RefreshGlow(client);
    Check(Owned() == 0, "access recovery does not silently re-enable a revoked preference");

    World();
    RequestGlow(client, true);
    g_Team[client] = 3;
    Check(RefreshGlow(client) && Owned() == 4 && g_DestroyCalls == 4, "team change replaces the old owned set");
    g_Style = 1;
    Check(RefreshGlow(client) && Owned() == 2, "team style has exactly two emitters");
    Check(g_LastEffects[client][0] == 0 && g_LastEffects[client][1] == 2, "BLU style chooses blue effects");
    g_Team[client] = 2;
    Check(RefreshGlow(client) && Owned() == 2, "team style replaces rather than appends effects");
    Check(g_LastEffects[client][0] == 1 && g_LastEffects[client][1] == 3, "RED style chooses red effects");
    g_Style = 0;
    Check(RefreshGlow(client) && Owned() == 4, "switch back to legacy style restores four effects");

    for (int fail = 1; fail <= 4; fail++)
    {
        World();
        g_FailAt = fail;
        RequestGlow(client, true);
        Check(Owned() == 0, "allocation failure rolls back the entire partial glow");
        Check(g_DestroyCalls == fail - 1, "every successfully allocated partial emitter is removed");
        Check(g_Failures == 1, "allocation failure is reported once");
        Check(!RefreshGlow(client) && g_CreateCalls == fail, "failure retry is throttled");
        g_Clock = 105.1;
        g_FailAt = 0;
        Check(RefreshGlow(client) && Owned() == 4, "retry recovers after capacity becomes available");
        RequestGlow(client, false);
        Check(Owned() == 0, "recovered glow cleans up normally");
    }

    World();
    g_InvalidAt = 3;
    RequestGlow(client, true);
    Check(Owned() == 0 && g_DestroyCalls == 2, "creation invalidated by engine rolls back earlier particles");
    Check(g_Failures == 1, "invalid returned reference is reported");

    World();
    RequestGlow(client, true);
    int stale = g_Glow[client].references[0];
    int reusedSlot = stale & 255;
    int foreign = stale + 256;
    g_Reference[reusedSlot] = foreign; // Same entity slot, different generation.
    g_Owned[reusedSlot] = false;
    RequestGlow(client, false);
    Check(GlowHostExists(foreign), "stale cleanup never deletes a reused foreign entity");
    Check(Owned() == 0 && g_DestroyCalls == 3, "remaining original emitters are all removed");

    World();
    RequestGlow(2, true);
    RequestGlow(client, true);
    Check(Owned() == 8, "players have independent particle sets");
    ResetGlow(client); // Disconnect/new-session initialization.
    Check(Owned() == 4 && Owned(2) == 4, "disconnect cleanup does not remove another player's glow");
    Check(!g_Glow[client].requested && !RefreshGlow(client), "new occupant cannot inherit glow preference");
    RequestGlow(client, true);
    for (int target = 1; target <= MAXPLAYERS; target++) StopGlow(target);
    Check(Owned() == 0, "map/plugin cleanup removes every tracked emitter");

    World();
    g_Capacity = 2;
    RequestGlow(client, true);
    Check(Owned() == 0 && g_DestroyCalls == 2, "entity-budget exhaustion leaves no partial glow");
    World();
    for (int repeat; repeat < 100; repeat++)
    {
        RequestGlow(client, true);
        Check(Owned() == 4, "toggle churn remains bounded at four emitters");
        RequestGlow(client, false);
        Check(Owned() == 0, "toggle churn leaks no emitters");
    }
    Check(g_CreateCalls == 400 && g_DestroyCalls == 400, "all 400 churn emitters are accounted for");
    World();
    RequestGlow(MAXPLAYERS, true);
    Check(Owned(MAXPLAYERS) == 4, "highest supported client slot is safe");
    ResetGlow(MAXPLAYERS);
    Check(Owned() == 0, "highest supported slot cleans up");
    World();
    g_Style = 2; RequestGlow(1, true);
    Check(Owned(1) == 1, "subtle team style uses only one emitter");
    RefreshGlow(1);
    Check(Owned(1) == 1 && g_CreateCalls == 1, "subtle style never duplicates");
    g_Style = 1; RefreshGlow(1);
    Check(Owned(1) == 2, "switching personal style rebuilds expected emitter count");
    ResetGlow(1); Check(Owned(1) == 0, "personal-style switch cleans every old emitter");
    PrintToServer("PASS: %d glow ownership assertions", g_Checks);
    return g_Checks;
}
