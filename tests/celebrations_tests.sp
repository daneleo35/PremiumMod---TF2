#pragma semicolon 1
#pragma newdecls required
#include <sourcemod>
#include "../scripting/premium/feature_policy.inc"
enum struct TestSetting
{
    bool BoolValue;
    int IntValue;
    float FloatValue;
}
TestSetting cvCelebration, cvCelebrationSeconds, cvCelebrationBudget;
bool g_RoundActive = true;
bool g_Member[5], g_Visible[5], g_Entities[5];
int g_Team[5], g_Creates, g_Destroys, g_Stops, g_Checks;
bool g_Allocate = true;
float g_Clock = 100.0;
float TestClock() { return g_Clock; }
bool TestMember(int client) { return client > 0 && client < 5 && g_Member[client]; }
bool HasFeature(int client, PremiumFeature feature) { return feature == Feature_Celebration && TestMember(client); }
int TestTeam(int client) { return g_Team[client]; }
bool CosmeticOwnerVisible(int client) { return g_Visible[client]; }
bool GlowHostExists(int reference) { return reference >= 1001 && reference <= 1004 && g_Entities[reference - 1000]; }
void GlowHostDestroy(int reference) { if (GlowHostExists(reference)) { g_Entities[reference - 1000] = false; g_Destroys++; } }
void StopGlow(int client) { if (client > 0 && client < 5) g_Stops++; }
void StopSpawnEffect(int client) { if (client < 1) ThrowError("invalid spawn cleanup"); }
int GlowHostCreate(int client, const char[] effect, int slot)
{
    if (!g_Allocate) return INVALID_ENT_REFERENCE;
    if (slot != 4 || !effect[0]) ThrowError("bad celebration host call");
    g_Entities[client] = true; g_Creates++; return client + 1000;
}
#define MaxClients 4
#define GetEngineTime TestClock
#define GetClientTeam TestTeam
#include "../scripting/premium/celebrations.inc"
void Check(bool condition, const char[] label) { g_Checks++; if (!condition) ThrowError("FAIL: %s", label); }
int Count() { int count; for (int client = 1; client <= 4; client++) if (g_Entities[client]) count++; return count; }

public int main()
{
    cvCelebration.BoolValue = true; cvCelebrationSeconds.FloatValue = 2.0; cvCelebrationBudget.IntValue = 2;
    for (int client = 1; client <= 4; client++) { g_Member[client] = true; g_Visible[client] = true; g_Team[client] = 2; }
    g_Team[4] = 3;
    BeginCelebration(2); RefreshCelebrations();
    Check(Count() == 0, "no celebration particles during active combat");
    g_RoundActive = false; RefreshCelebrations();
    Check(Count() == 2 && !g_Entities[4], "only winning team within effect budget");
    int creates = g_Creates; RefreshCelebrations();
    Check(g_Creates == creates, "heartbeat cannot duplicate emitters");
    g_Visible[1] = false; RefreshCelebrations();
    Check(!g_Entities[1] && Count() == 2, "cloaked/disguised/hidden owner removed, slot reused safely");
    cvCelebrationBudget.IntValue = 1; RefreshCelebrations();
    Check(Count() == 1, "live effect-budget reduction enforced");
    g_Member[2] = false; RefreshCelebrations();
    Check(!g_Entities[2], "access loss removes celebration");
    g_Clock = 102.1; RefreshCelebrations();
    Check(Count() == 0, "deadline removes all owned effects");
    BeginCelebration(3); RefreshCelebrations();
    Check(g_Entities[4] && Count() == 1, "next winner uses its own team");
    RebuildPremiumCosmetics();
    Check(Count() == 0 && g_Stops == 4, "visibility change stops already-running emitters");
    RefreshCelebrations();
    Check(Count() == 1, "permitted effects recreate within original celebration window");
    cvCelebration.BoolValue = false; RefreshCelebrations();
    Check(Count() == 0, "owner disable cleans effects");
    cvCelebration.BoolValue = true; StopCelebrations(); BeginCelebration(0); RefreshCelebrations();
    Check(Count() == 0 && g_CelebrationUntil == 0.0, "non-winning events cannot create celebration");
    g_Allocate = false; BeginCelebration(3); RefreshCelebrations();
    Check(Count() == 0, "entity allocation failure leaves no partial effect");
    StopCelebrations();
    Check(Count() == 0 && g_CelebrationUntil == 0.0 && g_Destroys > 0, "round/map/unload reset clears ownership and deadline");
    PrintToServer("PASS: %d celebration ownership assertions", g_Checks);
    return g_Checks;
}
