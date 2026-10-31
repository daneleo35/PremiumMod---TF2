void PublicCommand(const char[] name, ConCmd callback)
{
    char prefixed[64];
    FormatEx(prefixed, sizeof(prefixed), "sm_%s", name);
    RegConsoleCmd(prefixed, callback);
    RegConsoleCmd(name, callback); // Retain old console bindings as well as chat triggers.
}

void AdminCommand(const char[] name, ConCmd callback)
{
    char prefixed[64];
    FormatEx(prefixed, sizeof(prefixed), "sm_%s", name);
    RegAdminCmd(prefixed, callback, ADMFLAG_ROOT);
    RegAdminCmd(name, callback, ADMFLAG_ROOT);
}

void RegisterCommands()
{
    PublicCommand("premium", CommandMenu);
    PublicCommand("premium_points", CommandPoints);
    PublicCommand("premium_shop", CommandPointsShop);
    if (!CommandExists("sm_points") && !CommandExists("points")) PublicCommand("points", CommandPoints);
    if (!CommandExists("sm_shop") && !CommandExists("shop")) PublicCommand("shop", CommandPointsShop);
    AdminCommand("premium_shop_reload", CommandShopReload);
    AdminCommand("premium_points_adjust", CommandPointsAdjust);
    PublicCommand("premium_features", CommandFeatureList);
    AdminCommand("premium_feature", CommandFeatureGrant);
    AdminCommand("premium_feature_public", CommandFeaturePublic);
    AdminCommand("premium_features_reload", CommandFeatureReload);
    PublicCommand("premium_options", CommandOptions);
    PublicCommand("premium_benefits", CommandBenefits);
    PublicCommand("premium_swapstatus", CommandSwapStatus);
    AdminCommand("premium_rules_reload", CommandRulesReload);
    PublicCommand("trial", CommandTrial);
    PublicCommand("premium_status", CommandStatus);
    PublicCommand("premium_fov", CommandFov);
    PublicCommand("dark", CommandDark);
    PublicCommand("glow", CommandGlow);
    PublicCommand("colorme", CommandColor);
    PublicCommand("swapteam", CommandSwap);
    PublicCommand("save_loc", CommandSave);
    PublicCommand("load_loc", CommandLoad);
    PublicCommand("saveloc", CommandSave);
    PublicCommand("loadloc", CommandLoad);
    PublicCommand("premium_weapons", CommandWeapons);
    PublicCommand("premium_chat", CommandChatDiagnostics);
    PublicCommand("premium_flight", CommandFlight);
    AdminCommand("premium_profile", CommandProfile);
    AdminCommand("premadduser", CommandAdd);
    AdminCommand("premremuser", CommandRemove);
    AdminCommand("createtrial", CommandCreateTrial);
    AdminCommand("resettrial", CommandResetTrial);
    AdminCommand("prem_reload", CommandReload);
    AdminCommand("premium_database_create", CommandDatabase);
    AdminCommand("premium_import_legacy", CommandImport);
    AdminCommand("premium_diagnostics", CommandDiagnostics);
}

public Action CommandMenu(int client, int args)
{
    if (ValidClient(client)) DisplayFeatures(client);
    else Tell(client, "%t", "In Game Only");
    return Plugin_Handled;
}

void DisplayFeatures(int client)
{
    Menu menu = new Menu(FeatureSelected);
    char label[192];
    FormatEx(label, sizeof(label), "%T", "Menu Title", client);
    menu.SetTitle(label);
    FormatEx(label, sizeof(label), "%T", "Menu Status", client);
    menu.AddItem("status", label);
    menu.AddItem("options", "My preferences");
    menu.AddItem("features", "My feature access");
    if (cvPoints.BoolValue && cvPointsShop.BoolValue) menu.AddItem("shop", "Points shop");
    menu.AddItem("swapstatus", "Team swap availability");
    FormatEx(label, sizeof(label), "%T", "Menu Benefits", client);
    menu.AddItem("benefits", label);
    FormatEx(label, sizeof(label), "%T", "Menu Trial", client);
    bool available = cvEnable.BoolValue && cvTrial.BoolValue && g_DatabaseReady
        && g_Player[client].loaded && !g_Player[client].usedTrial && !HasPremium(client);
    menu.AddItem("trial", label, available ? ITEMDRAW_DEFAULT : ITEMDRAW_DISABLED);
    FormatEx(label, sizeof(label), "%T", "Menu Dark", client);
    menu.AddItem("dark", label, HasFeature(client, Feature_Dark) && cvDark.BoolValue ? ITEMDRAW_DEFAULT : ITEMDRAW_DISABLED);
    FormatEx(label, sizeof(label), "%T", "Menu Glow", client);
    menu.AddItem("glow", label, HasFeature(client, Feature_Glow) && cvGlow.BoolValue ? ITEMDRAW_DEFAULT : ITEMDRAW_DISABLED);
    FormatEx(label, sizeof(label), "%T", "Menu Save", client);
    menu.AddItem("save", label, GameplayFeatureAllowed(client, Feature_Teleport) && TeleportEnabled(client) ? ITEMDRAW_DEFAULT : ITEMDRAW_DISABLED);
    FormatEx(label, sizeof(label), "%T", "Menu Load", client);
    menu.AddItem("load", label, GameplayFeatureAllowed(client, Feature_Teleport) && TeleportEnabled(client) && g_Player[client].saved ? ITEMDRAW_DEFAULT : ITEMDRAW_DISABLED);
    menu.Display(client, 30);
}

