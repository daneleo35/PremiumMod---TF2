#pragma semicolon 1
#pragma newdecls required
#include <sourcemod>
#include <sdktools>

int g_Checks, g_Writes, g_Clears;
bool g_Valid[128], g_Exists[128], g_Provider = true, g_SetWorks = true;
float g_Value[128];
bool TestNatives() { return g_Provider; }
int TestRef(int entity) { return entity + 4096; }
int TestIndex(int reference) { return reference - 4096; }
bool TestValid(int entity) { return entity > 0 && entity < sizeof(g_Valid) && g_Valid[entity]; }
bool TestClient(int client) { return client == 1; }
bool TestProp(int entity, PropType type, const char[] name) { return entity > 0 && type == Prop_Send && StrEqual(name, "m_hOwnerEntity"); }
int TestOwner(int entity, PropType type, const char[] name) { return TestProp(entity, type, name) ? 1 : 0; }
Address TestGet(int entity, const char[] name) { return name[0] && g_Exists[entity] ? view_as<Address>(entity) : Address_Null; }
float TestValue(Address address) { return g_Value[view_as<int>(address)]; }
bool TestSet(int entity, const char[] name, float value)
{
    if (!g_SetWorks || !name[0]) return false;
    g_Exists[entity] = true; g_Value[entity] = value; g_Writes++; return true;
}
bool TestRemove(int entity, const char[] name) { if (!name[0]) return false; g_Exists[entity] = false; return true; }
bool TestClear(int entity) { if (entity <= 0) return false; g_Clears++; return true; }

#define MaxClients 32
#define PremiumAttributeNativesAvailable TestNatives
#define EntIndexToEntRef TestRef
#define EntRefToEntIndex TestIndex
#define IsValidEntity TestValid
#define ValidClient TestClient
#define HasEntProp TestProp
#define GetEntPropEnt TestOwner
#define TF2Attrib_GetByName TestGet
#define TF2Attrib_GetValue TestValue
#define TF2Attrib_SetByName TestSet
#define TF2Attrib_RemoveByName TestRemove
#define TF2Attrib_ClearCache TestClear
#include "../scripting/premium/attributes.inc"

void Check(bool condition, const char[] label)
{
    g_Checks++;
    if (!condition) ThrowError("FAIL: %s", label);
}

public int main()
{
    g_Valid[40] = true; g_Valid[41] = true; g_Valid[1] = true;
    AttributeState primary, secondary, reserve;
    ApplyAttribute(40, primary, "clip size bonus", 2.0);
    Check(primary.owned && primary.reference == 4136, "ownership propagates through enum-struct reference");
    Check(g_Exists[40] && FloatAbs(g_Value[40] - 2.0) < 0.001, "primary clip multiplied");
    Check(g_Clears == 1, "weapon owner cache cleared");
    for (int i; i < 10; i++) ApplyAttribute(40, primary, "clip size bonus", 2.0);
    Check(g_Writes == 1, "maintenance does not stack or rewrite unchanged value");
    g_Exists[40] = false;
    ApplyAttribute(40, primary, "clip size bonus", 2.0);
    Check(g_Writes == 2 && primary.owned, "removed inventory attribute repaired");
    g_Exists[41] = true; g_Value[41] = 0.75;
    ApplyAttribute(41, secondary, "clip size bonus", 2.0);
    Check(FloatAbs(g_Value[41] - 1.5) < 0.001, "existing secondary modifier preserved multiplicatively");
    Check(FloatAbs(g_Value[40] - 2.0) < 0.001, "secondary does not replace primary ownership");
    RestoreAttribute(primary, "clip size bonus");
    Check(!primary.owned && !g_Exists[40], "primary cleanup removes only the new attribute");
    RestoreAttribute(secondary, "clip size bonus");
    Check(!secondary.owned && FloatAbs(g_Value[41] - 0.75) < 0.001, "secondary cleanup restores its original value");
    ApplyAttribute(1, reserve, "maxammo grenades1 increased", 15.0);
    Check(FloatAbs(g_Value[1] - 15.0) < 0.001, "native throwable capacity raised to fifteen");
    ApplyAttribute(1, reserve, "maxammo grenades1 increased", 3.0);
    Check(FloatAbs(g_Value[1] - 3.0) < 0.001, "live reserve change is not compounded");
    ApplyAttribute(1, reserve, "maxammo grenades1 increased", 1.0);
    Check(!reserve.owned && !g_Exists[1], "disabling reserve restores native capacity");
    ApplyAttribute(40, primary, "clip size bonus", 2.0);
    g_Value[40] = 3.0;
    RestoreAttribute(primary, "clip size bonus");
    Check(FloatAbs(g_Value[40] - 3.0) < 0.001 && !primary.owned, "cleanup preserves another plugin's later value");
    g_SetWorks = false;
    ApplyAttribute(41, secondary, "clip size bonus", 2.0);
    Check(!secondary.owned, "failed provider write does not claim ownership");
    g_SetWorks = true; g_Exists[40] = false;
    ApplyAttribute(40, primary, "clip size bonus", 2.0);
    g_Valid[40] = false;
    RestoreAttribute(primary, "clip size bonus");
    Check(!primary.owned, "deleted weapon reference can be released safely");
    PrintToServer("PASS: %d attribute ownership assertions", g_Checks);
    return g_Checks;
}
