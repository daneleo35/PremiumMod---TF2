#pragma semicolon 1
#pragma newdecls required
#include <sourcemod>
#include <sdktools>
#include <tf2_stocks>
#include "../scripting/premium/core.inc"
#include "../scripting/premium/feature_policy.inc"
#pragma unused PremiumRemaining, PremiumCanRespawn, PremiumRespawnPhaseAllowed, PremiumFlameFlightActive
#pragma unused PremiumThrowableAmmoSlot, PremiumGroundJumpLatch, PremiumNormalizeSteam, PremiumReserveAmmoIndex, PremiumCanAddSentry
#pragma unused PremiumCloakAfterDrain, PremiumTeamAllowed, PremiumSwapBalanced, PremiumFlightVertical

enum struct TestSetting
{
    float FloatValue;
}
TestSetting cvAirblastForce, cvAirblastLift, cvAirblastCap, cvAirblastCooldown;
bool g_Access = true, g_Rule = true, g_Alive = true, g_Blocked, g_WeaponValid = true, g_HasState = true;
int g_Weapon = 40, g_Fuel = 200, g_State = 0, g_Flags = FL_ONGROUND, g_Buttons = IN_ATTACK2, g_Launches, g_Checks;
TFClassType g_Class = TFClass_Pyro;
float g_Clock = 100.0, g_Secondary = 99.0, g_Pitch = 89.0;
float g_Velocity[3], g_Direction[3] = {0.0, 0.0, -1.0};
bool g_DroppedGround;
char g_Classname[64] = "tf_weapon_flamethrower";
bool GameplayAllowed(int client) { return client == 1 && g_Access; }
bool GameplayFeatureAllowed(int client, PremiumFeature feature) { return feature == Feature_Airblast && GameplayAllowed(client); }
bool AirblastEnabled(int client) { return client == 1 && g_Rule; }
bool TestAlive(int client) { return client == 1 && g_Alive; }
TFClassType TestClass(int client) { return client == 1 ? g_Class : TFClass_Unknown; }
bool MovementBlocked(int client) { return client != 1 || g_Blocked; }
int TestButtons(int client) { return client == 1 ? g_Buttons : 0; }
int TestEntity(int client, PropType type, const char[] name) { return client == 1 && type == Prop_Send && name[0] ? g_Weapon : -1; }
bool ValidWeapon(int weapon) { return g_WeaponValid && (weapon == 40 || weapon == 41); }
bool TestHas(int weapon, PropType type, const char[] name)
{
    return ValidWeapon(weapon) && type == Prop_Send && (!StrEqual(name, "m_iWeaponState") || g_HasState);
}
bool TestName(int weapon, char[] output, int size) { strcopy(output, size, g_Classname); return ValidWeapon(weapon); }
int TestRef(int weapon) { return weapon + 4096; }
int TestProp(int entity, PropType type, const char[] name, int size = 4, int element = 0)
{
    if (type != Prop_Send || size != 4) return -1;
    if (StrEqual(name, "m_iPrimaryAmmoType")) return 1;
    if (StrEqual(name, "m_iWeaponState")) return g_State;
    if (entity == 1 && StrEqual(name, "m_iAmmo") && element == 1) return g_Fuel;
    ThrowError("unexpected property"); return -1;
}
int TestSize(int client, PropType type, const char[] name) { return client == 1 && type == Prop_Send && name[0] ? 8 : 0; }
float TestFloat(int weapon, PropType type, const char[] name) { return ValidWeapon(weapon) && type == Prop_Send && name[0] ? g_Secondary : -1.0; }
float TestClock() { return g_Clock; }
void TestAngles(int client, float angles[3]) { angles[0] = g_Pitch; angles[1] = 0.0; angles[2] = float(client - 1); }
void TestVectors(const float angles[3], float direction[3], float right[3], float up[3])
{
    if (FloatAbs(angles[2]) > 0.001 || FloatAbs(right[0] - up[0]) > 0.001) ThrowError("bad angle client");
    for (int axis; axis < 3; axis++) direction[axis] = g_Direction[axis];
}
void TestVelocity(int client, PropType type, const char[] name, float velocity[3])
{
    if (client != 1 || type != Prop_Data || !name[0]) ThrowError("bad velocity read");
    for (int axis; axis < 3; axis++) velocity[axis] = g_Velocity[axis];
}
int TestFlags(int client) { return client == 1 ? g_Flags : 0; }
void TestSetFlags(int client, int flags) { if (client == 1) g_Flags = flags; }
void TestGround(int client, PropType type, const char[] name, int value)
{
    g_DroppedGround = client == 1 && type == Prop_Send && StrEqual(name, "m_hGroundEntity") && value == -1;
}
void TestTeleport(int client, const float origin[3], const float angles[3], const float velocity[3])
{
    if (client != 1 || FloatAbs(origin[0] - angles[0]) > 0.001) ThrowError("bad teleport client");
    for (int axis; axis < 3; axis++) g_Velocity[axis] = velocity[axis];
    g_Launches++;
}
#define IsPlayerAlive TestAlive
#define TF2_GetPlayerClass TestClass
#define GetClientButtons TestButtons
#define GetEntPropEnt TestEntity
#define HasEntProp TestHas
#define GetEntityClassname TestName
#define EntIndexToEntRef TestRef
#define GetEntProp TestProp
#define GetEntPropArraySize TestSize
#define GetEntPropFloat TestFloat
#define GetGameTime TestClock
#define GetClientEyeAngles TestAngles
#define GetAngleVectors TestVectors
#define GetEntPropVector TestVelocity
#define GetEntityFlags TestFlags
#define SetEntityFlags TestSetFlags
#define SetEntPropEnt TestGround
#define TeleportEntity TestTeleport
#include "../scripting/premium/airblast.inc"