public int FeatureSelected(Menu menu, MenuAction action, int client, int selection)
{
    if (action == MenuAction_End) delete menu;
    else if (action == MenuAction_Select && ValidClient(client))
    {
        char key[32];
        menu.GetItem(selection, key, sizeof(key));
        if (StrEqual(key, "options")) DisplayOptions(client);
        else if (StrEqual(key, "features")) CommandFeatureList(client, 0);
        else if (StrEqual(key, "shop")) DisplayPointsShop(client);
        else if (StrEqual(key, "swapstatus")) CommandSwapStatus(client, 0);
        else if (StrEqual(key, "trial")) CommandTrial(client, 0);
        else if (StrEqual(key, "benefits")) DisplayBenefits(client);
        else if (StrEqual(key, "dark")) CommandDark(client, 0);
        else if (StrEqual(key, "glow")) ToggleGlow(client, -1);
        else if (StrEqual(key, "save")) CommandSave(client, 0);
        else if (StrEqual(key, "load")) CommandLoad(client, 0);
        else CommandStatus(client, 0);
    }
    return 0;
}

public Action CommandStatus(int client, int args)
{
    if (!ValidClient(client))
    {
        Tell(client, "Premium %s; enabled=%d; storage=%s; ready=%d; generation=%d",
            VERSION, cvEnable.BoolValue, g_Backend, g_DatabaseReady, g_Generation);
        return Plugin_Handled;
    }
    int remaining = PremiumRemaining(g_Player[client].trialUntil, GetTime());
    Tell(client, "%t", HasPremium(client) ? "Status Active" : "Status Inactive");
    if (EconomyTrialActive(client)) Tell(client, "Purchased trial: %d minute(s) remaining.", PremiumRemaining(g_Economy[client].trialUntil, GetTime()) / 60);
    if (remaining && cvTrial.BoolValue) Tell(client, "%t", "Trial Remaining", remaining / 60, remaining % 60);
    if (HasAnyFeature(client) && !HasPremium(client)) Tell(client, "Individual features are available; use !premium_features to view your access.");
    if (!g_Player[client].loaded) Tell(client, "%t", "Storage Pending");
    return Plugin_Handled;
}

void AddBenefit(Menu menu, int client, const char[] phrase)
{
    char label[192];
    FormatEx(label, sizeof(label), "%T", phrase, client);
    menu.AddItem("info", label, ITEMDRAW_DISABLED);
}

void DisplayBenefits(int client)
{
    DisplayConfiguredBenefits(client);
}

public int BenefitsSelected(Menu menu, MenuAction action, int client, int selection)
{
    if (action == MenuAction_End) delete menu;
    else if (action == MenuAction_Cancel && selection == MenuCancel_ExitBack && ValidClient(client)) DisplayFeatures(client);
    return 0;
}

Action ToggleGlow(int client, int mode)
{
    if (!ValidClient(client))
    {
        Tell(client, "%t", "In Game Only");
        return Plugin_Handled;
    }
    bool requested = mode < 0 ? !g_Glow[client].requested : mode != 0;
    if (!requested)
    {
        // Always allow cleanup of one's own emitters, even after access changes.
        ResetGlow(client);
        Tell(client, "%t", "Glow Disabled");
        return Plugin_Handled;
    }
    if (!FeatureAccess(client, "premium_feature_glow")) return Plugin_Handled;
    if (!cvGlow.BoolValue)
    {
        Tell(client, "%t", "Feature Disabled");
        return Plugin_Handled;
    }
    RequestGlow(client, true);
    if (GlowHostVisible(client) && !RefreshGlow(client)) Tell(client, "%t", "Glow Unavailable");
    else Tell(client, "%t", "Glow Enabled");
    return Plugin_Handled;
}

