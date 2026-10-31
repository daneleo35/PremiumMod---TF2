// Normal PDA/blueprint construction. While selecting/placing an extra sentry,
// existing sentries temporarily use object mode 1 so client AND server native
// mode-0 ownership checks permit another blueprint. Their real object type,
// builder/team registration and disposable/mini flags are never changed.
// A carried sentry stays in mode 0 so native redeployment can find it at cap.
// Restore modes on weapon exit, cap/access changes and every cleanup.
int g_ExtraSentries[MAXPLAYERS + 1][3];
int g_MaskedSentries[MAXPLAYERS + 1][3];
int g_BonusOrder[MAXPLAYERS + 1][3];
int g_BonusSequence;
float g_NextSentryNotice[MAXPLAYERS + 1];

bool SentryPlacementBlocked(int client)
{
    if (TF2_GetPlayerClass(client) != TFClass_Engineer
        || (HasEntProp(client, Prop_Send, "m_bCarryingObject") && GetEntProp(client, Prop_Send, "m_bCarryingObject"))) return false;
    int weapon = GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon");
    if (!ValidWeapon(weapon)) return false;
    char classname[64]; GetEntityClassname(weapon, classname, sizeof(classname));
    if (!StrEqual(classname, "tf_weapon_builder")) return false;
    if (!HasEntProp(weapon, Prop_Send, "m_iObjectType")
        || GetEntProp(weapon, Prop_Send, "m_iObjectType") != view_as<int>(TFObject_Sentry)
        || !HasPrimarySentry(client) || TeamExtraRoom(client)) return false;
    // Other players may consume the last team slot while this blueprint is
    // open. Block placement input before native metal spending, not afterwards.
    RestoreSentryModes(client);
    if (GetGameTime() >= g_NextSentryNotice[client])
    { Tell(client, "%t", "Team Sentry Limit", cvTeamSentries.IntValue); g_NextSentryNotice[client] = GetGameTime() + 2.0; }
    return true;
}

int CountTeamExtraSentries(int team)
{
    int count;
    for (int client = 1; client <= MaxClients; client++)
    {
        if (!ValidClient(client) || GetClientTeam(client) != team) continue;
        for (int slot; slot < 3; slot++) if (OwnedExtraSentry(client, g_ExtraSentries[client][slot])) count++;
    }
    return count;
}

bool TeamExtraRoom(int client)
{
    return PremiumTeamExtraRoom(CountTeamExtraSentries(GetClientTeam(client)), cvTeamSentries.IntValue);
}

void EnforceTeamSentryCaps()
{
    for (int team = 2; team <= 3; team++)
    {
        int count = CountTeamExtraSentries(team);
        while (count > cvTeamSentries.IntValue)
        {
            int newest = -1, owner, chosen;
            for (int client = 1; client <= MaxClients; client++)
            {
                if (!ValidClient(client) || GetClientTeam(client) != team) continue;
                for (int slot; slot < 3; slot++)
                    if (OwnedExtraSentry(client, g_ExtraSentries[client][slot]) && g_BonusOrder[client][slot] > newest)
                    { newest = g_BonusOrder[client][slot]; owner = client; chosen = slot; }
            }
            if (!owner) break;
            int reference = g_ExtraSentries[owner][chosen];
            g_ExtraSentries[owner][chosen] = INVALID_ENT_REFERENCE;
            RemoveEntity(EntRefToEntIndex(reference));
            RestoreSentryModes(owner);
            count--;
        }
    }
}

bool OwnedExtraSentry(int client, int reference)
{
    int entity = EntRefToEntIndex(reference);
    if (entity <= MaxClients || !IsValidEntity(entity)) return false;
    char classname[32];
    GetEntityClassname(entity, classname, sizeof(classname));
    return StrEqual(classname, "obj_sentrygun") && GetEntPropEnt(entity, Prop_Send, "m_hBuilder") == client;
}

bool PlacedSentry(int client, int entity)
{
    return OwnedExtraSentry(client, EntIndexToEntRef(entity))
        && (!HasEntProp(entity, Prop_Send, "m_bPlacing") || !GetEntProp(entity, Prop_Send, "m_bPlacing"));
}

bool IsBonusSentry(int client, int entity)
{
    int reference = EntIndexToEntRef(entity);
    for (int slot; slot < 3; slot++) if (g_ExtraSentries[client][slot] == reference) return true;
    return false;
}

