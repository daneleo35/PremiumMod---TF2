#include "ui_policy.inc"

char g_UiFeatureNames[][] = {
    "Movement speed", "Gravity", "Starting health", "Spare ammo / fuel", "Health for kills", "Health for assists",
    "Player colour", "Dark tint", "Glow", "Field of view", "Scout balls", "Sniper jars", "Starting Medic charge",
    "Faster respawn", "Team swap", "Cloak duration", "Dead Ringer duration", "Starting metal", "Extra sentries",
    "Extra dispensers", "Extra teleporters", "Lit arrows", "Bunnyhop / trimp", "Follow-up jump", "Pyro flame flight",
    "Airblast jump", "Saved-location teleport", "Round-win celebration", "Fake-kill notice"
};

bool UiAdmin(int client)
{
    return ValidClient(client) && !IsFakeClient(client) && CheckCommandAccess(client, "sm_premium_admin", ADMFLAG_ROOT);
}

void UiStatus(int client, char[] output, int size)
{
    char time[32];
    if (!cvEnable.BoolValue) strcopy(output, size, "Premium is switched off");
    else if (FlagPremium(client) || (g_Player[client].loaded && g_Player[client].member)) strcopy(output, size, "Premium member");
    else if (EconomyTrialActive(client))
    { PremiumUiDuration(PremiumRemaining(g_Economy[client].trialUntil, GetTime()), time, sizeof(time)); FormatEx(output, size, "Purchased trial | %s left", time); }
    else if (HasPremium(client))
    { PremiumUiDuration(PremiumRemaining(g_Player[client].trialUntil, GetTime()), time, sizeof(time)); FormatEx(output, size, "Free trial | %s left", time); }
    else if (!g_Player[client].loaded) strcopy(output, size, "Membership is loading...");
    else strcopy(output, size, HasAnyFeature(client) ? "Individual features unlocked" : "Standard player");
}

void UiHome(int client)
{
    if (!ValidClient(client) || IsFakeClient(client)) return;
    RefreshClient(client);
    char status[96], wallet[64]; UiStatus(client, status, sizeof(status));
    if (!cvPoints.BoolValue) strcopy(wallet, sizeof(wallet), "Points are switched off");
    else if (!g_EconomyReady || !g_Economy[client].loaded) strcopy(wallet, sizeof(wallet), "Points are loading...");
    else FormatEx(wallet, sizeof(wallet), "Your points: %d", g_Economy[client].points);
    Menu menu = new Menu(UiHomeSelected); menu.SetTitle("Premium\n%s\n%s\n ", status, wallet);
    menu.AddItem("benefits", "My bonuses");
    menu.AddItem("features", "My unlocked features");
    menu.AddItem("looks", "Appearance");
    menu.AddItem("shop", cvPoints.BoolValue && cvPointsShop.BoolValue ? "Points shop & my purchases" : "Points & my purchases");
    menu.AddItem("actions", "Actions & free trial");
    menu.AddItem("options", "Personal settings");
    if (UiAdmin(client)) menu.AddItem("admin", "Admin controls");
    menu.Display(client, 60);
}

public int UiHomeSelected(Menu menu, MenuAction action, int client, int selection)
{
    if (action == MenuAction_End) delete menu;
    else if (action == MenuAction_Select && ValidClient(client))
    {
        char key[24]; menu.GetItem(selection, key, sizeof(key));
        if (StrEqual(key, "benefits")) DisplayBenefits(client);
        else if (StrEqual(key, "features")) UiFeatures(client);
        else if (StrEqual(key, "looks")) UiAppearance(client);
        else if (StrEqual(key, "shop")) UiWallet(client);
        else if (StrEqual(key, "actions")) UiActions(client);
        else if (StrEqual(key, "options")) DisplayOptions(client);
        else if (StrEqual(key, "admin")) UiAdminHome(client);
    }
    return 0;
}

void UiFeatureReason(int client, int index, char[] text, int size)
{
    if (!HasFeature(client, view_as<PremiumFeature>(index))) strcopy(text, size, "Not unlocked");
    else if ((PremiumGameplayFeatures() & (1 << index)) && !PremiumTeamAllowed(GetClientTeam(client), cvGameplayTeams.IntValue)) strcopy(text, size, "Blocked for your team");
    else if (!ShopConfigured(client, index)) strcopy(text, size, "Off for this class / map / setting");
    else strcopy(text, size, "Unlocked and enabled");
}

