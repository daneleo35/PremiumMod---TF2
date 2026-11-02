#pragma semicolon 1
#pragma newdecls required
#include <sourcemod>
#include <sdktools>
#include <sdkhooks>
#include <tf2_stocks>
#include "../scripting/premium/core.inc"
#include "../scripting/premium/feature_policy.inc"
#include "../scripting/premium/rule_policy.inc"
#pragma unused PremiumRuleNumber, PremiumMapMatches, PremiumToggle, PremiumRuleFloat
#pragma unused PremiumRemaining, PremiumCanRespawn, PremiumRespawnPhaseAllowed, PremiumFlameFlightActive
#pragma unused PremiumThrowableAmmoSlot, PremiumGroundJumpLatch, PremiumNormalizeSteam, PremiumReserveAmmoIndex
#pragma unused PremiumCloakAfterDrain, PremiumTeamAllowed, PremiumSwapBalanced, PremiumFlightWeapon, PremiumFlightVertical

// Execute production sentry tracking/mode ownership against fake entities.
// Native client prediction, blueprint placement and sappers require a real server.
enum struct TestSetting
{
    int IntValue;
}
TestSetting cvSentryLimit, cvTeamSentries, cvDispenserLimit, cvEntranceLimit, cvExitLimit;
TestSetting cvTeamDispensers, cvTeamEntrances, cvTeamExits;
int g_Teams[3] = {0, 2, 3};
int TestTeam(int client) { return g_Teams[client]; }
float TestTime() { return 100.0; }
int g_BuilderType = 2, g_BuilderMode;
bool g_RoundActive = true, g_Allowed = true, g_Alive = true;
bool g_Carrying;
float g_WrenchDistance = 100.0;
void TestAbsOrigin(int client, float origin[3]) { origin[0] = float(client); }
void TestOrigin(int entity, PropType type, const char[] name, float origin[3]) { origin[0] = float(entity + view_as<int>(type) + strlen(name)); }
float TestDistance(const float first[3], const float second[3]) { return g_WrenchDistance + first[0] * 0.0 + second[0] * 0.0; }
char g_CommandType[16], g_CommandMode[16];
int TestArgument(int argument, char[] output, int size)
{ strcopy(output, size, argument == 1 ? g_CommandType : g_CommandMode); return strlen(output); }
int g_Active = 80, g_Entities = 100, g_Checks;
bool g_Valid[128], g_Placing[128], g_Carried[128], g_Disposable[128];
bool g_TransmitHooked[128], g_TransmitAvailable = true;
bool g_NoBuilder[128], g_NoMode[128];
int g_EdictFlags[128];
int g_Owner[128], g_Mode[128], g_Locked[128], g_Refs[128];
char g_Class[128][40];