public Action CommandGlow(int client, int args)
{
    if (args == 0) return ToggleGlow(client, -1);
    char argument[16];
    GetCmdArg(1, argument, sizeof(argument));
    if (args == 1 && StrEqual(argument, "on", false)) return ToggleGlow(client, 1);
    if (args == 1 && StrEqual(argument, "off", false)) return ToggleGlow(client, 0);
    Tell(client, "%t", "Glow Usage");
    return Plugin_Handled;
}

public Action CommandDiagnostics(int client, int args)
{
    int members, activeTrials, glowUsers, emitters;
    for (int target = 1; target <= MaxClients; target++)
    {
        if (ValidClient(target))
        {
            if (HasPremium(target)) members++;
            if (PremiumRemaining(g_Player[target].trialUntil, GetTime()) > 0) activeTrials++;
        }
        if (g_Glow[target].requested) glowUsers++;
        for (int slot; slot < PREMIUM_GLOW_PARTICLES; slot++)
            if (GlowHostExists(g_Glow[target].references[slot])) emitters++;
    }
    Tell(client, "Premium %s: enabled=%d dependencies=%d storage=%s ready=%d", VERSION,
        cvEnable.BoolValue, g_DependencyReady, g_Backend, g_DatabaseReady);
    Tell(client, "Online premium=%d active trials=%d; glow requested=%d owned emitters=%d; entities=%d/%d",
        members, activeTrials, glowUsers, emitters, GetEntityCount(), GetMaxEntities());
    return Plugin_Handled;
}

public Action CommandWeapons(int client, int args)
{
    if (!ValidClient(client)) { Tell(client, "%t", "In Game Only"); return Plugin_Handled; }
    Tell(client, "Ammo report: premium=%d gameplay=%d reserve multiplier=%.2f; magazines are native",
        HasPremium(client), GameplayAllowed(client), EffectiveAmmoMultiplier(client));
    for (int pool; pool < 2; pool++)
    {
        float multiplier = PremiumAttributeNativesAvailable() && g_DependencyReady
            ? TF2Attrib_HookValueFloat(1.0, g_AmmoHooks[pool], client) : -1.0;
        Tell(client, "pool=%d reserve=%d capacity hook=%.3f owned=%d", pool + 1,
            GetEntProp(client, Prop_Send, "m_iAmmo", _, pool + 1), multiplier, g_AmmoCapacity[client][pool].owned);
    }
    for (int slot; slot < 3; slot++)
    {
        int weapon = GetPlayerWeaponSlot(client, slot);
        if (!ValidWeapon(weapon)) continue;
        char classname[64]; GetEntityClassname(weapon, classname, sizeof(classname));
        int definition = GetEntProp(weapon, Prop_Send, "m_iItemDefinitionIndex");
        int ammo = HasEntProp(weapon, Prop_Send, "m_iPrimaryAmmoType") ? GetEntProp(weapon, Prop_Send, "m_iPrimaryAmmoType") : -1;
        int pool = PremiumReserveAmmoIndex(ammo, GetEntPropArraySize(client, Prop_Send, "m_iAmmo"));
        int reserve = pool >= 0 ? GetEntProp(client, Prop_Send, "m_iAmmo", _, ammo) : -1;
        Tell(client, "slot=%d item=%d %s ammo pool=%d reserve=%d normal reserve perk=%d",
            slot, definition, classname, ammo, reserve, pool >= 0);
    }
    return Plugin_Handled;
}

public Action CommandTrial(int client, int args)
{
    if (!ValidClient(client) || IsFakeClient(client))
    {
        Tell(client, "%t", "In Game Only");
        return Plugin_Handled;
    }
    if (HasPremium(client)) return CommandStatus(client, args);
    StartTrial(client);
    return Plugin_Handled;
}