void UiFeatures(int client)
{
    RefreshClient(client);
    Menu menu = new Menu(UiFeaturesSelected); menu.SetTitle("My features\nChoose a feature for details\n ");
    for (int i; i < view_as<int>(Feature_Count); i++)
    {
        char data[12], label[160], status[72]; IntToString(i, data, sizeof(data)); UiFeatureReason(client, i, status, sizeof(status));
        FormatEx(label, sizeof(label), "%s | %s", g_UiFeatureNames[i], status); menu.AddItem(data, label);
    }
    menu.ExitBackButton = true; menu.Display(client, 60);
}

public int UiFeaturesSelected(Menu menu, MenuAction action, int client, int selection)
{
    if (action == MenuAction_End) delete menu;
    else if (action == MenuAction_Cancel && selection == MenuCancel_ExitBack && ValidClient(client)) UiHome(client);
    else if (action == MenuAction_Select && ValidClient(client))
    {
        char data[12], status[80], time[32], detail[160]; int index; menu.GetItem(selection, data, sizeof(data));
        if (!PremiumUiIndex(data, view_as<int>(Feature_Count), index)) return 0;
        UiFeatureReason(client, index, status, sizeof(status));
        Menu info = new Menu(UiFeatureInfoSelected); info.SetTitle("%s\n%s\n ", g_UiFeatureNames[index], status);
        int now = GetTime();
        if (g_Economy[client].loaded && g_Economy[client].rentalUntil[index] > now)
        { PremiumUiDuration(g_Economy[client].rentalUntil[index] - now, time, sizeof(time)); FormatEx(detail, sizeof(detail), "Rental: %s left (offline time counts)", time); info.AddItem("info", detail, ITEMDRAW_DISABLED); }
        if (g_PublicFeatures & (1 << index)) info.AddItem("info", "Free to everyone on this server", ITEMDRAW_DISABLED);
        int personal; g_UserFeatures.GetValue(g_Player[client].steam, personal);
        if (personal & (1 << index)) info.AddItem("info", "Granted to your account", ITEMDRAW_DISABLED);
        if (HasPremium(client)) info.AddItem("info", "Included with your Premium access", ITEMDRAW_DISABLED);
        if (index == view_as<int>(Feature_Glow)) info.AddItem("glow", g_Glow[client].requested ? "Turn glow off" : "Turn glow on");
        else if (index == view_as<int>(Feature_Dark)) info.AddItem("dark", g_Player[client].dark ? "Turn dark tint off" : "Turn dark tint on");
        else if (index == view_as<int>(Feature_Fov)) info.AddItem("fov", "Choose field of view");
        else if (index == view_as<int>(Feature_Color)) info.AddItem("colour", "Choose player colour");
        else info.AddItem("benefits", "View my configured bonuses");
        info.AddItem("shop", "Browse the points shop");
        info.ExitBackButton = true; info.Display(client, 60);
    }
    return 0;
}

public int UiFeatureInfoSelected(Menu menu, MenuAction action, int client, int selection)
{
    if (action == MenuAction_End) delete menu;
    else if (action == MenuAction_Cancel && selection == MenuCancel_ExitBack && ValidClient(client)) UiFeatures(client);
    else if (action == MenuAction_Select && ValidClient(client))
    {
        char key[20]; menu.GetItem(selection, key, sizeof(key));
        if (StrEqual(key, "glow")) { ToggleGlow(client, -1); UiAppearance(client); }
        else if (StrEqual(key, "dark")) { CommandDark(client, 0); UiAppearance(client); }
        else if (StrEqual(key, "fov")) UiFov(client);
        else if (StrEqual(key, "colour")) UiColour(client);
        else if (StrEqual(key, "shop")) DisplayPointsShop(client);
        else DisplayBenefits(client);
    }
    return 0;
}

void UiAppearance(int client)
{
    Menu menu = new Menu(UiAppearanceSelected); menu.SetTitle("Appearance\nChoose your look\n ");
    menu.AddItem("glow", g_Glow[client].requested ? "Glow: ON (turn off)" : "Glow: OFF (turn on)");
    menu.AddItem("dark", g_Player[client].dark ? "Dark tint: ON (turn off)" : "Dark tint: OFF (turn on)");
    menu.AddItem("colour", "Player colour"); menu.AddItem("fov", "Field of view"); menu.AddItem("options", "Tag, glow style & visibility settings");
    menu.ExitBackButton = true; menu.Display(client, 60);
}