bool TestValid(int entity) { return entity > 0 && entity < 128 && g_Valid[entity]; }
int TestRef(int entity) { return TestValid(entity) ? g_Refs[entity] : INVALID_ENT_REFERENCE; }
int TestIndex(int reference)
{
    int entity = reference & 127;
    return TestValid(entity) && g_Refs[entity] == reference ? entity : INVALID_ENT_REFERENCE;
}
bool TestClient(int client) { return client == 1 || client == 2; }
bool GameplayAllowed(int client) { return TestClient(client) && g_Allowed; }
bool GameplayFeatureAllowed(int client, PremiumFeature feature) { return feature >= Feature_Speed && feature < Feature_Count && GameplayAllowed(client); }
TFClassType TestPlayerClass(int client) { return TestClient(client) ? TFClass_Engineer : TFClass_Unknown; }
bool TestAlive(int client) { return TestClient(client) && g_Alive; }
bool TestAccess(int client, const char[] name, int flags, bool overrideOnly) { return TestClient(client) && name[0] && flags == 0 && overrideOnly; }
int TestRule(const char[] name) { return name[0] ? 0 : 1; }
bool TestBuilding(int entity)
{
    return TestValid(entity) && (StrEqual(g_Class[entity], "obj_sentrygun")
        || StrEqual(g_Class[entity], "obj_dispenser") || StrEqual(g_Class[entity], "obj_teleporter"));
}
bool TestHas(int entity, PropType type, const char[] name)
{
    if (entity <= 0 || type != Prop_Send || !name[0]) return false;
    if (StrEqual(name, "m_hBuilder")) return TestBuilding(entity) && !g_NoBuilder[entity];
    if (StrEqual(name, "m_iObjectMode")) return (entity == 80 || TestBuilding(entity)) && !g_NoMode[entity];
    return true;
}
int TestGet(int entity, PropType type, const char[] name, int size = 4, int element = 0)
{
    if (type != Prop_Send || size != 4 || element != 0) return 0;
    if (StrEqual(name, "m_iObjectType")) return g_BuilderType;
    if (StrEqual(name, "m_bCarryingObject")) return g_Carrying;
    if (StrEqual(name, "m_iObjectMode"))
    {
        if (!TestHas(entity, type, name)) ThrowError("Property m_iObjectMode not found on %s", g_Class[entity]);
        return entity == 80 ? g_BuilderMode : g_Mode[entity];
    }
    if (StrEqual(name, "m_bPlacing")) return g_Placing[entity];
    if (StrEqual(name, "m_bCarried")) return g_Carried[entity];
    if (StrEqual(name, "m_bDisposableBuilding")) return g_Disposable[entity];
    return 0;
}
int TestGetEnt(int entity, PropType type, const char[] name)
{
    if (type != Prop_Send) return -1;
    if (StrEqual(name, "m_hBuilder") && !TestHas(entity, type, name))
        ThrowError("Property m_hBuilder not found on %s", g_Class[entity]);
    return StrEqual(name, "m_hActiveWeapon") ? g_Active : g_Owner[entity];
}
void TestSet(int entity, PropType type, const char[] name, int value)
{
    if (type != Prop_Send) return;
    if (StrEqual(name, "m_iObjectMode")) g_Mode[entity] = value;
    if (StrEqual(name, "m_bWasMapPlaced")) g_Locked[entity] = value;
    if (StrEqual(name, "m_bDisposableBuilding")) g_Disposable[entity] = value != 0;
}
bool TestClassname(int entity, char[] output, int size) { strcopy(output, size, g_Class[entity]); return true; }
int TestFind(int start, const char[] classname)
{
    for (int entity = start + 1; entity < 128; entity++)
        if (g_Valid[entity] && StrEqual(g_Class[entity], classname)) return entity;
    return -1;
}
void TestRemove(int entity) { g_Valid[entity] = false; }
bool TestHookEx(int entity, SDKHookType type, SDKHookCB callback)
{
    if (!g_TransmitAvailable || !TestValid(entity) || type != SDKHook_SetTransmit || callback == INVALID_FUNCTION) return false;
    g_TransmitHooked[entity] = true; return true;
}
void TestUnhook(int entity, SDKHookType type, SDKHookCB callback)
{ if (TestValid(entity) && type == SDKHook_SetTransmit && callback != INVALID_FUNCTION) g_TransmitHooked[entity] = false; }
int TestEdictFlags(int entity) { return g_EdictFlags[entity]; }
void TestSetEdictFlags(int entity, int flags) { g_EdictFlags[entity] = flags; }
void TestLogError(const char[] format, any ...) { if (!format[0]) ThrowError("bad log"); }
bool ValidWeapon(int entity) { return TestValid(entity) && entity >= 80; }
int TestSlot(int client, int slot) { return client > 0 && slot == 3 ? 80 : -1; }
int TestEntityCount() { return g_Entities; }
int TestMaxEntities() { return 128; }
void Tell(int client, const char[] format, any ...) { if (client < 0 || !format[0]) ThrowError("bad reply"); }

#define GetClientAbsOrigin TestAbsOrigin
#define GetEntPropVector TestOrigin
#define GetVectorDistance TestDistance
#define GetCmdArg TestArgument
#define GetClientTeam TestTeam
#define GetGameTime TestTime
#define MaxClients 32
#define EntIndexToEntRef TestRef
#define EntRefToEntIndex TestIndex
#define IsValidEntity TestValid
#define ValidClient TestClient
#define TF2_GetPlayerClass TestPlayerClass
#define IsPlayerAlive TestAlive
#define CheckCommandAccess TestAccess
#define GameRules_GetProp TestRule
#define HasEntProp TestHas
#define GetEntProp TestGet
#define GetEntPropEnt TestGetEnt
#define SetEntProp TestSet
#define GetEntityClassname TestClassname
#define FindEntityByClassname TestFind
#define RemoveEntity TestRemove
#define GetPlayerWeaponSlot TestSlot
#define GetEntityCount TestEntityCount
#define GetMaxEntities TestMaxEntities
#define SDKHookEx TestHookEx
#define SDKUnhook TestUnhook
#define GetEdictFlags TestEdictFlags
#define SetEdictFlags TestSetEdictFlags
#define LogError TestLogError
#include "../scripting/premium/utility_buildings.inc"
#include "../scripting/premium/buildings.inc"

void Check(bool condition, const char[] label)
{
    g_Checks++;
    if (!condition) ThrowError("FAIL: %s", label);
}