void Check(bool condition, const char[] label) { g_Checks++; if (!condition) ThrowError("FAIL: %s", label); }
void Reset()
{
    g_Access = true; g_Rule = true; g_Alive = true; g_Blocked = false; g_WeaponValid = true; g_HasState = true;
    g_Weapon = 40; g_Fuel = 200; g_State = 0; g_Flags = FL_ONGROUND; g_Buttons = IN_ATTACK2; g_Launches = 0;
    g_Class = TFClass_Pyro; g_Clock = 100.0; g_Secondary = 99.0; g_Pitch = 89.0;
    g_NextAirblast[1] = 0.0; g_AirblastSample[1] = false; g_DroppedGround = false;
    cvAirblastForce.FloatValue = 400.0; cvAirblastLift.FloatValue = 350.0; cvAirblastCap.FloatValue = 900.0; cvAirblastCooldown.FloatValue = 0.75;
    for (int axis; axis < 3; axis++) { g_Velocity[axis] = 0.0; g_Direction[axis] = axis == 2 ? -1.0 : 0.0; }
    strcopy(g_Classname, sizeof(g_Classname), "tf_weapon_flamethrower");
}
void NativeBlast(int cost = 20) { g_Fuel -= cost; g_Secondary = g_Clock + 0.75; g_State = 3; }

