// Extra dispensers and teleporter ends stay in TF2's normal ownership list.
// Never exceed its six networked objects. Teleporter modes remain 0/1 so
// native matching, recharge, upgrades and transport remain engine-owned.
int g_ExtraUtilities[MAXPLAYERS + 1][3][3];
int g_UtilityOrder[MAXPLAYERS + 1][3][3];
int g_UtilitySequence;
int g_MaskedUtilities[MAXPLAYERS + 1][6];
int g_UtilityTransmitFlags[MAXPLAYERS + 1][6];
float g_NextBuildingNotice[MAXPLAYERS + 1];
float g_UtilityMaskResume[MAXPLAYERS + 1];

#define UTILITY_TRANSMIT_FLAGS (FL_EDICT_ALWAYS | FL_EDICT_DONTSEND | FL_EDICT_PVSCHECK)

public Action UtilityOwnerTransmit(int entity, int viewer)
{
    if (!ValidClient(viewer) || !IsPlayerAlive(viewer) || !g_RoundActive
        || !IsValidEntity(entity) || OwnedUtilityKind(viewer, EntIndexToEntRef(entity)) < 0) return Plugin_Continue;
    int reference = EntIndexToEntRef(entity);
    bool masked;
    for (int slot; slot < 6; slot++) if (g_MaskedUtilities[viewer][slot] == reference) { masked = true; break; }
    if (!masked || !GetEntProp(entity, Prop_Send, "m_bDisposableBuilding")) return Plugin_Continue;
    int weapon = GetEntPropEnt(viewer, Prop_Send, "m_hActiveWeapon");
    if (!ValidWeapon(weapon)) return Plugin_Continue;
    char classname[64]; GetEntityClassname(weapon, classname, sizeof(classname));
    // C_TFPlayer::GetObjectOfType ignores disposable flags but skips dormant
    // entities. Only the owner's build-PDA selection needs this client mask.
    // Keep real types/modes/ownership; show everything again during placement.
    return StrEqual(classname, "tf_weapon_pda_engineer_build") ? Plugin_Handled : Plugin_Continue;
}

void ProtectUtilityWrench(int attacker, const char[] classname)
{
    if (!ValidClient(attacker) || (!StrEqual(classname, "tf_weapon_wrench") && !StrEqual(classname, "tf_weapon_robot_arm"))) return;
    // The disposable flag also gates native wrench upgrades. Drop our masks
    // before a teammate's swing, through the delayed melee impact. The PDA
    // may briefly show Already Built while the wrench interaction completes.
    for (int owner = 1; owner <= MaxClients; owner++)
    {
        if (!ValidClient(owner) || GetClientTeam(owner) != GetClientTeam(attacker)) continue;
        // Keep unrelated Engineers' PDAs usable. The margin covers a native
        // wrench swing and movement until its delayed impact, not just its ray.
        float attackerOrigin[3], origin[3]; GetClientAbsOrigin(attacker, attackerOrigin);
        bool nearby;
        char classes[][] = {"obj_dispenser", "obj_teleporter"};
        for (int type; type < sizeof(classes) && !nearby; type++)
        {
            int entity = -1;
            while ((entity = FindEntityByClassname(entity, classes[type])) != -1)
            {
                if (GetEntPropEnt(entity, Prop_Send, "m_hBuilder") != owner) continue;
                GetEntPropVector(entity, Prop_Send, "m_vecOrigin", origin);
                if (GetVectorDistance(attackerOrigin, origin) <= 256.0) { nearby = true; break; }
            }
        }
        if (!nearby) continue;
        RestoreUtilityMasks(owner);
        g_UtilityMaskResume[owner] = GetGameTime() + 0.5;
    }
}

int UtilityKind(int type, int mode)
{
    if (type == view_as<int>(TFObject_Dispenser) && mode == 0) return 0;
    if (type == view_as<int>(TFObject_Teleporter) && (mode == 0 || mode == 1)) return mode + 1;
    return -1;
}