public int UiAppearanceSelected(Menu menu, MenuAction action, int client, int selection)
{
    if (action == MenuAction_End) delete menu;
    else if (action == MenuAction_Cancel && selection == MenuCancel_ExitBack && ValidClient(client)) UiHome(client);
    else if (action == MenuAction_Select && ValidClient(client))
    {
        char key[20]; menu.GetItem(selection, key, sizeof(key));
        if (StrEqual(key, "glow")) { ToggleGlow(client, -1); UiAppearance(client); }
        else if (StrEqual(key, "dark")) { CommandDark(client, 0); UiAppearance(client); }
        else if (StrEqual(key, "colour")) UiColour(client);
        else if (StrEqual(key, "fov")) UiFov(client);
        else DisplayOptions(client);
    }
    return 0;
}

void UiColour(int client)
{
    Menu menu = new Menu(UiColourSelected); menu.SetTitle("Player colour\nServer colour settings still apply\n ");
    menu.AddItem("reset", "Server default"); menu.AddItem("255 100 100", "Red"); menu.AddItem("100 150 255", "Blue");
    menu.AddItem("100 255 100", "Green"); menu.AddItem("255 210 80", "Gold"); menu.AddItem("210 100 255", "Purple"); menu.AddItem("255 255 255", "White");
    menu.ExitBackButton = true; menu.Display(client, 60);
}

public int UiColourSelected(Menu menu, MenuAction action, int client, int selection)
{
    if (action == MenuAction_End) delete menu;
    else if (action == MenuAction_Cancel && selection == MenuCancel_ExitBack && ValidClient(client)) UiAppearance(client);
    else if (action == MenuAction_Select && ValidClient(client))
    { char value[24]; menu.GetItem(selection, value, sizeof(value)); FakeClientCommand(client, "sm_colorme %s", value); UiColour(client); }
    return 0;
}

void UiFov(int client)
{
    Menu menu = new Menu(UiFovSelected); menu.SetTitle("Field of view\nChoose a value\n ");
    menu.AddItem("reset", "Server default"); menu.AddItem("75", "75"); menu.AddItem("90", "90"); menu.AddItem("100", "100"); menu.AddItem("110", "110"); menu.AddItem("120", "120");
    menu.ExitBackButton = true; menu.Display(client, 60);
}
public int UiFovSelected(Menu menu, MenuAction action, int client, int selection)
{
    if (action == MenuAction_End) delete menu;
    else if (action == MenuAction_Cancel && selection == MenuCancel_ExitBack && ValidClient(client)) UiAppearance(client);
    else if (action == MenuAction_Select && ValidClient(client))
    { char value[12]; menu.GetItem(selection, value, sizeof(value)); FakeClientCommand(client, "sm_premium_fov %s", value); UiFov(client); }
    return 0;
}

void UiActions(int client)
{
    char status[96], reason[192]; UiStatus(client, status, sizeof(status)); SwapAvailability(client, reason, sizeof(reason));
    Menu menu = new Menu(UiActionsSelected); menu.SetTitle("Actions & free trial\n%s\n ", status);
    menu.AddItem("swap", "Swap teams"); menu.AddItem("swapstatus", "Why can / can't I swap?");
    menu.AddItem("save", "Save my current location"); menu.AddItem("load", g_Player[client].saved ? "Return to saved location" : "Return to location (none saved)");
    char trial[96];
    if (!cvTrial.BoolValue) strcopy(trial, sizeof(trial), "Free trial: disabled by owner");
    else if (HasPremium(client)) strcopy(trial, sizeof(trial), "Free trial: Premium already active");
    else if (!g_Player[client].loaded) strcopy(trial, sizeof(trial), "Free trial: loading your account");
    else if (g_Player[client].usedTrial) strcopy(trial, sizeof(trial), "Free trial: already used");
    else strcopy(trial, sizeof(trial), "Start my free trial");
    menu.AddItem("trial", trial); menu.AddItem("info", reason, ITEMDRAW_DISABLED);
    menu.ExitBackButton = true; menu.Display(client, 60);
}