bool HasPrimarySentry(int client, int ignore = -1)
{
    int entity = -1;
    while ((entity = FindEntityByClassname(entity, "obj_sentrygun")) != -1)
        if (entity != ignore && PlacedSentry(client, entity) && !IsBonusSentry(client, entity)) return true;
    return false;
}

int CountOwnedBuildings(int client, bool sentriesOnly = true)
{
    char classes[][] = {"obj_sentrygun", "obj_dispenser", "obj_teleporter"};
    int count;
    for (int type; type < (sentriesOnly ? 1 : sizeof(classes)); type++)
    {
        int entity = -1;
        while ((entity = FindEntityByClassname(entity, classes[type])) != -1)
            if (GetEntPropEnt(entity, Prop_Send, "m_hBuilder") == client
                && (!HasEntProp(entity, Prop_Send, "m_bPlacing") || !GetEntProp(entity, Prop_Send, "m_bPlacing"))) count++;
    }
    return count;
}

void RestoreSentryModes(int client)
{
    for (int slot; slot < 3; slot++)
    {
        int reference = g_MaskedSentries[client][slot];
        g_MaskedSentries[client][slot] = INVALID_ENT_REFERENCE;
        if (!OwnedExtraSentry(client, reference)) continue;
        int entity = EntRefToEntIndex(reference);
        if (GetEntProp(entity, Prop_Send, "m_iObjectMode") == 1)
            SetEntProp(entity, Prop_Send, "m_iObjectMode", 0);
    }
}

bool ExtraSentryAllowed(int client)
{
    return GameplayFeatureAllowed(client, Feature_Sentry) && TF2_GetPlayerClass(client) == TFClass_Engineer
        && cvSentryLimit.IntValue > 1 && CheckCommandAccess(client, "premium_feature_sentry", 0, true)
        && !GameRules_GetProp("m_bPlayingMannVsMachine") && GameRules_GetProp("m_nGameType") != 4;
}

void UpdateSentryPda(int client, int weapon = -1)
{
    RestoreSentryModes(client);
    if (!ExtraSentryAllowed(client) || !IsPlayerAlive(client) || !g_RoundActive) return;
    bool carrying = HasEntProp(client, Prop_Send, "m_bCarryingObject") && GetEntProp(client, Prop_Send, "m_bCarryingObject") != 0;
    if (!carrying && (CountOwnedBuildings(client) >= cvSentryLimit.IntValue || CountOwnedBuildings(client, false) >= 6
        || GetEntityCount() >= GetMaxEntities() - 32 || (HasPrimarySentry(client) && !TeamExtraRoom(client)))) return;
    if (weapon == -1) weapon = GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon");
    if (!ValidWeapon(weapon)) return;
    char classname[64];
    GetEntityClassname(weapon, classname, sizeof(classname));
    if (!StrEqual(classname, "tf_weapon_pda_engineer_build") && !StrEqual(classname, "tf_weapon_builder")) return;
    int entity = -1, slot;
    while ((entity = FindEntityByClassname(entity, "obj_sentrygun")) != -1 && slot < 3)
    {
        if (!PlacedSentry(client, entity) || GetEntProp(entity, Prop_Send, "m_iObjectMode") != 0
            || (HasEntProp(entity, Prop_Send, "m_bCarried") && GetEntProp(entity, Prop_Send, "m_bCarried"))
            || (HasEntProp(entity, Prop_Send, "m_bDisposableBuilding") && GetEntProp(entity, Prop_Send, "m_bDisposableBuilding"))) continue;
        g_MaskedSentries[client][slot++] = EntIndexToEntRef(entity);
        SetEntProp(entity, Prop_Send, "m_iObjectMode", 1);
    }
}

void LockBonusSentries(int client)
{
    for (int slot; slot < 3; slot++)
    {
        int reference = g_ExtraSentries[client][slot];
        if (!OwnedExtraSentry(client, reference)) continue;
        // Native CanBeUpgraded rejects map-placed sentries without its explicit
        // upgradeable spawnflag. Repairs/ammo/sappers still use native behavior.
        SetEntProp(EntRefToEntIndex(reference), Prop_Send, "m_bWasMapPlaced", 1);
    }
}