int OwnedUtilityKind(int client, int reference)
{
    int entity = EntRefToEntIndex(reference);
    if (entity <= MaxClients || !IsValidEntity(entity)) return -1;
    // Aim traces can hit any entity. Reject map props, ammo packs and other
    // non-buildings before reading building-only network properties.
    char classname[32]; GetEntityClassname(entity, classname, sizeof(classname));
    if ((!StrEqual(classname, "obj_dispenser") && !StrEqual(classname, "obj_teleporter"))
        || !HasEntProp(entity, Prop_Send, "m_hBuilder")
        || GetEntPropEnt(entity, Prop_Send, "m_hBuilder") != client) return -1;
    if (StrEqual(classname, "obj_dispenser")) return 0;
    if (HasEntProp(entity, Prop_Send, "m_iObjectMode"))
        return UtilityKind(view_as<int>(TFObject_Teleporter), GetEntProp(entity, Prop_Send, "m_iObjectMode"));
    return -1;
}

bool PlacedUtility(int client, int entity, int kind)
{
    return kind >= 0 && kind < 3 && OwnedUtilityKind(client, EntIndexToEntRef(entity)) == kind
        && (!HasEntProp(entity, Prop_Send, "m_bPlacing") || !GetEntProp(entity, Prop_Send, "m_bPlacing"));
}

int CountUtilities(int client, int kind)
{
    int count, entity = -1;
    while ((entity = FindEntityByClassname(entity, kind == 0 ? "obj_dispenser" : "obj_teleporter")) != -1)
        if (PlacedUtility(client, entity, kind)) count++;
    return count;
}

int UtilityLimit(int client, int kind)
{
    if (!GameplayFeatureAllowed(client, kind == 0 ? Feature_Dispenser : Feature_Teleporter) || TF2_GetPlayerClass(client) != TFClass_Engineer
        || GameRules_GetProp("m_bPlayingMannVsMachine") || GameRules_GetProp("m_nGameType") == 4
        || !CheckCommandAccess(client, kind == 0 ? "premium_feature_dispenser" : "premium_feature_teleporter", 0, true)) return 1;
    return kind == 0 ? cvDispenserLimit.IntValue : (kind == 1 ? cvEntranceLimit.IntValue : cvExitLimit.IntValue);
}

bool IsBonusUtility(int client, int kind, int reference)
{
    for (int slot; slot < 3; slot++) if (g_ExtraUtilities[client][kind][slot] == reference) return true;
    return false;
}

int TeamUtilityCap(int kind)
{
    return kind == 0 ? cvTeamDispensers.IntValue : (kind == 1 ? cvTeamEntrances.IntValue : cvTeamExits.IntValue);
}

int CountTeamExtraUtilities(int team, int kind)
{
    int count;
    for (int client = 1; client <= MaxClients; client++)
    {
        if (!ValidClient(client) || GetClientTeam(client) != team) continue;
        for (int slot; slot < 3; slot++) if (OwnedUtilityKind(client, g_ExtraUtilities[client][kind][slot]) == kind) count++;
    }
    return count;
}

bool TeamUtilityRoom(int client, int kind)
{
    int cap = TeamUtilityCap(kind);
    return cap < 0 || CountTeamExtraUtilities(GetClientTeam(client), kind) < cap;
}

void EnforceTeamUtilityCaps()
{
    for (int team = 2; team <= 3; team++) for (int kind; kind < 3; kind++)
    {
        int cap = TeamUtilityCap(kind);
        if (cap < 0) continue;
        int count = CountTeamExtraUtilities(team, kind);
        while (count > cap)
        {
            int owner, chosen, newest = -1;
            for (int client = 1; client <= MaxClients; client++)
            {
                if (!ValidClient(client) || GetClientTeam(client) != team) continue;
                for (int slot; slot < 3; slot++)
                    if (OwnedUtilityKind(client, g_ExtraUtilities[client][kind][slot]) == kind && g_UtilityOrder[client][kind][slot] > newest)
                    { owner = client; chosen = slot; newest = g_UtilityOrder[client][kind][slot]; }
            }
            if (!owner) break;
            int reference = g_ExtraUtilities[owner][kind][chosen];
            g_ExtraUtilities[owner][kind][chosen] = INVALID_ENT_REFERENCE;
            RestoreUtilityMasks(owner); RemoveEntity(EntRefToEntIndex(reference)); count--;
        }
    }
}