public int UiActionsSelected(Menu menu, MenuAction action, int client, int selection)
{
    if (action == MenuAction_End) delete menu;
    else if (action == MenuAction_Cancel && selection == MenuCancel_ExitBack && ValidClient(client)) UiHome(client);
    else if (action == MenuAction_Select && ValidClient(client))
    {
        char key[20]; menu.GetItem(selection, key, sizeof(key));
        if (StrEqual(key, "swap")) CommandSwap(client, 0);
        else if (StrEqual(key, "save")) CommandSave(client, 0);
        else if (StrEqual(key, "load")) CommandLoad(client, 0);
        else if (StrEqual(key, "trial"))
        {
            Menu confirm = new Menu(UiTrialSelected); char time[32]; PremiumUiDuration(cvTrialSeconds.IntValue, time, sizeof(time));
            confirm.SetTitle("Start your free trial?\nLength: %s\nOne free claim. Offline time counts.\n ", time);
            confirm.AddItem("yes", "Start trial"); confirm.AddItem("no", "Not now"); confirm.ExitBackButton = true; confirm.Display(client, 30); return 0;
        }
        else CommandSwapStatus(client, 0);
        UiActions(client);
    }
    return 0;
}
public int UiTrialSelected(Menu menu, MenuAction action, int client, int selection)
{
    if (action == MenuAction_End) delete menu;
    else if (action == MenuAction_Cancel && selection == MenuCancel_ExitBack && ValidClient(client)) UiActions(client);
    else if (action == MenuAction_Select && ValidClient(client))
    { char key[8]; menu.GetItem(selection, key, sizeof(key)); if (StrEqual(key, "yes")) CommandTrial(client, 0); UiActions(client); }
    return 0;
}

void UiWallet(int client)
{
    Menu menu = new Menu(UiWalletSelected); char title[256], time[32], limit[24];
    if (!cvPoints.BoolValue) strcopy(title, sizeof(title), "Points & purchases\nPoints are switched off\n ");
    else if (!g_EconomyReady || !g_Economy[client].loaded) strcopy(title, sizeof(title), "Points & purchases\nAccount is loading / unavailable\n ");
    else
    {
        PremiumUiDuration(cvPointsMinutes.IntValue * 60, time, sizeof(time));
        if (cvPointsEarn.BoolValue) FormatEx(title, sizeof(title), "Points & purchases\nBalance: %d points\nEarn %d points per %s of eligible play\n ", g_Economy[client].points, cvPointsReward.IntValue, time);
        else FormatEx(title, sizeof(title), "Points & purchases\nBalance: %d points\nEarning is paused by the owner\n ", g_Economy[client].points);
    }
    menu.SetTitle(title); menu.AddItem("shop", cvPoints.BoolValue && cvPointsShop.BoolValue ? "Browse the shop" : "Shop: switched off", cvPoints.BoolValue && cvPointsShop.BoolValue ? ITEMDRAW_DEFAULT : ITEMDRAW_DISABLED);
    int now = GetTime(), shown;
    if (g_Economy[client].loaded)
    {
        for (int i; i < view_as<int>(Feature_Count); i++) if (g_Economy[client].rentalUntil[i] > now)
        { char label[128]; PremiumUiDuration(g_Economy[client].rentalUntil[i] - now, time, sizeof(time)); FormatEx(label, sizeof(label), "%s: %s left", g_UiFeatureNames[i], time); menu.AddItem("info", label, ITEMDRAW_DISABLED); shown++; }
        if (g_Economy[client].trialUntil > now)
        { char label[96]; PremiumUiDuration(g_Economy[client].trialUntil - now, time, sizeof(time)); FormatEx(label, sizeof(label), "Bought Premium trial: %s left", time); menu.AddItem("info", label, ITEMDRAW_DISABLED); shown++; }
        if (cvPointsTrialLimit.IntValue) IntToString(cvPointsTrialLimit.IntValue, limit, sizeof(limit)); else strcopy(limit, sizeof(limit), "unlimited");
        char usage[96]; FormatEx(usage, sizeof(usage), "Trial purchases used: %d / %s", g_Economy[client].trialPurchases, limit); menu.AddItem("info", usage, ITEMDRAW_DISABLED);
    }
    if (g_Economy[client].purchasing) menu.AddItem("info", "Saving purchase... refresh shortly", ITEMDRAW_DISABLED);
    if (!shown) { char empty[96]; PremiumUiEmptyPurchases(g_Economy[client].loaded, empty, sizeof(empty)); menu.AddItem("info", empty, ITEMDRAW_DISABLED); }
    menu.AddItem("refresh", "Refresh balance & purchases"); menu.ExitBackButton = true; menu.Display(client, 60);
}
public int UiWalletSelected(Menu menu, MenuAction action, int client, int selection)
{
    if (action == MenuAction_End) delete menu;
    else if (action == MenuAction_Cancel && selection == MenuCancel_ExitBack && ValidClient(client)) UiHome(client);
    else if (action == MenuAction_Select && ValidClient(client))
    { char key[12]; menu.GetItem(selection, key, sizeof(key)); if (StrEqual(key, "shop")) DisplayPointsShop(client); else { if (cvPoints.BoolValue && !g_Economy[client].loaded) EconomyAvailable(client); UiWallet(client); } }
    return 0;
}

