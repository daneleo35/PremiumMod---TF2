#pragma semicolon 1
#pragma newdecls required
#include <sourcemod>

bool g_Library;
bool g_Ready;
char g_MissingNative[64];
int g_ReadyCalls;
int g_Checks;

bool TestLibraryExists(const char[] name) { return g_Library && StrEqual(name, "tf2attributes"); }
FeatureStatus TestFeatureStatus(FeatureType type, const char[] name)
{
    return type != FeatureType_Native || StrEqual(name, g_MissingNative) ? FeatureStatus_Unavailable : FeatureStatus_Available;
}
bool TestAttributesReady() { g_ReadyCalls++; return g_Ready; }

// Supply host-boundary fakes; the actual production dependency checker is used.
#define LibraryExists TestLibraryExists
#define GetFeatureStatus TestFeatureStatus
#define TF2Attrib_IsReady TestAttributesReady
#include "../scripting/premium/dependencies.inc"
#undef LibraryExists
#undef GetFeatureStatus
#undef TF2Attrib_IsReady

void Check(bool condition, const char[] label)
{
    g_Checks++;
    if (!condition) ThrowError("FAIL: %s", label);
}

public int main()
{
    char reason[512];
    Check(!PremiumCheckDependencies(reason, sizeof(reason)), "missing library rejected");
    Check(StrContains(reason, "TF2Attributes is missing") >= 0, "named missing-library warning");
    Check(StrContains(reason, "plugins") >= 0 && StrContains(reason, "gamedata") >= 0, "installation paths supplied");
    Check(StrContains(reason, "https://github.com/FlaminSarge/tf2attributes/releases") >= 0, "download link supplied");
    Check(g_ReadyCalls == 0, "unbound readiness native never called");
    g_Library = true;
    char names[][] = {"TF2Attrib_IsReady", "TF2Attrib_GetByName", "TF2Attrib_GetValue",
        "TF2Attrib_SetByName", "TF2Attrib_RemoveByName", "TF2Attrib_ClearCache", "TF2Attrib_HookValueFloat"};
    for (int i; i < sizeof(names); i++)
    {
        strcopy(g_MissingNative, sizeof(g_MissingNative), names[i]);
        Check(!PremiumCheckDependencies(reason, sizeof(reason)), "incomplete API rejected");
        Check(StrContains(reason, "API is unavailable") >= 0, "API warning supplied");
        Check(g_ReadyCalls == 0, "no provider call with incomplete API");
    }
    g_MissingNative[0] = '\0';
    Check(!PremiumCheckDependencies(reason, sizeof(reason)), "failed provider readiness rejected");
    Check(StrContains(reason, "failed to initialize") >= 0, "gamedata warning supplied");
    Check(g_ReadyCalls == 1, "bound provider checked once");
    g_Ready = true;
    Check(PremiumCheckDependencies(reason, sizeof(reason)), "healthy dependency accepted");
    Check(reason[0] == '\0', "old warning cleared on success");
    Check(g_ReadyCalls == 2, "healthy native invoked");
    PrintToServer("PASS: %d dependency assertions", g_Checks);
    return g_Checks;
}