void RestoreUtilityMasks(int client)
{
    for (int slot; slot < 6; slot++)
    {
        int reference = g_MaskedUtilities[client][slot];
        int savedFlags = g_UtilityTransmitFlags[client][slot];
        g_MaskedUtilities[client][slot] = INVALID_ENT_REFERENCE;
        g_UtilityTransmitFlags[client][slot] = 0;
        int entity = EntRefToEntIndex(reference);
        if (entity <= MaxClients || !IsValidEntity(entity)) continue;
        SDKUnhook(entity, SDKHook_SetTransmit, UtilityOwnerTransmit);
        int flags = GetEdictFlags(entity);
        // Do not overwrite a later transmit policy installed by another plugin.
        if (!(flags & UTILITY_TRANSMIT_FLAGS)) SetEdictFlags(entity, (flags & ~UTILITY_TRANSMIT_FLAGS) | savedFlags);
        if (OwnedUtilityKind(client, reference) < 0) continue;
        if (GetEntProp(entity, Prop_Send, "m_bDisposableBuilding"))
            SetEntProp(entity, Prop_Send, "m_bDisposableBuilding", 0);
    }
}

void UpdateUtilityPda(int client, int weapon = -1)
{
    RestoreUtilityMasks(client);
    if (!GameplayAllowed(client) || !IsPlayerAlive(client) || !g_RoundActive
        || TF2_GetPlayerClass(client) != TFClass_Engineer || GetGameTime() < g_UtilityMaskResume[client]) return;
    if (weapon == -1) weapon = GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon");
    if (!ValidWeapon(weapon)) return;
    char classname[64]; GetEntityClassname(weapon, classname, sizeof(classname));
    if (!StrEqual(classname, "tf_weapon_pda_engineer_build") && !StrEqual(classname, "tf_weapon_builder")) return;
    bool carrying = HasEntProp(client, Prop_Send, "m_bCarryingObject") && GetEntProp(client, Prop_Send, "m_bCarryingObject") != 0;
    if (!carrying && (CountOwnedBuildings(client, false) >= 6 || GetEntityCount() >= GetMaxEntities() - 32)) return;
    int slot;
    for (int kind; kind < 3; kind++)
    {
        if (UtilityLimit(client, kind) <= 1) continue;
        int carried = -1, entity = -1;
        if (carrying)
        {
            while ((entity = FindEntityByClassname(entity, kind == 0 ? "obj_dispenser" : "obj_teleporter")) != -1)
                if (PlacedUtility(client, entity, kind) && GetEntProp(entity, Prop_Send, "m_bCarried")) { carried = entity; break; }
            if (carried == -1) continue;
        }
        else if (CountUtilities(client, kind) >= UtilityLimit(client, kind) || !TeamUtilityRoom(client, kind)) continue;
        entity = -1;
        while ((entity = FindEntityByClassname(entity, kind == 0 ? "obj_dispenser" : "obj_teleporter")) != -1 && slot < 6)
        {
            if (!PlacedUtility(client, entity, kind) || entity == carried
                || !HasEntProp(entity, Prop_Send, "m_bDisposableBuilding")
                || GetEntProp(entity, Prop_Send, "m_bDisposableBuilding")) continue;
            int flags = GetEdictFlags(entity);
            // Respect objects deliberately hidden by another plugin.
            if (flags & FL_EDICT_DONTSEND) continue;
            if (!SDKHookEx(entity, SDKHook_SetTransmit, UtilityOwnerTransmit))
            {
                if (GetGameTime() >= g_NextBuildingNotice[client])
                { g_NextBuildingNotice[client] = GetGameTime() + 5.0; Tell(client, "Extra utility building menu could not initialize its SDKHooks transmit hook. Check the server log."); LogError("Premium: SetTransmit hook unavailable for utility entity %d", entity); }
                continue;
            }
            // Server counts skip disposable objects; the client does not.
            // Force the normal ShouldTransmit path so SDKHooks can make these
            // dormant for their owner only while the build selection is open.
            g_MaskedUtilities[client][slot] = EntIndexToEntRef(entity);
            g_UtilityTransmitFlags[client][slot++] = flags & UTILITY_TRANSMIT_FLAGS;
            SetEdictFlags(entity, flags & ~UTILITY_TRANSMIT_FLAGS);
            SetEntProp(entity, Prop_Send, "m_bDisposableBuilding", 1);
        }
    }
}