public Action CommandFov(int client, int args)
{
    if (!FeatureAccess(client, "premium_feature_fov")) return Plugin_Handled;
    if (!cvFov.BoolValue)
    {
        Tell(client, "%t", "Feature Disabled");
        return Plugin_Handled;
    }
    char argument[32];
    GetCmdArg(1, argument, sizeof(argument));
    int fov = StringToInt(argument);
    if (args != 1 || (!StrEqual(argument, "reset", false) && (strlen(argument) > 3 || !PremiumDigits(argument) || fov < 75 || fov > 120)))
    {
        Tell(client, "%t", "FOV Usage");
        return Plugin_Handled;
    }
    g_Player[client].fov = StrEqual(argument, "reset", false) ? 0 : fov;
    QueueEffects(client);
    Tell(client, "%t", "FOV Updated", g_Player[client].fov ? g_Player[client].fov : cvDefaultFov.IntValue);
    return Plugin_Handled;
}

public Action CommandDark(int client, int args)
{
    if (!FeatureAccess(client, "premium_feature_dark")) return Plugin_Handled;
    if (!cvDark.BoolValue)
    {
        Tell(client, "%t", "Feature Disabled");
        return Plugin_Handled;
    }
    g_Player[client].dark = !g_Player[client].dark;
    QueueEffects(client);
    Tell(client, "%t", g_Player[client].dark ? "Dark Enabled" : "Dark Disabled");
    return Plugin_Handled;
}

public Action CommandColor(int client, int args)
{
    if (!FeatureAccess(client, "premium_feature_color")) return Plugin_Handled;
    if (cvColor.IntValue == 0)
    {
        Tell(client, "%t", "Feature Disabled");
        return Plugin_Handled;
    }
    char argument[32];
    GetCmdArg(1, argument, sizeof(argument));
    if (args == 1 && StrEqual(argument, "reset", false)) g_Player[client].customColor = false;
    else
    {
        int rgb[3];
        for (int i; i < 3; i++)
        {
            GetCmdArg(i + 1, argument, sizeof(argument));
            if (args != 3 || strlen(argument) > 3 || !PremiumDigits(argument) || StringToInt(argument) > 255)
            {
                Tell(client, "%t", "Color Usage");
                return Plugin_Handled;
            }
            rgb[i] = StringToInt(argument);
        }
        for (int i; i < 3; i++) g_Player[client].rgb[i] = rgb[i];
        g_Player[client].customColor = true;
        g_Player[client].dark = false;
    }
    QueueEffects(client);
    Tell(client, "%t", "Color Updated");
    return Plugin_Handled;
}

public Action CommandSwap(int client, int args)
{
    char reason[192];
    if (!SwapAvailability(client, reason, sizeof(reason)))
    { Tell(client, "%s", reason); return Plugin_Handled; }
    int team = GetClientTeam(client);
    CancelRespawn(client);
    g_Player[client].saved = false;
    g_Player[client].nextSwap = GetGameTime() + cvSwapCooldown.FloatValue;
    ChangeClientTeam(client, team == 2 ? 3 : 2);
    Tell(client, "%t", "Team Changed");
    return Plugin_Handled;
}

bool TeleportAllowed(int client)
{
    if (!GameplayCommandAccess(client, "premium_feature_teleport")) return false;
    if (!TeleportEnabled(client))
    {
        Tell(client, "%t", "Feature Disabled");
        return false;
    }
    if (!IsPlayerAlive(client) || GetClientTeam(client) < 2 || !g_RoundActive
        || GetEntityMoveType(client) != MOVETYPE_WALK
        || TF2_IsPlayerInCondition(client, TFCond_Taunting)
        || TF2_IsPlayerInCondition(client, TFCond_Dazed)
        || TF2_IsPlayerInCondition(client, TFCond_Charging)
        || (HasEntProp(client, Prop_Send, "m_hItem") && GetEntPropEnt(client, Prop_Send, "m_hItem") != -1))
    {
        Tell(client, "%t", "Teleport Unavailable");
        return false;
    }
    return true;
}

bool TeleportDestinationClear(int client, const float position[3])
{
    float minimum[3], maximum[3];
    GetEntPropVector(client, Prop_Send, "m_vecMins", minimum);
    GetEntPropVector(client, Prop_Send, "m_vecMaxs", maximum);
    float scale = GetEntPropFloat(client, Prop_Send, "m_flModelScale");
    if (maximum[2] < 82.0 * scale) maximum[2] = 82.0 * scale;
    Handle trace = TR_TraceHullFilterEx(position, position, minimum, maximum, MASK_PLAYERSOLID, TeleportTraceFilter, client);
    bool clear = !TR_StartSolid(trace) && !TR_AllSolid(trace) && !TR_DidHit(trace);
    delete trace;
    return clear;
}

