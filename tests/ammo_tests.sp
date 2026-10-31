#pragma semicolon 1
#pragma newdecls required
#include <sourcemod>
#include <sdktools>
#include "../scripting/premium/core.inc"
#include "../scripting/premium/feature_policy.inc"
#pragma unused PremiumRemaining, PremiumCanRespawn, PremiumRespawnPhaseAllowed, PremiumFlameFlightActive
#pragma unused PremiumThrowableAmmoSlot, PremiumGroundJumpLatch, PremiumNormalizeSteam, PremiumCanAddSentry
#pragma unused PremiumCloakAfterDrain, PremiumTeamAllowed, PremiumSwapBalanced, PremiumFlightWeapon, PremiumFlightVertical

enum struct TestSetting
{
    float FloatValue;
}
TestSetting cvAmmo;
float EffectiveAmmoMultiplier(int client) { return client == 1 ? cvAmmo.FloatValue : 1.0; }
bool g_Allowed = true, g_Alive = true, g_SetWorks = true;
bool g_Exists[2];
float g_Attribute[2], g_ItemMultiplier[2];
int g_Reserve[2], g_BaseMax[2], g_Weapons[3], g_WeaponPool[128];
int g_Checks, g_Gives, g_Writes;
int AttributePool(const char[] name)
{
    if (StrEqual(name, "maxammo primary increased")) return 0;
    if (StrEqual(name, "maxammo secondary increased")) return 1;
    ThrowError("unexpected attribute %s", name);
    return -1;
}
bool TestProvider() { return true; }
bool TestValid(int entity) { return entity == 1 || (entity >= 40 && entity < 128); }
bool TestClient(int entity) { return entity == 1; }
int TestRef(int entity) { return TestValid(entity) ? entity + 4096 : INVALID_ENT_REFERENCE; }
int TestIndex(int reference) { return reference - 4096; }
Address TestGet(int entity, const char[] name)
{
    int pool = AttributePool(name);
    return entity == 1 && g_Exists[pool] ? view_as<Address>(pool + 1) : Address_Null;
}
float TestValue(Address address) { return g_Attribute[view_as<int>(address) - 1]; }
bool TestSet(int entity, const char[] name, float value)
{
    if (entity != 1 || !g_SetWorks) return false;
    int pool = AttributePool(name);
    g_Exists[pool] = true; g_Attribute[pool] = value; g_Writes++; return true;
}
bool TestRemove(int entity, const char[] name)
{
    if (entity != 1) return false;
    g_Exists[AttributePool(name)] = false; return true;
}
bool TestClear(int entity) { return entity == 1; }
bool TestHas(int entity, PropType type, const char[] name)
{
    return entity >= 40 && type == Prop_Send && StrEqual(name, "m_iPrimaryAmmoType");
}
int TestOwner(int entity, PropType type, const char[] name) { return entity >= 40 && type == Prop_Send && name[0] ? 1 : -1; }
bool GameplayAllowed(int client) { return client == 1 && g_Allowed; }
bool GameplayFeatureAllowed(int client, PremiumFeature feature) { return feature >= Feature_Speed && feature < Feature_Count && GameplayAllowed(client); }
bool TestAlive(int client) { return client == 1 && g_Alive; }
int TestSlot(int client, int slot) { return client == 1 ? g_Weapons[slot] : -1; }
bool ValidWeapon(int entity) { return entity >= 40 && entity < 128; }
int TestProp(int entity, PropType type, const char[] name)
{
    return TestHas(entity, type, name) ? g_WeaponPool[entity] : -1;
}
int TestSize(int entity, PropType type, const char[] name) { return entity == 1 && type == Prop_Send && StrEqual(name, "m_iAmmo") ? 8 : 0; }
int TestGive(int client, int amount, int ammo, bool silent)
{
    if (client != 1 || (ammo != 1 && ammo != 2) || !silent || amount < 1) ThrowError("invalid ammo native call");
    int pool = ammo - 1;
    int maximum = RoundToFloor(float(g_BaseMax[pool]) * g_ItemMultiplier[pool] * (g_Exists[pool] ? g_Attribute[pool] : 1.0));
    int addition = maximum - g_Reserve[pool];
    if (addition < 0) addition = 0;
    if (addition > amount) addition = amount;
    g_Reserve[pool] += addition; g_Gives++;
    return addition;
}

#define MaxClients 32
#define PremiumAttributeNativesAvailable TestProvider
#define EntIndexToEntRef TestRef
#define EntRefToEntIndex TestIndex
#define IsValidEntity TestValid
#define ValidClient TestClient
#define HasEntProp TestHas
#define GetEntPropEnt TestOwner
#define TF2Attrib_GetByName TestGet
#define TF2Attrib_GetValue TestValue
#define TF2Attrib_SetByName TestSet
#define TF2Attrib_RemoveByName TestRemove
#define TF2Attrib_ClearCache TestClear
#include "../scripting/premium/attributes.inc"
#define IsPlayerAlive TestAlive
#define GetPlayerWeaponSlot TestSlot
#define GetEntProp TestProp
#define GetEntPropArraySize TestSize
#define GivePlayerAmmo TestGive
#include "../scripting/premium/ammo.inc"
#pragma unused g_AmmoHooks