public Action CommandAdminMenu(int client, int args)
{
    if (!ValidClient(client)) Tell(client, "%t", "In Game Only"); else UiAdminHome(client);
    return Plugin_Handled;
}

void UiAdminHome(int client)
{
    if (!UiAdmin(client)) return;
    Menu menu = new Menu(UiAdminHomeSelected); menu.SetTitle("Premium admin\nChanges require confirmation\n ");
    menu.AddItem("players", "Manage a player"); menu.AddItem("public", "Free features for everyone");
    menu.AddItem("settings", "Server settings"); menu.AddItem("profiles", "Balance presets"); menu.AddItem("reload", "Reload files / check status");
    menu.ExitBackButton = true; menu.Display(client, 60);
}
public int UiAdminHomeSelected(Menu menu, MenuAction action, int client, int selection)
{
    if (action == MenuAction_End) delete menu;
    else if (action == MenuAction_Cancel && selection == MenuCancel_ExitBack && UiAdmin(client)) UiHome(client);
    else if (action == MenuAction_Select && UiAdmin(client))
    {
        char key[20]; menu.GetItem(selection, key, sizeof(key));
        if (StrEqual(key, "players")) UiPlayers(client);
        else if (StrEqual(key, "public")) UiGrantFeatures(client, 0);
        else if (StrEqual(key, "profiles")) UiProfiles(client);
        else if (StrEqual(key, "settings")) UiSettings(client);
        else UiReloads(client);
    }
    return 0;
}

int UiTarget(int client, int serial)
{
    int target = GetClientFromSerial(serial);
    bool valid = ValidClient(target);
    if (!PremiumUiTargetAllowed(UiAdmin(client), serial, valid ? GetClientSerial(target) : 0,
        valid && !IsFakeClient(target), valid && IsClientAuthorized(target), valid && CanUserTarget(client, target)))
    { Tell(client, "That player left, is not ready, or cannot be targeted. No change made."); return 0; }
    return target;
}