public bool TeleportTraceFilter(int entity, int contentsMask, any client)
{
    return entity != client;
}

public Action CommandSave(int client, int args)
{
    if (!TeleportAllowed(client)) return Plugin_Handled;
    if (!(GetEntityFlags(client) & FL_ONGROUND) || (GetClientButtons(client) & IN_DUCK))
    {
        Tell(client, "%t", "Save Ground Only");
        return Plugin_Handled;
    }
    float position[3];
    GetClientAbsOrigin(client, position);
    position[2] += 1.0;
    if (!TeleportDestinationClear(client, position))
    {
        Tell(client, "%t", "Teleport Blocked");
        return Plugin_Handled;
    }
    g_Player[client].location = position;
    GetClientEyeAngles(client, g_Player[client].angles);
    g_Player[client].savedTeam = GetClientTeam(client);
    g_Player[client].saved = true;
    Tell(client, "%t", "Location Saved");
    return Plugin_Handled;
}

public Action CommandLoad(int client, int args)
{
    if (!TeleportAllowed(client)) return Plugin_Handled;
    if (!g_Player[client].saved || g_Player[client].savedTeam != GetClientTeam(client))
    {
        Tell(client, "%t", "No Location");
        return Plugin_Handled;
    }
    float remaining = g_Player[client].nextTeleport - GetGameTime();
    if (remaining > 0.0)
    {
        Tell(client, "%t", "Teleport Cooldown", RoundToCeil(remaining));
        return Plugin_Handled;
    }
    if (!TeleportDestinationClear(client, g_Player[client].location))
    {
        Tell(client, "%t", "Teleport Blocked");
        return Plugin_Handled;
    }
    float stopped[3];
    TeleportEntity(client, g_Player[client].location, g_Player[client].angles, stopped);
    g_Player[client].nextTeleport = GetGameTime() + cvTeleportCooldown.FloatValue;
    ResetMovement(client);
    Tell(client, "%t", "Location Loaded");
    return Plugin_Handled;
}

Action MembershipCommand(int client, int args, StorageOperation operation)
{
    if (args != 1)
    {
        Tell(client, "%t", "Membership Usage");
        return Plugin_Handled;
    }
    char argument[128], steam[STEAM_LENGTH];
    GetCmdArg(1, argument, sizeof(argument));
    if (PremiumNormalizeSteam(argument, steam, sizeof(steam)))
    {
        WriteMembership(client, steam, operation);
        return Plugin_Handled;
    }
    int targets[MAXPLAYERS], count;
    char targetName[MAX_TARGET_LENGTH];
    bool multilingual;
    count = ProcessTargetString(argument, client, targets, sizeof(targets), COMMAND_FILTER_CONNECTED | COMMAND_FILTER_NO_BOTS,
        targetName, sizeof(targetName), multilingual);
    if (count <= 0)
    {
        ReplyToTargetError(client, count);
        return Plugin_Handled;
    }
    for (int i; i < count; i++)
    {
        char auth[STEAM_LENGTH];
        if (!GetClientAuthId(targets[i], AuthId_Steam3, auth, sizeof(auth))
            || !PremiumNormalizeSteam(auth, steam, sizeof(steam))) continue;
        WriteMembership(client, steam, operation);
    }
    return Plugin_Handled;
}

public Action CommandAdd(int client, int args) { return MembershipCommand(client, args, Storage_Add); }
public Action CommandRemove(int client, int args) { return MembershipCommand(client, args, Storage_Remove); }
public Action CommandCreateTrial(int client, int args) { return MembershipCommand(client, args, Storage_GrantTrial); }
public Action CommandResetTrial(int client, int args) { return MembershipCommand(client, args, Storage_ResetTrial); }

public Action CommandReload(int client, int args)
{
    RefreshEverybody();
    for (int target = 1; target <= MaxClients; target++)
    {
        if (!ValidClient(target)) continue;
        QueueEffects(target);
        LoadClientRecord(target);
    }
    Tell(client, "%t", "Settings Refreshed");
    return Plugin_Handled;
}

public Action CommandDatabase(int client, int args)
{
    RestartDatabase(true);
    Tell(client, "%t", "Database Connecting");
    return Plugin_Handled;
}

public Action CommandImport(int client, int args)
{
    char argument[16];
    GetCmdArg(1, argument, sizeof(argument));
    if (args > 1 || (args == 1 && (!PremiumDigits(argument) || strlen(argument) > 7)))
    {
        Tell(client, "Usage: premium_import_legacy [offset 0-9999999]");
        return Plugin_Handled;
    }
    ImportLegacy(client, args ? StringToInt(argument) : 0);
    return Plugin_Handled;
}