public int main()
{
    Reset(); CaptureAirblast(1); NativeBlast(); ApplyAirblastBoost(1);
    Check(g_Launches == 1 && g_Velocity[2] == 400.0, "real downward blast launches once");
    Check(g_DroppedGround && !(g_Flags & FL_ONGROUND), "grounded blast can take off");
    Check(g_Fuel == 180 && g_Secondary == 100.75, "native ammo cost and weapon timers remain untouched");
    Check(!PremiumAirblastLookingDown(0.0) && !PremiumAirblastLookingDown(-0.1), "level/upward pitch rejected");
    Check(PremiumAirblastLookingDown(0.1) && PremiumAirblastLookingDown(89.0), "downward pitch accepted");
    Check(!PremiumAirblastLookingDown(180.0) && !PremiumAirblastLookingDown(270.0), "invalid/wrapped upward pitches rejected");
    ApplyAirblastBoost(1); Check(g_Launches == 1, "consumed observation cannot launch twice");
    g_Clock = 100.1; CaptureAirblast(1); ApplyAirblastBoost(1);
    Check(g_Launches == 1, "holding right click between actual blasts adds no per-frame thrust");
    Reset(); g_Pitch = 0.0; CaptureAirblast(1); NativeBlast(); ApplyAirblastBoost(1);
    Check(g_Launches == 0 && g_Fuel == 180 && g_Secondary == 100.75, "level airblast stays native without jump boost");
    Check(!g_DroppedGround && g_NextAirblast[1] == 0.0, "rejected angle cannot detach ground or consume boost cooldown");
    Reset(); g_Pitch = -45.0; CaptureAirblast(1); NativeBlast(); ApplyAirblastBoost(1);
    Check(g_Launches == 0 && g_Fuel == 180, "upward airblast spends normal fuel without boost");
    Reset(); CaptureAirblast(1); NativeBlast(); g_Pitch = 0.0; ApplyAirblastBoost(1);
    Check(g_Launches == 0, "looking level after capture cancels boost");
    Reset(); g_Pitch = 0.0; CaptureAirblast(1); NativeBlast(); g_Pitch = 89.0; ApplyAirblastBoost(1);
    Check(g_Launches == 0, "looking down after a level blast cannot create a delayed boost");
    Reset(); g_Pitch = 0.1; CaptureAirblast(1); NativeBlast(); ApplyAirblastBoost(1);
    Check(g_Launches == 1, "any aim below the horizon permits the configured boost");
    Reset(); g_Fuel = 20; CaptureAirblast(1); NativeBlast(); ApplyAirblastBoost(1);
    Check(g_Launches == 1 && g_Fuel == 0, "last complete airblast fuel charge can boost");
    Reset(); g_Fuel = 10; CaptureAirblast(1); ApplyAirblastBoost(1);
    Check(g_Launches == 0, "insufficient fuel/native rejection cannot boost");
    Reset(); CaptureAirblast(1); g_State = 3; g_Secondary = 100.75; ApplyAirblastBoost(1);
    Check(g_Launches == 0, "Phlog rage/charge state without fuel spending cannot boost");
    Reset(); g_Buttons = IN_ATTACK; CaptureAirblast(1); NativeBlast(); ApplyAirblastBoost(1);
    Check(g_Launches == 0, "primary fire cannot be mistaken for an airblast");
    Reset(); CaptureAirblast(1); NativeBlast(50); ApplyAirblastBoost(1);
    Check(g_Launches == 1 && g_Fuel == 150, "Backburner-style higher native cost supported");
    Reset(); g_HasState = false; strcopy(g_Classname, sizeof(g_Classname), "tf_weapon_rocketlauncher_fireball");
    CaptureAirblast(1); NativeBlast(5); ApplyAirblastBoost(1);
    Check(g_Launches == 1 && g_Fuel == 195, "Dragon Fury detection does not require flame-stream state");
    Reset(); cvAirblastCooldown.FloatValue = 2.0; CaptureAirblast(1); NativeBlast(); ApplyAirblastBoost(1);
    g_Clock = 100.8; CaptureAirblast(1); NativeBlast(); ApplyAirblastBoost(1);
    Check(g_Launches == 1, "owner cooldown can restrict otherwise valid native blasts");
    g_Clock = 102.1; CaptureAirblast(1); ApplyAirblastBoost(1);
    Check(g_Launches == 1, "suppressed blast cannot queue a delayed boost");
    Reset(); CaptureAirblast(1); NativeBlast(); g_Access = false; ApplyAirblastBoost(1);
    Check(g_Launches == 0, "access loss after capture cancels impulse");
    Reset(); CaptureAirblast(1); NativeBlast(); g_Rule = false; ApplyAirblastBoost(1);
    Check(g_Launches == 0, "map/class/feature disable cancels impulse");
    Reset(); g_Blocked = true; CaptureAirblast(1); NativeBlast(); ApplyAirblastBoost(1);
    Check(g_Launches == 0, "frozen/taunting/stunned/water movement guard respected");
    Reset(); g_Class = TFClass_Soldier; CaptureAirblast(1); NativeBlast(); ApplyAirblastBoost(1);
    Check(g_Launches == 0, "other classes cannot activate Pyro boost");
    Reset(); CaptureAirblast(1); NativeBlast(); g_Weapon = 41; ApplyAirblastBoost(1);
    Check(g_Launches == 0, "weapon switch rejects stale observation");
    Reset(); strcopy(g_Classname, sizeof(g_Classname), "tf_weapon_shotgun_pyro"); CaptureAirblast(1); NativeBlast(); ApplyAirblastBoost(1);
    Check(g_Launches == 0, "shotgun secondary input cannot boost");
    Reset(); cvAirblastForce.FloatValue = 0.0; cvAirblastLift.FloatValue = 0.0; CaptureAirblast(1); NativeBlast(); ApplyAirblastBoost(1);
    Check(g_Launches == 0, "zero configured impulse leaves native airblast unchanged");
    float direction[3] = {1.0, 0.0, 0.0}, before[3], after[3];
    PremiumAirblastImpulse(direction, before, 400.0, 350.0, 900.0, after);
    Check(after[0] == -400.0 && after[2] == 350.0, "horizontal aim recoils backward with upward lift");
    before[0] = -900.0; before[2] = 900.0;
    PremiumAirblastImpulse(direction, before, 400.0, 350.0, 900.0, after);
    Check(FloatAbs(after[0] + 900.0) < 0.01 && after[2] == 900.0, "repeated impulses cannot exceed configured caps");
    before[0] = -1200.0; before[2] = 1200.0;
    PremiumAirblastImpulse(direction, before, 400.0, 350.0, 900.0, after);
    Check(FloatAbs(after[0] + 1200.0) < 0.01 && after[2] == 1200.0, "pre-existing faster momentum is preserved without further acceleration");
    before[0] = 0.0; before[2] = -800.0;
    PremiumAirblastImpulse(direction, before, 400.0, 350.0, 900.0, after);
    Check(after[2] == -450.0, "boost adds impulse rather than erasing all falling velocity");
    Check(!PremiumAirblastFired(99.0, 100.5, 100.0, 200, 200), "cooldown advance alone is insufficient");
    Check(!PremiumAirblastFired(99.0, 99.0, 100.0, 200, 180), "fuel consumption alone is insufficient");
    PrintToServer("PASS: %d airblast boost assertions", g_Checks);
    return g_Checks;
}
