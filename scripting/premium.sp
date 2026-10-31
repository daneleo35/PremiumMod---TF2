/**
 * Premium Mod 3 — a new implementation for TF2 / SourceMod 1.12.
 * Copyright (C) 2026 daneleo. GPL-3.0-or-later, with the SourceMod linking exception.
 * Original feature concept: daneleo and noodleboy347. No legacy code is executed.
 */
#pragma semicolon 1
#pragma newdecls required

#include <sourcemod>
#include <sdktools>
#include <sdkhooks>
#include <tf2_stocks>
#include <clientprefs>
#undef REQUIRE_PLUGIN
#tryinclude <tf2attributes>
#define REQUIRE_PLUGIN
#if !defined _tf2attributes_included
 #error Missing tf2attributes.inc: download from https://github.com/FlaminSarge/tf2attributes/releases and put it in scripting/include
#endif
#pragma newdecls optional
#tryinclude <morecolors>
#pragma newdecls required
#if !defined _colors_included
 #error Missing morecolors.inc: download from https://github.com/DoctorMcKay/sourcemod-plugins and put it in scripting/include
#endif
#include "premium/core.inc"

#define VERSION "3.0.0-rc13"
#define STEAM_LENGTH 32
#define MEMBER_TABLE "premium_v3_members"
#define TRIAL_TABLE "premium_v3_trials"

public Plugin myinfo =
{
    name = "[TF2] Premium Mod — Rebuilt",
    author = "daneleo (rewrite), original concept noodleboy347",
    description = "Persistent premium membership, trials and configurable TF2 perks",
    version = VERSION,
    url = "https://forums.alliedmods.net/showthread.php?t=225876"
};

enum struct PlayerState
{
    char steam[STEAM_LENGTH];
    bool loaded;
    bool member;
    bool usedTrial;
    bool trialPending;
    int trialUntil;
    int revision;
    bool effective;
    bool dark;
    bool customColor;
    int rgb[3];
    int fov;
    bool saved;
    int savedTeam;
    float location[3];
    float angles[3];
    float nextTeleport;
    int previousButtons;
    float previousVelocity[3];
    bool wasGrounded;
    bool trimpJump;
    float flightInput[2];
    float flightAngles[2];
    int flightButtons;
    int flightTick;
    float nextSwap;
    bool spawnPending;
    bool framePending;
    Handle respawn;
    bool gravityOwned;
    float oldGravity;
    float appliedGravity;
    bool colorOwned;
    int oldColor[4];
    int appliedColor[4];
    bool fovOwned;
    int oldFov;
    int appliedFov;
    int bonusLifeHealth;
}

#include "premium/attributes.inc"

PlayerState g_Player[MAXPLAYERS + 1];
AttributeState g_Speed[MAXPLAYERS + 1];
AttributeState g_MetalCapacity[MAXPLAYERS + 1];
AttributeState g_ThrowableCapacity[MAXPLAYERS + 1];
Database g_Database;
bool g_DatabaseReady;
bool g_Connecting;
bool g_MySQL;
int g_Generation;
Handle g_Heartbeat;
float g_NextDatabaseRetry;
float g_NextAdvert;
bool g_Stopping;
bool g_RoundActive;
bool g_DependencyReady;
ConVar cvEnable, cvFlags, cvDatabase, cvMySQL, cvTrial, cvTrialSeconds;
ConVar cvAds, cvAdInterval, cvWelcome, cvJoin, cvTag, cvFeatures;
ConVar cvSpeed, cvClassSpeed[10], cvGravity, cvHealth, cvAmmo, cvLegacyClip;
ConVar cvKillHealth, cvAssistHealth, cvOverheal, cvColor, cvDark, cvGlow, cvGlowStyle;
ConVar cvFov, cvDefaultFov, cvBalls, cvJarate, cvUber, cvRespawn;
ConVar cvSwap, cvCloak, cvMetal, cvFakeKill, cvArrows;
ConVar cvBhop, cvBhopGain, cvBhopCap, cvDoubleJump, cvFlight, cvTeleport, cvTeleportCooldown;
ConVar cvFlightSpeed, cvFlightLift, cvFlightAcceleration, cvSentryLimit, cvChatTags, cvMemberTag, cvTrialTag;
ConVar cvCloakMultiplier, cvDeadRingerMultiplier, cvGameplayTeams, cvSwapCooldown, cvSwapDifference;
ConVar cvDispenserLimit, cvEntranceLimit, cvExitLimit;
ConVar cvTeamSentries, cvEngineerHud, cvSentryLabels, cvReminders, cvTagStyles, cvPersonalGlow;
ConVar cvCelebration, cvCelebrationSeconds, cvCelebrationBudget;
ConVar cvAirblast, cvAirblastForce, cvAirblastLift, cvAirblastCap, cvAirblastCooldown;

