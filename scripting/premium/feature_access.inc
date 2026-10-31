#include "feature_runtime.inc"

bool ReadFeatureSection(KeyValues file, int &mask)
{
    mask = 0;
    if (!file.GotoFirstSubKey(false)) return true;
    bool valid = true;
    do
    {
        char key[64], value[16]; file.GetSectionName(key, sizeof(key)); file.GetString(NULL_STRING, value, sizeof(value));
        int index = PremiumFeatureIndex(key);
        if (index < 0 || !PremiumFeatureValue(value)) { valid = false; break; }
        if (StrEqual(value, "1")) mask |= 1 << index;
    } while (file.GotoNextKey(false));
    file.GoBack();
    return valid;
}

bool LoadFeatureAccess()
{
    if (g_UserFeatures == null) g_UserFeatures = new StringMap();
    char path[PLATFORM_MAX_PATH]; BuildPath(Path_SM, path, sizeof(path), "configs/premium_features.cfg");
    KeyValues file = new KeyValues("PremiumFeatures");
    StringMap users = new StringMap();
    bool valid = file.ImportFromFile(path);
    int publicMask, count;
    char root[64]; file.GetSectionName(root, sizeof(root));
    valid = valid && StrEqual(root, "PremiumFeatures");
    if (valid && file.JumpToKey("Public")) { valid = ReadFeatureSection(file, publicMask); file.GoBack(); }
    if (valid && file.JumpToKey("Users") && file.GotoFirstSubKey())
    {
        do
        {
            char raw[STEAM_LENGTH], steam[STEAM_LENGTH]; int mask, duplicate;
            file.GetSectionName(raw, sizeof(raw));
            if (++count > 1024 || strlen(raw) >= sizeof(raw) - 1 || !PremiumNormalizeSteam(raw, steam, sizeof(steam))
                || users.GetValue(steam, duplicate) || !ReadFeatureSection(file, mask)) { valid = false; break; }
            users.SetValue(steam, mask);
        } while (file.GotoNextKey());
    }
    delete file;
    if (!valid)
    {
        delete users; g_PublicFeatures = 0; g_UserFeatures.Clear();
        LogError("Invalid/missing %s: independent public/user grants disabled. Membership is unchanged. Use known feature names, 0/1 and unique Steam IDs.", path);
        return false;
    }
    delete g_UserFeatures; g_UserFeatures = users; g_PublicFeatures = publicMask;
    return true;
}

bool SaveFeatureAccess(int publicMask, StringMap users)
{
    char path[PLATFORM_MAX_PATH], temporary[PLATFORM_MAX_PATH], backup[PLATFORM_MAX_PATH];
    BuildPath(Path_SM, path, sizeof(path), "configs/premium_features.cfg");
    FormatEx(temporary, sizeof(temporary), "%s.new", path); FormatEx(backup, sizeof(backup), "%s.bak", path);
    KeyValues file = new KeyValues("PremiumFeatures"); file.JumpToKey("Public", true);
    for (int i; i < view_as<int>(Feature_Count); i++) file.SetNum(g_FeatureKeys[i], (publicMask & (1 << i)) != 0);
    file.GoBack(); file.JumpToKey("Users", true);
    StringMapSnapshot entries = users.Snapshot();
    for (int i; i < entries.Length; i++)
    {
        char steam[STEAM_LENGTH]; int mask; entries.GetKey(i, steam, sizeof(steam)); users.GetValue(steam, mask);
        if (!mask) continue;
        file.JumpToKey(steam, true);
        for (int feature; feature < view_as<int>(Feature_Count); feature++)
            if (mask & (1 << feature)) file.SetNum(g_FeatureKeys[feature], 1);
        file.GoBack();
    }
    delete entries; file.Rewind();
    bool exported = file.ExportToFile(temporary); delete file;
    if (!exported) return false;
    // Both renames target an absent file, including on Windows. Keep the last
    // config as .bak; do not apply new grants unless persistence succeeds.
    bool existed = FileExists(path);
    if (existed)
    {
        if ((FileExists(backup) && !DeleteFile(backup)) || !RenameFile(backup, path)) { DeleteFile(temporary); return false; }
    }
    if (!RenameFile(path, temporary))
    {
        if (existed && !RenameFile(path, backup)) LogError("Could not restore Premium feature configuration backup: %s", backup);
        DeleteFile(temporary); return false;
    }
    return true;
}

StringMap CopyFeatureUsers()
{
    StringMap result = new StringMap();
    StringMapSnapshot entries = g_UserFeatures.Snapshot();
    for (int i; i < entries.Length; i++)
    { char steam[STEAM_LENGTH]; int mask; entries.GetKey(i, steam, sizeof(steam)); g_UserFeatures.GetValue(steam, mask); result.SetValue(steam, mask); }
    delete entries; return result;
}

public Action CommandFeatureReload(int client, int args)
{
    bool loaded = LoadFeatureAccess(); RefreshEverybody();
    Tell(client, "%s", loaded ? "Feature access reloaded. Membership unchanged; spawn-only rewards wait until respawn." : "Feature configuration failed; independent file grants disabled. Check the server log.");
    return Plugin_Handled;
}