void RemoveExtraSentries(int client)
{
    g_NextSentryNotice[client] = 0.0;
    RestoreSentryModes(client);
    for (int slot; slot < 3; slot++)
    {
        int reference = g_ExtraSentries[client][slot];
        g_ExtraSentries[client][slot] = INVALID_ENT_REFERENCE;
        if (OwnedExtraSentry(client, reference)) RemoveEntity(EntRefToEntIndex(reference));
    }
}

void MaintainExtraSentries(int client)
{
    if (!ExtraSentryAllowed(client)) { RemoveExtraSentries(client); return; }
    int count = CountOwnedBuildings(client);
    for (int slot = 2; slot >= 0 && count > cvSentryLimit.IntValue; slot--)
    {
        int reference = g_ExtraSentries[client][slot];
        if (!OwnedExtraSentry(client, reference)) continue;
        g_ExtraSentries[client][slot] = INVALID_ENT_REFERENCE;
        RemoveEntity(EntRefToEntIndex(reference));
        count--;
    }
    LockBonusSentries(client);
    UpdateSentryPda(client);
}

public void SentryWeaponSwitched(int client, int weapon)
{
    UpdateSentryPda(client, weapon);
    UpdateUtilityPda(client, weapon);
}

public Action LimitNativeSentries(int client, const char[] command, int args)
{
    if (!ValidClient(client) || args < 1) return Plugin_Continue;
    char type[16]; GetCmdArg(1, type, sizeof(type));
    if (!PremiumDigits(type) || StringToInt(type) != view_as<int>(TFObject_Sentry)) return Plugin_Continue;
    // Always let native carried-object redeployment through, including at cap.
    if (HasEntProp(client, Prop_Send, "m_bCarryingObject") && GetEntProp(client, Prop_Send, "m_bCarryingObject"))
    { UpdateSentryPda(client, GetPlayerWeaponSlot(client, 3)); return Plugin_Continue; }
    if (!ExtraSentryAllowed(client)) { RestoreSentryModes(client); return Plugin_Continue; }
    if (HasPrimarySentry(client) && !TeamExtraRoom(client))
    { RestoreSentryModes(client); Tell(client, "%t", "Team Sentry Limit", cvTeamSentries.IntValue); return Plugin_Handled; }
    if (!PremiumCanAddSentry(CountOwnedBuildings(client), cvSentryLimit.IntValue)
        || CountOwnedBuildings(client, false) >= 6 || GetEntityCount() >= GetMaxEntities() - 32)
    {
        RestoreSentryModes(client);
        Tell(client, "%t", "Sentry Limit", cvSentryLimit.IntValue);
        return Plugin_Handled;
    }
    // Also supports an existing native build bind; users need no Premium command.
    int pda = GetPlayerWeaponSlot(client, 3);
    UpdateSentryPda(client, pda);
    return Plugin_Continue;
}

void TrackPlacedSentry(int client, int entity)
{
    if (entity > MaxClients && IsValidEntity(entity) && PlacedSentry(client, entity)
        && CountOwnedBuildings(client, false) > 6) { RemoveEntity(entity); return; }
    if (entity > MaxClients && IsValidEntity(entity) && PlacedSentry(client, entity)
        && !IsBonusSentry(client, entity) && HasPrimarySentry(client, entity))
    {
        // This is the second/third sentry, built entirely by the native tool.
        // Close races where access/cap changed between blueprint and placement.
        if (!ExtraSentryAllowed(client) || CountOwnedBuildings(client) > cvSentryLimit.IntValue || !TeamExtraRoom(client))
            RemoveEntity(entity);
        else for (int slot; slot < 3; slot++)
        {
            if (OwnedExtraSentry(client, g_ExtraSentries[client][slot])) continue;
            g_ExtraSentries[client][slot] = EntIndexToEntRef(entity);
            g_BonusOrder[client][slot] = ++g_BonusSequence;
            LockBonusSentries(client);
            break;
        }
    }
}

public void EventBuiltObject(Event event, const char[] name, bool dontBroadcast)
{
    int client = GetClientOfUserId(event.GetInt("userid"));
    if (!ValidClient(client)) return;
    TrackPlacedSentry(client, event.GetInt("index", -1));
    TrackPlacedUtility(client, event.GetInt("index", -1));
    // Let the engine finish its BuiltObject mode-0 notification first.
    RequestFrame(FrameSentryPda, GetClientSerial(client));
}

public void FrameSentryPda(any serial)
{
    int client = GetClientFromSerial(serial);
    if (ValidClient(client)) MaintainExtraBuildings(client);
}