#include "premium/economy_state.inc"
#include "premium/feature_access.inc"
#include "premium/options.inc"
#include "premium/rules.inc"
#include "premium/celebrations.inc"
#include "premium/config.inc"
#include "premium/dependencies.inc"
#include "premium/glow.inc"
#include "premium/glow_host.inc"
#include "premium/cloak.inc"
#include "premium/flight.inc"
#include "premium/airblast.inc"
#include "premium/ammo.inc"
#include "premium/state.inc"
#include "premium/storage.inc"
#include "premium/economy_outbox.inc"
#include "premium/economy_sql.inc"
#include "premium/economy.inc"
#include "premium/effects.inc"
#include "premium/commands.inc"
#include "premium/utility_buildings.inc"
#include "premium/buildings.inc"
#include "premium/chat.inc"
#include "premium/tooltip.inc"
#include "premium/presentation.inc"

public APLRes AskPluginLoad2(Handle myself, bool late, char[] error, int maximum)
{
    if (GetEngineVersion() != Engine_TF2)
    {
        strcopy(error, maximum, "Premium Mod requires Team Fortress 2.");
        return APLRes_SilentFailure;
    }
    return APLRes_Success;
}

public void OnPluginStart()
{
    LoadTranslations("common.phrases");
    LoadTranslations("premium_rebuilt.phrases");
    InitializeRules();
    CreateConfiguration();
    LoadFeatureAccess();
    InitializeEconomyOutbox();
    LoadShopCatalog();
    InitializeOptions();
    RegisterCommands();
    InitializeChatTags();
    AddCommandListener(LimitNativeBuildings, "build");
    HookEvent("player_builtobject", EventBuiltObject);
    HookEvent("player_dropobject", EventBuiltObject);
    HookEvent("player_spawn", EventSpawn);
    HookEvent("post_inventory_application", EventInventory);
    HookEvent("player_death", EventDeath);
    HookEvent("player_team", EventTeam);
    HookEvent("player_changeclass", EventClass);
    HookEvent("teamplay_round_start", EventRoundStart);
    HookEvent("teamplay_round_win", EventRoundEnd);
    HookEvent("teamplay_game_over", EventRoundEnd);
    AutoExecConfig(true, "premium_rebuilt");
    for (int client = 1; client <= MaxClients; client++)
        if (IsClientInGame(client)) InitializeClient(client);
}

public void OnAllPluginsLoaded()
{
    char reason[512];
    if (!PremiumCheckDependencies(reason, sizeof(reason)))
    {
        LogError("[Premium dependency warning] %s", reason);
        SetFailState("%s", reason);
        return;
    }
    g_DependencyReady = true;
    RefreshEverybody();
}

public void OnLibraryRemoved(const char[] name)
{
    if (!StrEqual(name, "tf2attributes") || g_Stopping) return;
    g_DependencyReady = false;
    LogError("[Premium dependency warning] TF2Attributes was unloaded. Premium is stopping. Reload the dependency and Premium; change map if an old attribute remains.");
    SetFailState("TF2Attributes was unloaded. Install/reload it from https://github.com/FlaminSarge/tf2attributes/releases before loading Premium.");
}

public void OnConfigsExecuted()
{
    LoadPremiumRules();
    LoadFeatureAccess();
    LoadShopCatalog();
    RestartDatabase();
    InitializeEconomyStorage();
    RefreshEverybody();
    g_NextAdvert = GetEngineTime() + cvAdInterval.FloatValue;
}

public void OnMapStart()
{
    g_PendingProfile[0] = '\0';
    StopCelebrations();
    LoadPremiumRules();
    g_RoundActive = true;
    g_NextAdvert = GetEngineTime() + 300.0;
    g_NextDatabaseRetry = 0.0;
    if (g_Heartbeat == null)
        g_Heartbeat = CreateTimer(0.25, Heartbeat, _, TIMER_REPEAT | TIMER_FLAG_NO_MAPCHANGE);
    for (int client = 1; client <= MaxClients; client++)
    {
        g_Player[client].saved = false;
        g_Player[client].nextTeleport = 0.0;
        g_Player[client].nextSwap = 0.0;
        g_NextAirblast[client] = 0.0;
        ResetMovement(client);
    }
}