void Entity(int entity, const char[] classname, int owner = 1)
{
    g_Valid[entity] = true;
    g_Refs[entity] += 128;
    if ((g_Refs[entity] & 127) != entity) g_Refs[entity] += entity;
    g_Owner[entity] = owner;
    g_Mode[entity] = 0; g_Locked[entity] = 0;
    g_Placing[entity] = false; g_Carried[entity] = false; g_Disposable[entity] = false;
    g_TransmitHooked[entity] = false; g_EdictFlags[entity] = FL_EDICT_FULLCHECK;
    g_NoBuilder[entity] = false; g_NoMode[entity] = false;
    strcopy(g_Class[entity], sizeof(g_Class[]), classname);
}

// Model the external client lookup from C_TFPlayer::GetObjectOfType: it checks
// type/mode and dormancy, NOT m_bDisposableBuilding. The previous regression
// tests only asserted the server flag and therefore missed the real PDA block.
int NativeClientUtility(int client, int kind, bool applyTransmit)
{
    for (int entity = 40; entity < 80; entity++)
    {
        if (!g_Valid[entity] || OwnedUtilityKind(client, TestRef(entity)) != kind) continue;
        if (applyTransmit && g_TransmitHooked[entity] && UtilityOwnerTransmit(entity, client) == Plugin_Handled) continue;
        return entity;
    }
    return -1;
}

