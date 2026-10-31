#include "rule_policy.inc"
enum struct PremiumClassRule
{
    float ammo;
    float speed;
    float gravity;
    int bhop;
    float gain;
    float cap;
    int doubleJump;
    int flight;
    int airblast;
    float flightSpeed;
    float flightLift;
    float flightAcceleration;
    int teleport;
}
PremiumClassRule g_ClassRules[10];
bool g_MapFlight = true, g_MapTeleport = true;
char g_RuleMap[128];
char g_PendingProfile[24];
char g_ClassNames[][] = {"unknown", "scout", "sniper", "soldier", "demoman", "medic", "heavy", "pyro", "spy", "engineer"};

void InitializeRules()
{
    for (int class; class < 10; class++)
    { g_ClassRules[class].bhop = -1; g_ClassRules[class].doubleJump = -1; g_ClassRules[class].flight = -1; g_ClassRules[class].airblast = -1; g_ClassRules[class].teleport = -1; }
}

void ApplyPendingProfile()
{
    if (!g_PendingProfile[0]) return;
    char profile[24]; strcopy(profile, sizeof(profile), g_PendingProfile);
    g_PendingProfile[0] = '\0';
    char path[PLATFORM_MAX_PATH]; FormatEx(path, sizeof(path), "cfg/sourcemod/premium_profiles/%s.cfg", profile);
    if (!FileExists(path)) { LogError("Queued Premium profile missing: %s", path); return; }
    ServerCommand("exec sourcemod/premium_profiles/%s.cfg", profile);
    ServerExecute();
    LogMessage("Applied Premium profile at round start: %s", profile);
}

float ReadRuleNumber(KeyValues file, const char[] key, float fallback, float minimum, float maximum, bool &valid)
{
    char raw[32]; file.GetString(key, raw, sizeof(raw), "");
    if (!raw[0]) return fallback;
    float value = StringToFloat(raw);
    if (!PremiumRuleNumber(raw) || strlen(raw) >= sizeof(raw) - 1
        || (value != 0.0 && (value < minimum || value > maximum)))
    { LogError("Premium rules: invalid %s value '%s'; file rejected.", key, raw); valid = false; return fallback; }
    return value;
}

int ReadRuleToggle(KeyValues file, const char[] key, bool &valid)
{
    float value = ReadRuleNumber(file, key, -1.0, -1.0, 1.0, valid);
    if (value != -1.0 && value != 0.0 && value != 1.0) valid = false;
    return RoundToFloor(value);
}

bool LoadPremiumRules()
{
    char path[PLATFORM_MAX_PATH]; BuildPath(Path_SM, path, sizeof(path), "configs/premium_rules.cfg");
    KeyValues file = new KeyValues("PremiumRules");
    if (!file.ImportFromFile(path))
    {
        LogError("Premium rules missing or invalid: %s. Install the supplied file; flight/teleport are blocked until a valid file is loaded.", path);
        delete file; g_MapFlight = false; g_MapTeleport = false; return false;
    }
    char rootName[32]; file.GetSectionName(rootName, sizeof(rootName));
    if (!StrEqual(rootName, "PremiumRules"))
    { LogError("Premium rules: root section must be PremiumRules."); delete file; g_MapFlight = false; g_MapTeleport = false; return false; }
    PremiumClassRule rules[10];
    bool valid = true, flight = true, teleport = true;
    for (int class; class < 10; class++)
    { rules[class].bhop = -1; rules[class].doubleJump = -1; rules[class].flight = -1; rules[class].airblast = -1; rules[class].teleport = -1; }
    if (file.JumpToKey("Classes"))
    {
        for (int class = 1; class <= 9; class++)
        {
            if (!file.JumpToKey(g_ClassNames[class])) continue;
            rules[class].ammo = ReadRuleNumber(file, "ammo_multiplier", 0.0, 1.0, 10.0, valid);
            rules[class].speed = ReadRuleNumber(file, "speed_multiplier", 0.0, 1.0, 2.0, valid);
            rules[class].gravity = ReadRuleNumber(file, "gravity_multiplier", 0.0, 0.1, 2.0, valid);
            rules[class].bhop = ReadRuleToggle(file, "bhop", valid);
            rules[class].gain = ReadRuleNumber(file, "bhop_gain", 0.0, 1.0, 1.5, valid);
            rules[class].cap = ReadRuleNumber(file, "bhop_cap", 0.0, 300.0, 1000.0, valid);
            rules[class].doubleJump = ReadRuleToggle(file, "doublejump", valid);
            rules[class].flight = ReadRuleToggle(file, "flight", valid);
            rules[class].airblast = ReadRuleToggle(file, "airblast", valid);
            rules[class].flightSpeed = ReadRuleNumber(file, "flight_speed", 0.0, 100.0, 800.0, valid);
            rules[class].flightLift = ReadRuleNumber(file, "flight_lift", 0.0, 50.0, 800.0, valid);
            rules[class].flightAcceleration = ReadRuleNumber(file, "flight_acceleration", 0.0, 100.0, 3000.0, valid);
            rules[class].teleport = ReadRuleToggle(file, "teleport", valid);
            file.GoBack();
        }
        file.GoBack();
    }
    GetCurrentMap(g_RuleMap, sizeof(g_RuleMap));
    if (file.JumpToKey("Maps") && file.GotoFirstSubKey())
    {
        do
        {
            char pattern[128]; file.GetSectionName(pattern, sizeof(pattern));
            int mapFlight = ReadRuleToggle(file, "flight", valid), mapTeleport = ReadRuleToggle(file, "teleport", valid);
            // Match full workshop paths or the base map name. All matching bans apply.
            int slash = FindCharInString(g_RuleMap, '/', true);
            if (PremiumMapMatches(pattern, g_RuleMap) || (slash >= 0 && PremiumMapMatches(pattern, g_RuleMap[slash + 1])))
            { if (mapFlight == 0) flight = false; if (mapTeleport == 0) teleport = false; }
        } while (file.GotoNextKey());
    }
    delete file;
    if (!valid) { g_MapFlight = false; g_MapTeleport = false; return false; }
    for (int class; class < 10; class++) g_ClassRules[class] = rules[class];
    g_MapFlight = flight; g_MapTeleport = teleport;
    return true;
}