void UiPlayers(int client)
{
    if (!UiAdmin(client)) return;
    Menu menu = new Menu(UiPlayersSelected); menu.SetTitle("Manage a player\nChoose a connected player\n "); int shown;
    for (int target = 1; target <= MaxClients; target++)
    {
        if (!ValidClient(target) || IsFakeClient(target) || !IsClientAuthorized(target) || !CanUserTarget(client, target)) continue;
        char data[16], label[160], status[96]; IntToString(GetClientSerial(target), data, sizeof(data)); UiStatus(target, status, sizeof(status));
        FormatEx(label, sizeof(label), "%N | %s", target, status); menu.AddItem(data, label); shown++;
    }
    if (!shown) menu.AddItem("none", "No players available", ITEMDRAW_DISABLED);
    menu.ExitBackButton = true; menu.Display(client, 60);
}
public int UiPlayersSelected(Menu menu, MenuAction action, int client, int selection)
{
    if (action == MenuAction_End) delete menu;
    else if (action == MenuAction_Cancel && selection == MenuCancel_ExitBack && UiAdmin(client)) UiAdminHome(client);
    else if (action == MenuAction_Select && UiAdmin(client))
    { char data[16]; menu.GetItem(selection, data, sizeof(data)); UiPlayer(client, StringToInt(data)); }
    return 0;
}
void UiPlayer(int client, int serial)
{
    int target = UiTarget(client, serial); if (!target) return;
    Menu menu = new Menu(UiPlayerSelected); char data[48], status[96]; UiStatus(target, status, sizeof(status));
    menu.SetTitle("Manage %N\n%s\n ", target, status);
    char operations[][] = {"add", "remove", "trial", "reset", "features", "points"};
    char labels[][] = {"Give saved Premium membership", "Remove saved membership only", "Give a free trial", "Reset / revoke free trial", "Give / remove individual features", "Add / remove points"};
    for (int i; i < sizeof(operations); i++) { FormatEx(data, sizeof(data), "%s:%d", operations[i], serial); menu.AddItem(data, labels[i]); }
    menu.ExitBackButton = true; menu.Display(client, 60);
}
public int UiPlayerSelected(Menu menu, MenuAction action, int client, int selection)
{
    if (action == MenuAction_End) delete menu;
    else if (action == MenuAction_Cancel && selection == MenuCancel_ExitBack && UiAdmin(client)) UiPlayers(client);
    else if (action == MenuAction_Select && UiAdmin(client))
    {
        char data[64], parts[2][32], text[192]; menu.GetItem(selection, data, sizeof(data)); if (ExplodeString(data, ":", parts, sizeof(parts), sizeof(parts[])) != 2) return 0;
        int serial = StringToInt(parts[1]), target = UiTarget(client, serial); if (!target) return 0;
        if (StrEqual(parts[0], "features")) UiGrantFeatures(client, serial);
        else if (StrEqual(parts[0], "points")) UiPointsAdjust(client, serial);
        else { FormatEx(text, sizeof(text), "Apply '%s' to %N?\nOther access sources remain.\nFree trials must be enabled to use them.", parts[0], target); UiAdminConfirm(client, data, text); }
    }
    return 0;
}
void UiGrantFeatures(int client, int serial)
{
    if (!UiAdmin(client)) return;
    int target; if (serial && !(target = UiTarget(client, serial))) return;
    Menu menu = new Menu(UiGrantSelected); if (target) menu.SetTitle("Individual free grants: %N\nOnly this source is changed\n ", target); else menu.SetTitle("Free features for everyone\nOnly public grants are changed\n ");
    int mask = g_PublicFeatures;
    if (target) { char auth[STEAM_LENGTH], steam[STEAM_LENGTH]; GetClientAuthId(target, AuthId_Steam3, auth, sizeof(auth)); if (!PremiumNormalizeSteam(auth, steam, sizeof(steam))) { delete menu; return; } mask = 0; g_UserFeatures.GetValue(steam, mask); }
    for (int i; i < view_as<int>(Feature_Count); i++)
    { char data[64], label[128]; FormatEx(data, sizeof(data), "%s:%d:%d:%d", target ? "grant" : "public", i, (mask & (1 << i)) ? 0 : 1, serial); FormatEx(label, sizeof(label), "%s: %s", g_UiFeatureNames[i], (mask & (1 << i)) ? "granted (remove)" : "not granted (give)"); menu.AddItem(data, label); }
    menu.ExitBackButton = true; menu.Display(client, 60);
}
public int UiGrantSelected(Menu menu, MenuAction action, int client, int selection)
{
    if (action == MenuAction_End) delete menu;
    else if (action == MenuAction_Cancel && selection == MenuCancel_ExitBack && UiAdmin(client)) UiAdminHome(client);
    else if (action == MenuAction_Select && UiAdmin(client))
    {
        char data[96], parts[4][32], title[192]; int index; menu.GetItem(selection, data, sizeof(data));
        if (ExplodeString(data, ":", parts, sizeof(parts), sizeof(parts[])) != 4 || !PremiumUiIndex(parts[1], view_as<int>(Feature_Count), index)) return 0;
        int target; if (StringToInt(parts[3]) && !(target = UiTarget(client, StringToInt(parts[3])))) return 0;
        if (target) FormatEx(title, sizeof(title), "%s %s for %N?\nChanges their individual free grant only.", StrEqual(parts[2], "1") ? "Give" : "Remove", g_UiFeatureNames[index], target);
        else FormatEx(title, sizeof(title), "%s %s for everyone?\nChanges the public free grant only.", StrEqual(parts[2], "1") ? "Give" : "Remove", g_UiFeatureNames[index]);
        UiAdminConfirm(client, data, title);
    }
    return 0;
}
void UiPointsAdjust(int client, int serial)
{
    int target = UiTarget(client, serial); if (!target) return;
    Menu menu = new Menu(UiPointsAdjustSelected); menu.SetTitle("Points: %N\nChoose an amount\n ", target);
    int amounts[] = {10, 50, 100, 500, -10, -50, -100};
    for (int i; i < sizeof(amounts); i++) { char data[64], label[48]; FormatEx(data, sizeof(data), "points:%d:%d", amounts[i], serial); FormatEx(label, sizeof(label), "%s %d points", amounts[i] > 0 ? "Add" : "Remove", amounts[i] > 0 ? amounts[i] : -amounts[i]); menu.AddItem(data, label); }
    menu.ExitBackButton = true; menu.Display(client, 60);
}
public int UiPointsAdjustSelected(Menu menu, MenuAction action, int client, int selection)
{
    if (action == MenuAction_End) delete menu;
    else if (action == MenuAction_Cancel && selection == MenuCancel_ExitBack && UiAdmin(client)) UiPlayers(client);
    else if (action == MenuAction_Select && UiAdmin(client))
    { char data[64], parts[3][24], title[128]; menu.GetItem(selection, data, sizeof(data)); if (ExplodeString(data, ":", parts, sizeof(parts), sizeof(parts[])) != 3) return 0; int target = UiTarget(client, StringToInt(parts[2])); if (!target) return 0; FormatEx(title, sizeof(title), "Adjust %N's points by %s?", target, parts[1]); UiAdminConfirm(client, data, title); }
    return 0;
}