public void OnMapEnd()
{
    FlushEconomyClients();
    StopCelebrations();
    g_RoundActive = false;
    g_Heartbeat = null; // NO_MAPCHANGE owns destruction.
    for (int client = 1; client <= MaxClients; client++)
    {
        StopGlow(client);
        RemoveExtraBuildings(client);
        CancelRespawn(client);
        if (ValidClient(client)) RestoreEffects(client);
        g_Player[client].saved = false;
        g_Player[client].framePending = false;
        g_Player[client].spawnPending = false;
        ResetMovement(client);
    }
}

public void OnPluginEnd()
{
    FlushEconomyClients();
    StopCelebrations();
    g_Stopping = true;
    if (g_Heartbeat != null) delete g_Heartbeat;
    for (int client = 1; client <= MaxClients; client++)
    {
        StopGlow(client);
        RemoveExtraBuildings(client);
        CancelRespawn(client);
        if (ValidClient(client)) RestoreEffects(client);
    }
    delete g_EconomyOutbox;
    delete g_UserFeatures;
    delete g_Database;
}

public void OnClientPutInServer(int client)
{
    InitializeClient(client);
}

public void OnClientPostAdminCheck(int client)
{
    if (!ValidClient(client) || IsFakeClient(client)) return;
    LoadClientRecord(client);
    LoadEconomyClient(client);
    RefreshClient(client);
    if (g_Player[client].effective && cvJoin.BoolValue && !g_Options[client].hideTag)
    {
        char tag[64];
        cvTag.GetString(tag, sizeof(tag));
        for (int recipient = 1; recipient <= MaxClients; recipient++)
            if (ValidClient(recipient) && !g_Options[recipient].quiet)
                CPrintToChat(recipient, "%s %t", tag, "Member Joined", client);
    }
    if (cvWelcome.BoolValue && g_Player[client].effective)
        Tell(client, "%t", "Welcome");
}

public void OnClientDisconnect(int client)
{
    FlushEconomyTime(client);
    ProcessEconomyOutbox();
    StopCelebration(client);
    RemoveExtraBuildings(client);
    ResetGlow(client);
    CancelRespawn(client);
    if (IsClientInGame(client)) RestoreEffects(client);
    ClearClient(client);
}

public void OnRebuildAdminCache(AdminCachePart part)
{
    RequestFrame(FrameRefreshAll);
}

public void FrameRefreshAll(any data)
{
    if (!g_Stopping) RefreshEverybody();
}

public Action Heartbeat(Handle timer)
{
    EconomyHeartbeat();
    int now = GetTime();
    for (int client = 1; client <= MaxClients; client++)
    {
        if (!ValidClient(client)) continue;
        if (g_Player[client].trialUntil > 0 && PremiumRemaining(g_Player[client].trialUntil, now) == 0)
        {
            g_Player[client].trialUntil = 0;
            if (cvTrial.BoolValue) Tell(client, "%t", "Trial Ended");
        }
        RefreshClient(client);
        RefreshGlow(client);
        MaintainWeaponPerks(client);
        MaintainExtraBuildings(client);
        TrialReminder(client, now);
        RefreshEngineerHud(client);
    }
    EnforceTeamSentryCaps();
    RefreshCelebrations();
    float clock = GetEngineTime();
    if (!g_DatabaseReady && !g_Connecting && clock >= g_NextDatabaseRetry)
        ConnectDatabase();
    if (clock >= g_NextAdvert)
    {
        g_NextAdvert = clock + cvAdInterval.FloatValue;
        if (cvEnable.BoolValue && cvAds.BoolValue)
        {
            char tag[64];
            cvTag.GetString(tag, sizeof(tag));
            for (int recipient = 1; recipient <= MaxClients; recipient++)
                if (ValidClient(recipient) && !g_Options[recipient].quiet)
                    CPrintToChat(recipient, "%s %t", tag, cvTrial.BoolValue ? "Advertisement" : "Advertisement No Trial");
        }
    }
    return Plugin_Continue;
}