int RuleClass(int client)
{
    int class = view_as<int>(TF2_GetPlayerClass(client));
    return class >= 1 && class <= 9 ? class : 0;
}
float EffectiveAmmoMultiplier(int client) { return PremiumRuleFloat(cvAmmo.FloatValue, g_ClassRules[RuleClass(client)].ammo); }
float EffectiveClassSpeed(int client) { int class = RuleClass(client); return PremiumRuleFloat(class ? cvClassSpeed[class].FloatValue : 1.0, g_ClassRules[class].speed); }
float EffectiveGravity(int client) { return PremiumRuleFloat(cvGravity.FloatValue, g_ClassRules[RuleClass(client)].gravity); }
bool ConfiguredBhop(int client) { return PremiumToggle(cvBhop.BoolValue, g_ClassRules[RuleClass(client)].bhop); }
bool BhopEnabled(int client) { return GameplayFeatureAllowed(client, Feature_Bhop) && ConfiguredBhop(client); }
float BhopGain(int client) { return PremiumRuleFloat(cvBhopGain.FloatValue, g_ClassRules[RuleClass(client)].gain); }
float BhopCap(int client) { return PremiumRuleFloat(cvBhopCap.FloatValue, g_ClassRules[RuleClass(client)].cap); }
bool ConfiguredDoubleJump(int client) { return PremiumToggle(cvDoubleJump.BoolValue, g_ClassRules[RuleClass(client)].doubleJump); }
bool DoubleJumpEnabled(int client) { return GameplayFeatureAllowed(client, Feature_DoubleJump) && ConfiguredDoubleJump(client); }
bool ConfiguredFlight(int client) { return g_MapFlight && PremiumToggle(cvFlight.BoolValue, g_ClassRules[RuleClass(client)].flight); }
bool FlightEnabled(int client) { return GameplayFeatureAllowed(client, Feature_Flight) && ConfiguredFlight(client); }
bool ConfiguredAirblast(int client) { return g_MapFlight && PremiumToggle(cvAirblast.BoolValue, g_ClassRules[RuleClass(client)].airblast); }
bool AirblastEnabled(int client) { return GameplayFeatureAllowed(client, Feature_Airblast) && ConfiguredAirblast(client); }
float FlightSpeed(int client) { return PremiumRuleFloat(cvFlightSpeed.FloatValue, g_ClassRules[RuleClass(client)].flightSpeed); }
float FlightLift(int client) { return PremiumRuleFloat(cvFlightLift.FloatValue, g_ClassRules[RuleClass(client)].flightLift); }
float FlightAcceleration(int client) { return PremiumRuleFloat(cvFlightAcceleration.FloatValue, g_ClassRules[RuleClass(client)].flightAcceleration); }
bool ConfiguredTeleport(int client) { return g_MapTeleport && PremiumToggle(cvTeleport.BoolValue, g_ClassRules[RuleClass(client)].teleport); }
bool TeleportEnabled(int client) { return GameplayFeatureAllowed(client, Feature_Teleport) && ConfiguredTeleport(client); }

public Action CommandRulesReload(int client, int args)
{
    if (!LoadPremiumRules()) { Tell(client, "%t", "Rules Failed"); return Plugin_Handled; }
    for (int target = 1; target <= MaxClients; target++) if (ValidClient(target)) { QueueEffects(target); ResetMovement(target); }
    Tell(client, "%t", "Rules Reloaded");
    return Plugin_Handled;
}