void Check(bool condition, const char[] label)
{
    g_Checks++;
    if (!condition) ThrowError("FAIL: %s", label);
}

public int main()
{
    cvAmmo.FloatValue = 2.0;
    g_BaseMax[0] = 200; g_BaseMax[1] = 32; // Pyro fuel and shotgun shells.
    g_ItemMultiplier[0] = 1.0; g_ItemMultiplier[1] = 1.0;
    g_Reserve[0] = 200; g_Reserve[1] = 32;
    g_Weapons[0] = 40; g_Weapons[1] = 41; g_Weapons[2] = -1;
    g_WeaponPool[40] = 1; g_WeaponPool[41] = 2;
    MaintainAmmoCapacity(1);
    Check(g_Exists[0] && g_Exists[1] && g_Attribute[0] == 2.0 && g_Attribute[1] == 2.0, "both player reserve attributes apply");
    Check(g_Reserve[0] == 200 && g_Gives == 0, "capacity maintenance does not give ammo");
    GiveSpawnAmmo(1);
    Check(g_Reserve[0] == 400, "clipless Pyro fuel increases from 200 to 400");
    Check(g_Reserve[1] == 64, "secondary reserve increases without changing its magazine");
    g_Reserve[0] = 123; g_Reserve[1] = 8;
    int writes = g_Writes;
    for (int tick; tick < 20; tick++) MaintainAmmoCapacity(1);
    Check(g_Reserve[0] == 123 && g_Reserve[1] == 8 && g_Gives == 2, "spent fuel and shells are never heartbeat-refilled");
    Check(g_Writes == writes, "capacity maintenance does not compound or rewrite unchanged values");
    cvAmmo.FloatValue = 3.0;
    MaintainAmmoCapacity(1);
    Check(g_Attribute[0] == 3.0 && g_Reserve[0] == 123, "live multiplier changes capacity without refilling");
    GiveSpawnAmmo(1);
    Check(g_Reserve[0] == 600 && g_Reserve[1] == 96, "next spawn fills new native capacity");
    g_Allowed = false;
    MaintainAmmoCapacity(1);
    Check(!g_Exists[0] && !g_Exists[1], "team/access loss restores both reserve attributes");
    int gives = g_Gives;
    GiveSpawnAmmo(1);
    Check(g_Gives == gives, "restricted player cannot receive spawn bonus");
    g_Allowed = true; cvAmmo.FloatValue = 2.0;
    g_BaseMax[0] = 20; g_BaseMax[1] = 32; // Soldier-like pools, with an item penalty.
    g_ItemMultiplier[0] = 0.5; g_Reserve[0] = 0; g_Reserve[1] = 0;
    MaintainAmmoCapacity(1); GiveSpawnAmmo(1);
    Check(g_Reserve[0] == 20 && g_Reserve[1] == 64, "native refill honors existing item reserve penalties");
    g_WeaponPool[41] = 1;
    gives = g_Gives; GiveSpawnAmmo(1);
    Check(g_Gives == gives + 1, "weapons sharing a pool receive one native supply call");
    g_WeaponPool[40] = 3; g_WeaponPool[41] = 4;
    gives = g_Gives; GiveSpawnAmmo(1);
    Check(g_Gives == gives, "metal and throwable reserves are excluded");
    g_WeaponPool[40] = -1; g_WeaponPool[41] = 8;
    GiveSpawnAmmo(1);
    Check(g_Gives == gives, "energy/no-ammo and invalid indices are excluded");
    RestoreAmmoCapacity(1);
    g_Exists[0] = true; g_Attribute[0] = 1.5;
    MaintainAmmoCapacity(1);
    Check(g_Attribute[0] == 3.0, "preexisting player capacity modifier is preserved");
    RestoreAmmoCapacity(1);
    Check(g_Exists[0] && g_Attribute[0] == 1.5 && !g_Exists[1], "cleanup restores only owned values");
    g_Exists[0] = false; g_SetWorks = false;
    MaintainAmmoCapacity(1);
    g_WeaponPool[40] = 1; g_WeaponPool[41] = 2;
    GiveSpawnAmmo(1);
    Check(g_Gives == gives && !g_AmmoCapacity[1][0].owned && !g_AmmoCapacity[1][1].owned, "failed attribute writes cannot award extra ammo");
    g_SetWorks = true; cvAmmo.FloatValue = 1.0;
    MaintainAmmoCapacity(1); GiveSpawnAmmo(1);
    Check(!g_Exists[0] && !g_Exists[1] && g_Gives == gives, "multiplier one is native capacity and no bonus supply");
    PrintToServer("PASS: %d reserve ammo assertions", g_Checks);
    return g_Checks;
}