public Action CommandProfile(int client, int args)
{
    char name[24], when[16];
    if (args >= 1) GetCmdArg(1, name, sizeof(name));
    if (args == 1 && StrEqual(name, "cancel"))
    { g_PendingProfile[0] = '\0'; Tell(client, "Pending Premium profile cancelled."); return Plugin_Handled; }
    if (args == 1 && StrEqual(name, "status"))
    { Tell(client, "Pending next-round profile: %s", g_PendingProfile[0] ? g_PendingProfile : "none"); return Plugin_Handled; }
    if (args == 2) GetCmdArg(2, when, sizeof(when)); else strcopy(when, sizeof(when), "next");
    if (args < 1 || args > 2 || !(StrEqual(name, "cosmetic") || StrEqual(name, "balanced") || StrEqual(name, "fun"))
        || !(StrEqual(when, "next") || StrEqual(when, "now")))
    { Tell(client, "Usage: sm_premium_profile <cosmetic|balanced|fun> [next|now], or status/cancel. Default: next round."); return Plugin_Handled; }
    char path[PLATFORM_MAX_PATH]; FormatEx(path, sizeof(path), "cfg/sourcemod/premium_profiles/%s.cfg", name);
    if (!FileExists(path)) { Tell(client, "Missing profile file: %s", path); return Plugin_Handled; }
    strcopy(g_PendingProfile, sizeof(g_PendingProfile), name);
    if (StrEqual(when, "now")) { ApplyPendingProfile(); Tell(client, "Applied Premium profile: %s", name); }
    else Tell(client, "Premium profile '%s' will apply at the next round start. status/cancel are available.", name);
    return Plugin_Handled;
}

public Action CommandFlight(int client, int args)
{
    if (!ValidClient(client)) { Tell(client, "%t", "In Game Only"); return Plugin_Handled; }
    int weapon = GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon");
    char classname[64] = "none";
    int definition = -1, state = -1, ammo = -1;
    bool stream;
    if (ValidWeapon(weapon))
    {
        GetEntityClassname(weapon, classname, sizeof(classname));
        definition = GetEntProp(weapon, Prop_Send, "m_iItemDefinitionIndex");
        if (HasEntProp(weapon, Prop_Send, "m_iWeaponState")) state = GetEntProp(weapon, Prop_Send, "m_iWeaponState");
        int pool = GetEntProp(weapon, Prop_Send, "m_iPrimaryAmmoType");
        if (pool >= 0 && pool < GetEntPropArraySize(client, Prop_Send, "m_iAmmo")) ammo = GetEntProp(client, Prop_Send, "m_iAmmo", _, pool);
        stream = FlightStreamActive(client, weapon);
    }
    float angles[3], velocity[3];
    GetClientEyeAngles(client, angles);
    GetEntPropVector(client, Prop_Data, "m_vecVelocity", velocity);
    PrintToConsole(client, "Premium %s flight: enabled=%d premium=%d gameplay=%d alive=%d class=%d ground=%d blocked=%d",
        VERSION, FlightEnabled(client), HasPremium(client), GameplayAllowed(client), IsPlayerAlive(client),
        TF2_GetPlayerClass(client), (GetEntityFlags(client) & FL_ONGROUND) != 0, MovementBlocked(client));
    PrintToConsole(client, "weapon=%s item=%d state=%d ammo=%d firing=%d buttons=%d pitch=%.1f velocity=%.1f/%.1f/%.1f speed=%.0f lift=%.0f acceleration=%.0f",
        classname, definition, state, ammo, stream, GetClientButtons(client), angles[0], velocity[0], velocity[1], velocity[2],
        FlightSpeed(client), FlightLift(client), FlightAcceleration(client));
    PrintToConsole(client, "airblast jump: enabled=%d force=%.0f lift=%.0f cap=%.0f cooldown=%.2f next=%.2f",
        AirblastEnabled(client), cvAirblastForce.FloatValue, cvAirblastLift.FloatValue, cvAirblastCap.FloatValue,
        cvAirblastCooldown.FloatValue, g_NextAirblast[client]);
    Tell(client, "Flight diagnostics printed in your console. Jump and hold primary fire; aim down to climb, level to hover, WASD to steer.");
    return Plugin_Handled;
}