void UiProfiles(int client)
{
    if (!UiAdmin(client)) return;
    Menu menu = new Menu(UiProfilesSelected); menu.SetTitle("Balance presets\nQueued: %s\n ", g_PendingProfile[0] ? g_PendingProfile : "none");
    menu.AddItem("profile:0:0", "Cosmetic - next round"); menu.AddItem("profile:1:0", "Balanced - next round"); menu.AddItem("profile:2:0", "Fun - next round");
    menu.AddItem("profile:0:1", "Cosmetic - apply now"); menu.AddItem("profile:1:1", "Balanced - apply now"); menu.AddItem("profile:2:1", "Fun - apply now"); menu.AddItem("cancelprofile", "Cancel queued preset");
    menu.ExitBackButton = true; menu.Display(client, 60);
}
public int UiProfilesSelected(Menu menu, MenuAction action, int client, int selection)
{
    if (action == MenuAction_End) delete menu;
    else if (action == MenuAction_Cancel && selection == MenuCancel_ExitBack && UiAdmin(client)) UiAdminHome(client);
    else if (action == MenuAction_Select && UiAdmin(client))
    { char data[64]; menu.GetItem(selection, data, sizeof(data)); UiAdminConfirm(client, data, "Apply this preset choice?\nImmediate changes can remove extra buildings.\nSave server settings separately to keep live values."); }
    return 0;
}
void UiReloads(int client)
{
    if (!UiAdmin(client)) return;
    Menu menu = new Menu(UiReloadsSelected); menu.SetTitle("Files & status\n ");
    menu.AddItem("diagnostics", "Show plugin / database status"); menu.AddItem("reload:0", "Reload free-feature file"); menu.AddItem("reload:1", "Reload shop offers"); menu.AddItem("reload:2", "Reload class / map rules"); menu.AddItem("reload:3", "Refresh membership & effects");
    menu.ExitBackButton = true; menu.Display(client, 60);
}
public int UiReloadsSelected(Menu menu, MenuAction action, int client, int selection)
{
    if (action == MenuAction_End) delete menu;
    else if (action == MenuAction_Cancel && selection == MenuCancel_ExitBack && UiAdmin(client)) UiAdminHome(client);
    else if (action == MenuAction_Select && UiAdmin(client))
    { char data[32]; menu.GetItem(selection, data, sizeof(data)); if (StrEqual(data, "diagnostics")) { FakeClientCommand(client, "sm_premium_diagnostics"); UiReloads(client); } else UiAdminConfirm(client, data, "Reload this file / refresh live data?\nUnsaved manual file edits are not changed."); }
    return 0;
}
void UiAdminConfirm(int client, const char[] operation, const char[] title)
{
    if (!UiAdmin(client)) return;
    Menu menu = new Menu(UiAdminConfirmSelected); menu.SetTitle("%s\n ", title); menu.AddItem(operation, "Confirm change"); menu.AddItem("cancel", "Cancel"); menu.ExitBackButton = true; menu.Display(client, 30);
}
public int UiAdminConfirmSelected(Menu menu, MenuAction action, int client, int selection)
{
    if (action == MenuAction_End) delete menu;
    else if (action == MenuAction_Cancel && selection == MenuCancel_ExitBack && UiAdmin(client)) UiAdminHome(client);
    else if (action == MenuAction_Select && UiAdmin(client))
    {
        char data[128], parts[4][32]; menu.GetItem(selection, data, sizeof(data));
        if (StrEqual(data, "cancel")) { UiAdminHome(client); return 0; }
        int count = ExplodeString(data, ":", parts, sizeof(parts), sizeof(parts[]));
        int target, index;
        if (StrEqual(parts[0], "grant") || StrEqual(parts[0], "public"))
        {
            if (count != 4 || !PremiumUiIndex(parts[1], view_as<int>(Feature_Count), index) || !PremiumFeatureValue(parts[2])) return 0;
            if (StrEqual(parts[0], "grant")) { target = UiTarget(client, StringToInt(parts[3])); if (!target) return 0; FakeClientCommand(client, "sm_premium_feature #%d %s %s", GetClientUserId(target), g_FeatureKeys[index], parts[2]); }
            else FakeClientCommand(client, "sm_premium_feature_public %s %s", g_FeatureKeys[index], parts[2]);
            UiGrantFeatures(client, target ? GetClientSerial(target) : 0); return 0;
        }
        else if (StrEqual(parts[0], "points"))
        { if (count != 3 || !(target = UiTarget(client, StringToInt(parts[2])))) return 0; int amount = StringToInt(parts[1]); if (!amount || amount < -1000000 || amount > 1000000) return 0; FakeClientCommand(client, "sm_premium_points_adjust #%d %d", GetClientUserId(target), amount); UiPlayer(client, GetClientSerial(target)); return 0; }
        else if (StrEqual(parts[0], "profile"))
        { if (count != 3 || !PremiumUiIndex(parts[1], 3, index) || !PremiumFeatureValue(parts[2])) return 0; char names[][] = {"cosmetic", "balanced", "fun"}; FakeClientCommand(client, "sm_premium_profile %s %s", names[index], StrEqual(parts[2], "1") ? "now" : "next"); UiProfiles(client); return 0; }
        else if (StrEqual(parts[0], "cancelprofile")) FakeClientCommand(client, "sm_premium_profile cancel");
        else if (StrEqual(parts[0], "reload"))
        { if (count != 2 || !PremiumUiIndex(parts[1], 4, index)) return 0; char commands[][] = {"sm_premium_features_reload", "sm_premium_shop_reload", "sm_premium_rules_reload", "sm_prem_reload"}; FakeClientCommand(client, "%s", commands[index]); }
        else if (StrEqual(parts[0], "savecfg")) { if (count != 2 || !PremiumDigits(parts[1]) || StringToInt(parts[1]) != g_UiSettingsRevision) Tell(client, "Settings changed while this confirmation was open. Review them and Save again."); else UiSaveSettings(client); UiSettings(client); return 0; }
        else
        {
            if (count != 2 || !(target = UiTarget(client, StringToInt(parts[1])))) return 0;
            if (StrEqual(parts[0], "add")) FakeClientCommand(client, "sm_premadduser #%d", GetClientUserId(target));
            else if (StrEqual(parts[0], "remove")) FakeClientCommand(client, "sm_premremuser #%d", GetClientUserId(target));
            else if (StrEqual(parts[0], "trial")) FakeClientCommand(client, "sm_createtrial #%d", GetClientUserId(target));
            else if (StrEqual(parts[0], "reset")) FakeClientCommand(client, "sm_resettrial #%d", GetClientUserId(target));
            UiPlayer(client, GetClientSerial(target)); return 0;
        }
        UiAdminHome(client);
    }
    return 0;
}