public int main()
{
    cvTeamDispensers.IntValue = -1; cvTeamEntrances.IntValue = -1; cvTeamExits.IntValue = -1;
    // Aim targets are arbitrary map entities, not necessarily buildings.
    // The host must throw for absent properties, just as SourceMod does.
    char nonBuildings[][] = {"tf_ammo_pack", "func_respawnroomvisualizer", "func_door", "prop_dynamic"};
    for (int i; i < sizeof(nonBuildings); i++)
    {
        Entity(90, nonBuildings[i]);
        Check(OwnedUtilityKind(1, TestRef(90)) == -1, "non-building aim target has no utility kind");
        Check(!PlacedUtility(1, 90, 0), "non-building target is not a placed dispenser");
        Check(!PlacedUtility(1, 90, -1), "invalid utility kind cannot match a non-building");
        Check(UtilityOwnerTransmit(90, 1) == Plugin_Continue, "unrelated transmit entity needs no building properties");
    }
    Entity(90, "obj_dispenser");
    Check(OwnedUtilityKind(1, TestRef(90)) == 0, "owned dispenser is recognised");
    Check(OwnedUtilityKind(2, TestRef(90)) == -1, "foreign utility is rejected");
    g_NoBuilder[90] = true;
    Check(OwnedUtilityKind(1, TestRef(90)) == -1, "building without ownership property is rejected safely");
    Check(UtilityOwnerTransmit(90, 1) == Plugin_Continue, "transmit rejects building without ownership property");
    Entity(90, "obj_teleporter");
    Check(OwnedUtilityKind(1, TestRef(90)) == 1, "entrance is recognised");
    g_Mode[90] = 1;
    Check(OwnedUtilityKind(1, TestRef(90)) == 2, "exit is recognised");
    g_Mode[90] = 2;
    Check(OwnedUtilityKind(1, TestRef(90)) == -1, "unsupported teleporter mode is rejected");
    g_NoMode[90] = true;
    Check(OwnedUtilityKind(1, TestRef(90)) == -1, "teleporter without mode property is rejected safely");
    int oldReference = TestRef(90);
    Entity(90, "tf_ammo_pack");
    Check(OwnedUtilityKind(1, oldReference) == -1, "stale building reference cannot follow a reused ammo-pack edict");
    Check(OwnedUtilityKind(1, INVALID_ENT_REFERENCE) == -1, "invalid reference is safe");
    Check(OwnedUtilityKind(1, 0) == -1, "world target is safe");
    Check(OwnedUtilityKind(1, 1) == -1, "player target is safe");
    g_Valid[90] = false;
    cvDispenserLimit.IntValue = 3; cvEntranceLimit.IntValue = 3; cvExitLimit.IntValue = 3;
    cvTeamSentries.IntValue = 32;
    cvSentryLimit.IntValue = 3;
    Entity(80, "tf_weapon_pda_engineer_build");
    Entity(81, "tf_weapon_wrench");
    Entity(40, "obj_sentrygun");
    TrackPlacedSentry(1, 40);
    Check(!IsBonusSentry(1, 40) && g_Locked[40] == 0, "ordinary primary stays upgradeable");
    UpdateSentryPda(1);
    Check(g_Mode[40] == 0, "entity reserve blocks extra placement before edict exhaustion");
    g_Entities = 50;
    UpdateSentryPda(1);
    Check(g_Mode[40] == 1, "PDA mode opens next native blueprint slot");
    Entity(41, "obj_sentrygun");
    TrackPlacedSentry(1, 41);
    Check(IsBonusSentry(1, 41) && g_Locked[41] == 1, "second native sentry is level-one extra");
    g_Active = 81;
    UpdateSentryPda(1);
    Check(g_Mode[40] == 0 && g_Mode[41] == 0, "leaving PDA restores real sentry modes");
    g_Active = 80;
    UpdateSentryPda(1);
    Check(g_Mode[40] == 1 && g_Mode[41] == 1, "both existing guns permit third blueprint");
    Entity(42, "obj_sentrygun");
    TrackPlacedSentry(1, 42);
    MaintainExtraSentries(1);
    Check(IsBonusSentry(1, 42) && g_Locked[42] == 1, "third gun is also locked at one");
    Check(g_Mode[40] == 0 && g_Mode[41] == 0 && g_Mode[42] == 0, "at cap PDA modes return to native limit");
    g_Carrying = true; g_Carried[41] = true;
    UpdateSentryPda(1);
    Check(g_Mode[40] == 1 && g_Mode[41] == 0 && g_Mode[42] == 1, "carried extra is the native mode-zero redeploy target at cap");
    g_Carrying = false; g_Carried[41] = false;
    UpdateSentryPda(1);
    Check(g_Mode[40] == 0 && g_Mode[42] == 0, "redeploy restores other guns at cap");
    Entity(43, "obj_sentrygun");
    TrackPlacedSentry(1, 43);
    Check(!g_Valid[43], "placement race cannot create fourth sentry");
    g_Valid[40] = false;
    Entity(44, "obj_sentrygun");
    TrackPlacedSentry(1, 44);
    Check(!IsBonusSentry(1, 44) && g_Locked[44] == 0, "replacement primary is upgradeable while extras survive");
    cvSentryLimit.IntValue = 2;
    MaintainExtraSentries(1);
    Check(CountOwnedBuildings(1) == 2 && g_Valid[44], "lowering total cap removes newest extra, preserves primary");
    Entity(45, "obj_sentrygun", 2);
    g_Allowed = false;
    MaintainExtraSentries(1);
    Check(CountOwnedBuildings(1) == 1 && g_Valid[44], "access/team restriction removes only tracked extras");
    Check(g_Valid[45] && g_Owner[45] == 2, "another player sentry never changed");
    g_Allowed = true; cvSentryLimit.IntValue = 3;
    UpdateSentryPda(1);
    Check(g_Mode[44] == 1, "PDA mask can be reacquired");
    // Reuse an edict with a new generation; old ownership must not restore its mode.
    Entity(44, "obj_sentrygun", 2); g_Mode[44] = 1;
    RestoreSentryModes(1);
    Check(g_Mode[44] == 1, "stale entity reference cannot modify a reused gun");
    Entity(46, "obj_sentrygun");
    Entity(47, "obj_sentrygun"); g_Placing[47] = true;
    Check(CountOwnedBuildings(1) == 1, "unplaced blueprint does not consume a completed sentry slot");
    Entity(48, "obj_dispenser"); Entity(49, "obj_teleporter"); Entity(50, "obj_teleporter");
    Entity(51, "obj_dispenser"); Entity(52, "obj_teleporter");
    UpdateSentryPda(1);
    Check(g_Mode[46] == 0, "six native object slots prevent ownership overflow");
    RemoveExtraSentries(1);
    Check(g_Valid[46] && g_Valid[45], "cleanup preserves all untracked native guns");
    // Fresh world, two Engineers on RED. The primary never consumes team budget.
    for (int entity = 40; entity < 80; entity++) g_Valid[entity] = false;
    RemoveExtraSentries(1); RemoveExtraSentries(2);
    g_Teams[2] = 2; cvTeamSentries.IntValue = 1;
    g_Active = 80; strcopy(g_Class[80], sizeof(g_Class[]), "tf_weapon_builder");
    Entity(40, "obj_sentrygun", 1); TrackPlacedSentry(1, 40);
    Entity(41, "obj_sentrygun", 2); TrackPlacedSentry(2, 41);
    Check(CountTeamExtraSentries(2) == 0, "ordinary primaries excluded from shared budget");
    Entity(42, "obj_sentrygun", 1); TrackPlacedSentry(1, 42);
    Check(CountTeamExtraSentries(2) == 1 && IsBonusSentry(1, 42), "first Engineer consumes the last shared slot");
    Check(SentryPlacementBlocked(2), "another Engineer's open blueprint is blocked before spending metal");
    g_BuilderType = 0;
    Check(!SentryPlacementBlocked(2), "team cap does not block dispenser placement");
    g_BuilderType = 2;
    Entity(43, "obj_sentrygun", 2); TrackPlacedSentry(2, 43);
    Check(!g_Valid[43] && CountTeamExtraSentries(2) == 1, "final placement check closes competing-slot race");
    g_Carrying = true;
    Check(!SentryPlacementBlocked(2), "shared cap cannot block hauling an existing gun");
    g_Carrying = false; g_Valid[41] = false;
    Entity(44, "obj_sentrygun", 2); TrackPlacedSentry(2, 44);
    Check(g_Valid[44] && !IsBonusSentry(2, 44), "replacement primary remains available at team cap");
    cvTeamSentries.IntValue = 3;
    Entity(45, "obj_sentrygun", 2); TrackPlacedSentry(2, 45);
    Check(CountTeamExtraSentries(2) == 2, "both players' extra guns counted together");
    cvTeamSentries.IntValue = 1; EnforceTeamSentryCaps();
    Check(g_Valid[42] && !g_Valid[45] && g_Valid[40] && g_Valid[44], "live lower cap removes newest extra, preserves both primaries");
    g_Teams[2] = 3;
    Entity(46, "obj_sentrygun", 2); TrackPlacedSentry(2, 46);
    Check(CountTeamExtraSentries(2) == 1 && CountTeamExtraSentries(3) == 1, "RED and BLU budgets are independent");
    cvTeamSentries.IntValue = 0; EnforceTeamSentryCaps();
    Check(CountTeamExtraSentries(2) == 0 && CountTeamExtraSentries(3) == 0 && g_Valid[40] && g_Valid[44], "zero team budget removes extras only");
    // Shared six-slot construction, with independent dispenser / entrance / exit limits.
    for (int entity = 40; entity < 80; entity++) g_Valid[entity] = false;
    RemoveExtraBuildings(1); RemoveExtraBuildings(2);
    cvTeamSentries.IntValue = 32; cvSentryLimit.IntValue = 3;
    g_BuilderType = 0; g_BuilderMode = 0; g_Carrying = false;
    strcopy(g_Class[80], sizeof(g_Class[]), "tf_weapon_pda_engineer_build");
    Entity(40, "obj_sentrygun"); TrackPlacedSentry(1, 40);
    Entity(41, "obj_dispenser"); TrackPlacedUtility(1, 41);
    g_EdictFlags[41] = FL_EDICT_ALWAYS | FL_EDICT_CHANGED;
    Entity(42, "obj_teleporter"); TrackPlacedUtility(1, 42);
    Entity(43, "obj_teleporter"); g_Mode[43] = 1; TrackPlacedUtility(1, 43);
    Check(CountOwnedBuildings(1, false) == 4 && CountUtilities(1, 0) == 1, "ordinary four buildings counted in shared capacity");
    Check(CountUtilities(1, 1) == 1 && CountUtilities(1, 2) == 1, "teleporter ends counted separately");
    UpdateUtilityPda(1);
    Check(g_Disposable[41] && g_Disposable[42] && g_Disposable[43], "PDA opens additional native utility blueprints");
    Check(NativeClientUtility(1, 0, false) == 41 && NativeClientUtility(1, 1, false) == 42 && NativeClientUtility(1, 2, false) == 43, "disposable flags alone do not bypass the native client menu");
    Check(NativeClientUtility(1, 0, true) == -1 && NativeClientUtility(1, 1, true) == -1 && NativeClientUtility(1, 2, true) == -1, "owner-only dormancy removes the actual client lookup block for all utility directions");
    Check(!(g_EdictFlags[41] & FL_EDICT_ALWAYS) && (g_EdictFlags[41] & FL_EDICT_CHANGED), "always-transmit fast path is bypassed without losing dirty flags");
    Check(g_TransmitHooked[41] && g_TransmitHooked[42] && g_TransmitHooked[43], "client PDA mask hooks dispenser and both teleporter ends");
    Check(UtilityOwnerTransmit(41, 1) == Plugin_Handled && UtilityOwnerTransmit(42, 1) == Plugin_Handled && UtilityOwnerTransmit(43, 1) == Plugin_Handled, "owner build selection omits all masked utility types");
    Check(UtilityOwnerTransmit(41, 2) == Plugin_Continue && UtilityOwnerTransmit(42, 2) == Plugin_Continue, "other viewers keep seeing functioning buildings");
    strcopy(g_Class[80], sizeof(g_Class[]), "tf_weapon_builder");
    Check(UtilityOwnerTransmit(41, 1) == Plugin_Continue && UtilityOwnerTransmit(42, 1) == Plugin_Continue, "blueprint placement shows existing buildings again");
    strcopy(g_Class[80], sizeof(g_Class[]), "tf_weapon_pda_engineer_build");
    Check(g_Mode[42] == 0 && g_Mode[43] == 1 && g_Owner[42] == 1 && g_Owner[43] == 1, "teleporter modes and builders remain native for pairing");
    Check(!g_Disposable[40], "utility masks never change sentry flags");
    g_Active = 81; UpdateUtilityPda(1);
    Check(!g_Disposable[41] && !g_Disposable[42] && !g_Disposable[43], "weapon exit restores all claimed utility flags");
    Check(!g_TransmitHooked[41] && !g_TransmitHooked[42] && !g_TransmitHooked[43], "weapon exit removes owned transmit hooks");
    Check((g_EdictFlags[41] & FL_EDICT_ALWAYS) && (g_EdictFlags[41] & FL_EDICT_CHANGED), "original transmit policy and dirty flags restored on exit");
    g_Active = 80;
    strcopy(g_Class[80], sizeof(g_Class[]), "tf_weapon_pda_engineer_destroy"); UpdateUtilityPda(1);
    Check(!g_Disposable[41] && !g_Disposable[42], "native destruction PDA retains real ownership flags");
    strcopy(g_Class[80], sizeof(g_Class[]), "tf_weapon_pda_engineer_build"); UpdateUtilityPda(1);
    ProtectUtilityWrench(2, "tf_weapon_wrench");
    Check(g_Disposable[41], "enemy wrench does not disturb another team's PDA masks");
    g_WrenchDistance = 400.0; ProtectUtilityWrench(1, "tf_weapon_wrench");
    Check(g_Disposable[41], "distant wrench swings cannot disrupt utility PDA availability");
    g_WrenchDistance = 100.0;
    ProtectUtilityWrench(1, "tf_weapon_wrench"); UpdateUtilityPda(1);
    Check(!g_Disposable[41] && !g_Disposable[42] && g_UtilityMaskResume[1] > 100.0, "wrench swing suspends masks through delayed native impact");
    g_UtilityMaskResume[1] = 0.0; UpdateUtilityPda(1);
    Check(g_Disposable[41], "PDA resumes after wrench protection interval");
    g_RoundActive = false; UpdateUtilityPda(1);
    Check(!g_Disposable[41] && !g_Disposable[42], "round closure restores utility flags");
    g_RoundActive = true;
    Entity(44, "obj_dispenser"); TrackPlacedUtility(1, 44);
    Check(IsBonusUtility(1, 0, TestRef(44)) && !g_Locked[44], "second dispenser tracked without level-one sentry restriction");
    cvSentryLimit.IntValue = 1; MaintainExtraBuildings(1);
    Check(g_Valid[44] && g_Disposable[41], "sentry cap one does not disable extra dispensers");
    cvSentryLimit.IntValue = 3;
    Entity(45, "obj_dispenser"); TrackPlacedUtility(1, 45); MaintainExtraBuildings(1);
    Check(CountOwnedBuildings(1, false) == 6 && CountUtilities(1, 0) == 3, "three dispensers fit alongside sentry and teleporter pair");
    Check(!g_Disposable[41] && !g_Disposable[42] && !g_Disposable[43] && !g_Disposable[44] && !g_Disposable[45], "all masks restored at shared six-building cap");
    strcopy(g_Class[80], sizeof(g_Class[]), "tf_weapon_builder");
    Check(BuildingPlacementBlocked(1), "full shared list blocks dispenser placement before metal spending");
    g_BuilderType = 1; g_BuilderMode = 1;
    Check(BuildingPlacementBlocked(1), "full shared list blocks a new exit despite spare per-exit limit");
    g_BuilderType = 2; g_BuilderMode = 0;
    Check(BuildingPlacementBlocked(1), "full shared list also blocks a sentry blueprint");
    Entity(46, "obj_dispenser"); TrackPlacedUtility(1, 46);
    Check(!g_Valid[46] && CountOwnedBuildings(1, false) == 6, "late utility placement cannot exceed six objects");
    Entity(46, "obj_sentrygun"); TrackPlacedSentry(1, 46);
    Check(!g_Valid[46], "late sentry placement shares final overflow guard");
    g_Carrying = true; g_Carried[44] = true; g_BuilderType = 0;
    UpdateUtilityPda(1);
    Check(!g_Disposable[44] && g_Disposable[41] && g_Disposable[45], "carried dispenser remains visible to native redeployment at cap");
    Check(!BuildingPlacementBlocked(1), "existing building hauling is permitted at shared cap");
    g_Carrying = false; g_Carried[44] = false; RestoreBuildingPda(1);
    cvDispenserLimit.IntValue = 2; MaintainUtilityBuildings(1);
    Check(g_Valid[41] && g_Valid[44] && !g_Valid[45], "lower utility cap removes newest extra and preserves primary");
    Check(CountOwnedBuildings(1, false) == 5, "lowering cap releases a shared slot");
    Check(BuildingPlacementBlocked(1), "per-dispenser limit enforced even with a free shared slot");
    strcopy(g_CommandType, sizeof(g_CommandType), "0");
    Check(LimitNativeBuildings(1, "build", 1) == Plugin_Handled, "native build bind enforces dispenser cap");
    strcopy(g_CommandType, sizeof(g_CommandType), "3");
    Check(LimitNativeBuildings(1, "build", 1) == Plugin_Continue, "legacy exit bind uses exit limit");
    strcopy(g_CommandType, sizeof(g_CommandType), "1"); strcopy(g_CommandMode, sizeof(g_CommandMode), "1");
    Check(LimitNativeBuildings(1, "build", 2) == Plugin_Continue, "modern exit bind is accepted within limits");
    cvExitLimit.IntValue = 1;
    Check(LimitNativeBuildings(1, "build", 2) == Plugin_Handled, "entrance and exit limits are independently configurable");
    strcopy(g_CommandMode, sizeof(g_CommandMode), "0");
    Check(LimitNativeBuildings(1, "build", 2) == Plugin_Continue, "exit cap cannot block available entrance slot");
    strcopy(g_CommandType, sizeof(g_CommandType), "garbage");
    Check(LimitNativeBuildings(1, "build", 2) == Plugin_Continue, "malformed command remains subject to engine validation");
    g_Valid[41] = false;
    Entity(47, "obj_dispenser"); TrackPlacedUtility(1, 47);
    Check(!IsBonusUtility(1, 0, TestRef(47)) && g_Valid[44], "replacement utility primary is not misclassified as a bonus");
    cvDispenserLimit.IntValue = 3; UpdateUtilityPda(1);
    Check(g_Disposable[47], "new primary can be masked for additional placement");
    Entity(47, "obj_dispenser", 2); g_Disposable[47] = true;
    RestoreUtilityMasks(1);
    Check(g_Disposable[47], "stale reference cannot restore a reused foreign utility entity");
    Entity(48, "obj_dispenser", 1); g_Disposable[48] = true;
    UpdateUtilityPda(1); RestoreUtilityMasks(1);
    Check(g_Disposable[48], "pre-existing disposable flags are never claimed or cleared");
    g_Allowed = false; MaintainExtraBuildings(1);
    Check(!g_Valid[44] && g_Valid[48] && g_Valid[42] && g_Valid[43], "access loss removes owned extras and preserves untracked primaries");
    Check(g_Valid[47] && g_Disposable[47], "access cleanup cannot remove another player's reused entity");
    g_Allowed = true;
    for (int entity = 40; entity < 80; entity++) g_Valid[entity] = false;
    RemoveExtraBuildings(1); RemoveExtraBuildings(2);
    cvEntranceLimit.IntValue = 3; cvExitLimit.IntValue = 3;
    for (int end; end < 6; end++)
    {
        Entity(40 + end, "obj_teleporter"); g_Mode[40 + end] = end % 2;
        TrackPlacedUtility(1, 40 + end);
    }
    Check(CountUtilities(1, 1) == 3 && CountUtilities(1, 2) == 3 && CountOwnedBuildings(1, false) == 6, "three complete pairs consume exactly all six slots");
    UpdateUtilityPda(1);
    Check(!g_Disposable[40] && !g_Disposable[45] && g_Mode[40] == 0 && g_Mode[45] == 1, "three pairs retain unmasked native entrance/exit modes at cap");
    g_Carrying = true; g_Carried[42] = true; g_BuilderType = 1; g_BuilderMode = 0;
    UpdateUtilityPda(1);
    Check(!g_Disposable[42] && g_Disposable[40] && g_Disposable[44] && !g_Disposable[41], "hauling masks only other ends of the carried teleporter direction");
    Check(!BuildingPlacementBlocked(1), "carried teleporter redeployment is permitted with six endpoints");
    g_Carrying = false; g_Carried[42] = false;
    cvEntranceLimit.IntValue = 2; MaintainUtilityBuildings(1);
    Check(!g_Valid[44] && g_Valid[40] && g_Valid[42] && g_Valid[45], "lowering entrance cap preserves exits and native primary");
    Check(!g_Disposable[40] && !g_Disposable[41], "each direction at its own cap does not mask endpoints");
    cvExitLimit.IntValue = 1; MaintainUtilityBuildings(1);
    Check(!g_Valid[43] && !g_Valid[45] && g_Valid[41], "lowering exit cap removes only extra exits");
    RemoveExtraBuildings(1);
    Check(g_Valid[40] && g_Valid[41] && !g_Valid[42], "complete cleanup keeps the ordinary pair and removes the extra entrance");
    Check(UtilityKind(1, 2) == -1 && UtilityKind(0, 1) == -1 && UtilityKind(2, 0) == -1, "invalid utility modes and sentries are not misclassified");
    for (int entity = 40; entity < 80; entity++) g_Valid[entity] = false;
    RemoveExtraBuildings(1); RemoveExtraBuildings(2);
    cvDispenserLimit.IntValue = 3; cvEntranceLimit.IntValue = 3; cvExitLimit.IntValue = 3;
    strcopy(g_Class[80], sizeof(g_Class[]), "tf_weapon_pda_engineer_build");
    Entity(40, "obj_dispenser"); g_TransmitAvailable = false; UpdateUtilityPda(1);
    Check(!g_Disposable[40] && !g_TransmitHooked[40], "failed transmit hook cannot leave a half-applied server mask");
    g_TransmitAvailable = true; g_EdictFlags[40] = FL_EDICT_DONTSEND; UpdateUtilityPda(1);
    Check(!g_Disposable[40] && !g_TransmitHooked[40] && (g_EdictFlags[40] & FL_EDICT_DONTSEND), "another plugin's hidden entity policy is respected");
    g_EdictFlags[40] = FL_EDICT_ALWAYS; UpdateUtilityPda(1);
    Check(g_Disposable[40] && g_TransmitHooked[40], "healthy hook can reacquire the mask after failure");
    g_EdictFlags[40] = FL_EDICT_DONTSEND; RestoreUtilityMasks(1);
    Check((g_EdictFlags[40] & FL_EDICT_DONTSEND) && !g_Disposable[40] && !g_TransmitHooked[40], "later foreign transmit changes survive restoration");
    for (int kind; kind<3;kind++)
    {
        for (int entity=40;entity<80;entity++) g_Valid[entity]=false;
        RemoveExtraBuildings(1);RemoveExtraBuildings(2);
        g_Teams[2]=2;cvTeamDispensers.IntValue=1;cvTeamEntrances.IntValue=1;cvTeamExits.IntValue=1;
        char classname[32];strcopy(classname,sizeof(classname),kind==0?"obj_dispenser":"obj_teleporter");
        Entity(40,classname);g_Mode[40]=kind==2?1:0;TrackPlacedUtility(1,40);
        Entity(41,classname);g_Mode[41]=g_Mode[40];TrackPlacedUtility(1,41);
        Check(CountTeamExtraUtilities(2,kind)==1,"team counts tracked extras only");
        Entity(42,classname,2);g_Mode[42]=g_Mode[40];TrackPlacedUtility(2,42);
        Check(g_Valid[42],"team extra cap never prevents another engineer primary");
        Entity(43,classname,2);g_Mode[43]=g_Mode[40];TrackPlacedUtility(2,43);
        Check(!g_Valid[43],"placement race cannot exceed shared team extra cap");
        g_BuilderType=kind==0?0:1;g_BuilderMode=kind==2?1:0;g_Active=80;
        strcopy(g_Class[80],sizeof(g_Class[]),"tf_weapon_builder");
        Check(BuildingPlacementBlocked(2),"team cap stops blueprint before metal spending");
        g_Carrying=true;Check(!BuildingPlacementBlocked(2),"existing carried utility remains redeployable at team cap");g_Carrying=false;
        cvTeamDispensers.IntValue=0;cvTeamEntrances.IntValue=0;cvTeamExits.IntValue=0;EnforceTeamUtilityCaps();
        Check(g_Valid[40]&&g_Valid[42]&&!g_Valid[41],"lowering team cap removes extras and preserves both primaries");
        cvTeamDispensers.IntValue=-1;cvTeamEntrances.IntValue=-1;cvTeamExits.IntValue=-1;
        Entity(43,classname,2);g_Mode[43]=g_Mode[40];TrackPlacedUtility(2,43);
        Check(g_Valid[43]&&TeamUtilityRoom(1,kind),"minus one disables team limit");
        g_Teams[2]=3;
        Check(CountTeamExtraUtilities(2,kind)==0&&CountTeamExtraUtilities(3,kind)==1,"RED and BLU budgets are independent");
    }
    PrintToServer("PASS: %d sentry ownership assertions", g_Checks);
    return g_Checks;
}