void BuildingLimitNotice(int client, int kind, bool total)
{
    if (GetGameTime() < g_NextBuildingNotice[client]) return;
    g_NextBuildingNotice[client] = GetGameTime() + 2.0;
    if (total) Tell(client, "%t", "Building Slots Full");
    else if (kind >= 0 && CountUtilities(client, kind) > 0 && !TeamUtilityRoom(client, kind)) Tell(client, "Your team's extra %s limit is full (%d). Ordinary buildings are still allowed.", kind == 0 ? "dispenser" : (kind == 1 ? "entrance" : "exit"), TeamUtilityCap(kind));
    else Tell(client, "%t", "Utility Building Limit", kind == 0 ? "dispenser" : (kind == 1 ? "teleporter entrance" : "teleporter exit"), UtilityLimit(client, kind));
}

bool BuildingPlacementBlocked(int client)
{
    if (TF2_GetPlayerClass(client) != TFClass_Engineer
        || (HasEntProp(client, Prop_Send, "m_bCarryingObject") && GetEntProp(client, Prop_Send, "m_bCarryingObject"))) return false;
    int weapon = GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon");
    if (!ValidWeapon(weapon)) return false;
    char classname[64]; GetEntityClassname(weapon, classname, sizeof(classname));
    if (!StrEqual(classname, "tf_weapon_builder") || !HasEntProp(weapon, Prop_Send, "m_iObjectType")) return false;
    int type = GetEntProp(weapon, Prop_Send, "m_iObjectType");
    int mode = HasEntProp(weapon, Prop_Send, "m_iObjectMode") ? GetEntProp(weapon, Prop_Send, "m_iObjectMode") : 0;
    int kind = UtilityKind(type, mode);
    if (type != view_as<int>(TFObject_Sentry) && kind < 0) return false;
    bool total = CountOwnedBuildings(client, false) >= 6 || GetEntityCount() >= GetMaxEntities() - 32;
    bool limit = kind >= 0 ? CountUtilities(client, kind) >= UtilityLimit(client, kind)
        : CountOwnedBuildings(client) >= (ExtraSentryAllowed(client) ? cvSentryLimit.IntValue : 1);
    if (kind >= 0 && CountUtilities(client, kind) > 0 && !TeamUtilityRoom(client, kind)) limit = true;
    if (total || limit)
    {
        RestoreBuildingPda(client);
        if (kind >= 0 || total) BuildingLimitNotice(client, kind, total);
        return true;
    }
    return SentryPlacementBlocked(client);
}

