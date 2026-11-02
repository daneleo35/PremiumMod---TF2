#pragma semicolon 1
#pragma newdecls required
#include <sourcemod>
enum struct TestSetting
{
    bool BoolValue;
    int IntValue;
    float FloatValue;
}
TestSetting cvSpawnEffect, cvSpawnStyle, cvSpawnSeconds, cvSpawnBudget;
bool g_RoundActive = true;
bool g_Alive[5], g_Member[5], g_Visible[5], g_Entities[5], g_Allocate = true;
int g_Serial[5], g_Team[5], g_Creates, g_Checks;
float g_Clock = 100.0;
char g_Effect[5][64];
bool ValidClient(int client) { return client > 0 && client < 5 && g_Alive[client]; }
bool HasPremium(int client) { return ValidClient(client) && g_Member[client]; }
bool CosmeticOwnerVisible(int client) { return ValidClient(client) && g_Visible[client]; }
float TestClock() { return g_Clock; }
int TestSerial(int serial) { for (int i=1; i<5; i++) if (g_Serial[i] == serial && ValidClient(i)) return i; return 0; }
int TestTeam(int client) { return g_Team[client]; }
bool GlowHostExists(int ref) { return ref > 1000 && ref < 1005 && g_Entities[ref-1000]; }
void GlowHostDestroy(int ref) { if (GlowHostExists(ref)) g_Entities[ref-1000] = false; }
int GlowHostCreate(int client, const char[] effect, int slot)
{
    if (slot != 5) ThrowError("wrong spawn slot");
    if (!g_Allocate) return INVALID_ENT_REFERENCE;
    g_Entities[client] = true; g_Creates++; strcopy(g_Effect[client], sizeof(g_Effect[]), effect); return 1000+client;
}
#define MaxClients 4
#define GetEngineTime TestClock
#define GetClientTeam TestTeam
#define GetClientFromSerial TestSerial
#include "../scripting/premium/spawn_effects.inc"
void Check(bool condition, const char[] label) { g_Checks++; if (!condition) ThrowError("FAIL: %s", label); }
public int main()
{
    for (int i=1;i<5;i++) { g_Alive[i]=true;g_Member[i]=true;g_Visible[i]=true;g_Team[i]=2;g_Serial[i]=100+i; }
    cvSpawnSeconds.FloatValue=1.5;cvSpawnBudget.IntValue=2;
    FrameMemberSpawnEffect(101); Check(g_Creates==0,"off by default");
    cvSpawnEffect.BoolValue=true;FrameMemberSpawnEffect(101);
    Check(g_Creates==1 && StrEqual(g_Effect[1],"player_recent_teleport_red"),"member gets owner-approved team shimmer");
    RefreshSpawnEffects();RefreshSpawnEffects();Check(g_Creates==1,"heartbeats do not repeat spawn effect");
    g_Team[2]=3;cvSpawnStyle.IntValue=1;FrameMemberSpawnEffect(102);
    Check(StrEqual(g_Effect[2],"critical_grenade_blue"),"owner second style uses correct team");
    FrameMemberSpawnEffect(103);Check(!g_Entities[3],"concurrent effect budget enforced");
    g_Member[1]=false;RefreshSpawnEffects();Check(!g_Entities[1],"membership loss cleans effect");
    g_Visible[2]=false;RefreshSpawnEffects();Check(!g_Entities[2],"cloak or disguise cleans effect");
    FrameMemberSpawnEffect(102);Check(!g_Entities[2],"hidden owner never emits");
    g_Visible[2]=true;FrameMemberSpawnEffect(102);g_Clock=101.5;RefreshSpawnEffects();Check(!g_Entities[2],"deadline removes effect");
    FrameMemberSpawnEffect(103);g_RoundActive=false;RefreshSpawnEffects();Check(!g_Entities[3],"round end cleans effect");
    FrameMemberSpawnEffect(103);Check(!g_Entities[3],"no spawn effect after combat");
    g_RoundActive=true;g_Serial[3]=203;FrameMemberSpawnEffect(103);Check(!g_Entities[3],"old serial cannot affect reused client slot");
    g_Allocate=false;FrameMemberSpawnEffect(203);Check(!g_Entities[3],"allocation failure leaves no owned emitter");
    g_Allocate=true;FrameMemberSpawnEffect(203);StopSpawnEffect(3);StopSpawnEffect(3);Check(!g_Entities[3],"repeated life cleanup is safe");
    FrameMemberSpawnEffect(203);cvSpawnEffect.BoolValue=false;RefreshSpawnEffects();Check(!g_Entities[3],"live disable removes effect");
    cvSpawnEffect.BoolValue=true;FrameMemberSpawnEffect(203);StopSpawnEffects();Check(!g_Entities[3] && g_SpawnUntil[3]==0.0,"map and unload reset clears deadline");
    PrintToServer("PASS: %d member spawn effect assertions",g_Checks);return g_Checks;
}