public Action CommandFeaturePublic(int client, int args)
{
    char key[64], value[16]; GetCmdArg(1, key, sizeof(key)); GetCmdArg(2, value, sizeof(value)); int index = PremiumFeatureIndex(key);
    if (args != 2 || index < 0 || !PremiumFeatureValue(value))
    { Tell(client, "Usage: sm_premium_feature_public <feature> <0|1>. Use sm_premium_features for feature names."); return Plugin_Handled; }
    int mask = StrEqual(value, "1") ? (g_PublicFeatures | (1 << index)) : (g_PublicFeatures & ~(1 << index));
    if (!SaveFeatureAccess(mask, g_UserFeatures)) { Tell(client, "Could not save feature access; no access was changed."); return Plugin_Handled; }
    g_PublicFeatures = mask; RefreshEverybody();
    Tell(client, "Public %s grant=%s. Feature settings and other grant sources still apply. Membership unchanged.", g_FeatureKeys[index], value);
    return Plugin_Handled;
}

public Action CommandFeatureGrant(int client, int args)
{
    char target[64], key[64], value[16]; GetCmdArg(1, target, sizeof(target)); GetCmdArg(2, key, sizeof(key)); GetCmdArg(3, value, sizeof(value));
    int index = PremiumFeatureIndex(key);
    if (args != 3 || index < 0 || !PremiumFeatureValue(value))
    { Tell(client, "Usage: sm_premium_feature <player|Steam ID> <feature> <0|1>. Use sm_premium_features for names."); return Plugin_Handled; }
    char steamIds[MAXPLAYERS][STEAM_LENGTH]; int count;
    if (PremiumNormalizeSteam(target, steamIds[0], sizeof(steamIds[]))) count = 1;
    else
    {
        int targets[MAXPLAYERS]; char name[MAX_TARGET_LENGTH]; bool multilingual;
        count = ProcessTargetString(target, client, targets, sizeof(targets), COMMAND_FILTER_CONNECTED | COMMAND_FILTER_NO_BOTS, name, sizeof(name), multilingual);
        if (count <= 0) { ReplyToTargetError(client, count); return Plugin_Handled; }
        for (int i; i < count; i++)
        {
            char raw[STEAM_LENGTH];
            if (!GetClientAuthId(targets[i], AuthId_Steam2, raw, sizeof(raw)) || !PremiumNormalizeSteam(raw, steamIds[i], sizeof(steamIds[])))
            { Tell(client, "Target is not authenticated yet; no access changed."); return Plugin_Handled; }
        }
    }
    StringMap staged = CopyFeatureUsers();
    for (int i; i < count; i++)
    {
        int mask; staged.GetValue(steamIds[i], mask);
        mask = StrEqual(value, "1") ? (mask | (1 << index)) : (mask & ~(1 << index));
        if (mask) staged.SetValue(steamIds[i], mask); else staged.Remove(steamIds[i]);
    }
    if (staged.Size > 1024 || !SaveFeatureAccess(g_PublicFeatures, staged))
    { delete staged; Tell(client, "Could not save feature access (maximum 1024 users); no access changed."); return Plugin_Handled; }
    delete g_UserFeatures; g_UserFeatures = staged; RefreshEverybody();
    LogMessage("Individual feature grant: actor=%L feature=%s value=%s targets=%d", client, g_FeatureKeys[index], value, count);
    Tell(client, "%s grant=%s for %d user(s). Other grant sources still apply. Membership unchanged.", g_FeatureKeys[index], value, count);
    return Plugin_Handled;
}

public Action CommandFeatureList(int client, int args)
{
    if (ValidClient(client))
    {
        RefreshClient(client);
        Menu menu = new Menu(FeatureAccessSelected);
        menu.SetTitle("Feature access | Premium: %s\nBalance and map/team rules still apply", HasPremium(client) ? "active" : "inactive");
        for (int i; i < view_as<int>(Feature_Count); i++)
        {
            char label[96]; FormatEx(label, sizeof(label), "%s: %s%s", g_FeatureKeys[i], HasFeature(client, view_as<PremiumFeature>(i)) ? "granted" : "no access",
                (g_PublicFeatures & (1 << i)) ? " (public)" : "");
            menu.AddItem("info", label, ITEMDRAW_DISABLED);
        }
        menu.ExitBackButton = true; menu.Display(client, 30);
        return Plugin_Handled;
    }
    Tell(client, "Feature keys (access does not bypass balance, team, class, map or enable settings):");
    for (int i; i < view_as<int>(Feature_Count); i++)
        Tell(client, "%s: public=%d your_access=%d", g_FeatureKeys[i], (g_PublicFeatures & (1 << i)) != 0,
            ValidClient(client) && HasFeature(client, view_as<PremiumFeature>(i)));
    return Plugin_Handled;
}

public int FeatureAccessSelected(Menu menu, MenuAction action, int client, int selection)
{
    if (action == MenuAction_End) delete menu;
    else if (action == MenuAction_Cancel && selection == MenuCancel_ExitBack && ValidClient(client)) DisplayFeatures(client);
    return 0;
}