public Action LimitNativeBuildings(int client, const char[] command, int args)
{
    if (!ValidClient(client) || (args != 1 && args != 2)) return Plugin_Continue;
    char argument[16]; GetCmdArg(1, argument, sizeof(argument));
    if (!PremiumDigits(argument)) return Plugin_Continue;
    int type = StringToInt(argument), mode;
    if (args == 1 && type == 3) { type = 1; mode = 1; } // TF2's legacy exit bind.
    else if (args == 2)
    {
        GetCmdArg(2, argument, sizeof(argument));
        if (!PremiumDigits(argument)) return Plugin_Continue;
        mode = StringToInt(argument);
    }
    if (type == view_as<int>(TFObject_Sentry)) return LimitNativeSentries(client, command, args);
    int kind = UtilityKind(type, mode);
    if (kind < 0) return Plugin_Continue;
    if (HasEntProp(client, Prop_Send, "m_bCarryingObject") && GetEntProp(client, Prop_Send, "m_bCarryingObject"))
    { UpdateUtilityPda(client, GetPlayerWeaponSlot(client, 3)); return Plugin_Continue; }
    bool total = CountOwnedBuildings(client, false) >= 6 || GetEntityCount() >= GetMaxEntities() - 32;
    if (total || CountUtilities(client, kind) >= UtilityLimit(client, kind)
        || (CountUtilities(client, kind) > 0 && !TeamUtilityRoom(client, kind)))
    {
        RestoreBuildingPda(client); BuildingLimitNotice(client, kind, total);
        return Plugin_Handled;
    }
    UpdateUtilityPda(client, GetPlayerWeaponSlot(client, 3));
    return Plugin_Continue;
}

void TrackPlacedUtility(int client, int entity)
{
    if (entity <= MaxClients || !IsValidEntity(entity)) return;
    int reference = EntIndexToEntRef(entity), kind = OwnedUtilityKind(client, reference);
    if (kind < 0 || !PlacedUtility(client, entity, kind) || IsBonusUtility(client, kind, reference)) return;
    int other = -1;
    bool primary;
    while ((other = FindEntityByClassname(other, kind == 0 ? "obj_dispenser" : "obj_teleporter")) != -1)
        if (other != entity && PlacedUtility(client, other, kind) && !IsBonusUtility(client, kind, EntIndexToEntRef(other))) { primary = true; break; }
    if (CountOwnedBuildings(client, false) > 6 || CountUtilities(client, kind) > UtilityLimit(client, kind))
    { RemoveEntity(entity); return; }
    if (!primary) return;
    if (!TeamUtilityRoom(client, kind)) { RemoveEntity(entity); BuildingLimitNotice(client, kind, false); return; }
    for (int slot; slot < 3; slot++)
    {
        if (OwnedUtilityKind(client, g_ExtraUtilities[client][kind][slot]) == kind) continue;
        g_ExtraUtilities[client][kind][slot] = reference;
        g_UtilityOrder[client][kind][slot] = ++g_UtilitySequence;
        return;
    }
}

void RemoveExtraUtilities(int client)
{
    RestoreUtilityMasks(client);
    g_NextBuildingNotice[client] = 0.0;
    g_UtilityMaskResume[client] = 0.0;
    for (int kind; kind < 3; kind++) for (int slot; slot < 3; slot++)
    {
        int reference = g_ExtraUtilities[client][kind][slot];
        g_ExtraUtilities[client][kind][slot] = INVALID_ENT_REFERENCE;
        if (OwnedUtilityKind(client, reference) == kind) RemoveEntity(EntRefToEntIndex(reference));
    }
}

void MaintainUtilityBuildings(int client)
{
    RestoreUtilityMasks(client);
    for (int kind; kind < 3; kind++)
    {
        int count = CountUtilities(client, kind), cap = UtilityLimit(client, kind);
        while (count > cap)
        {
            int chosen = -1, newest = -1;
            for (int slot; slot < 3; slot++)
                if (OwnedUtilityKind(client, g_ExtraUtilities[client][kind][slot]) == kind && g_UtilityOrder[client][kind][slot] > newest)
                { chosen = slot; newest = g_UtilityOrder[client][kind][slot]; }
            if (chosen < 0) break;
            int reference = g_ExtraUtilities[client][kind][chosen];
            g_ExtraUtilities[client][kind][chosen] = INVALID_ENT_REFERENCE;
            RemoveEntity(EntRefToEntIndex(reference)); count--;
        }
    }
    UpdateUtilityPda(client);
}

void RestoreBuildingPda(int client) { RestoreSentryModes(client); RestoreUtilityMasks(client); }
void RemoveExtraBuildings(int client) { RemoveExtraSentries(client); RemoveExtraUtilities(client); }
void MaintainExtraBuildings(int client) { MaintainExtraSentries(client); MaintainUtilityBuildings(client); }
