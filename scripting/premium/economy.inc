void EconomyChat(int client, const char[] format, any ...)
{
    if (!ValidClient(client)) return;
    char message[384], tag[64]; SetGlobalTransTarget(client); VFormat(message, sizeof(message), format, 3); cvTag.GetString(tag, sizeof(tag));
    CPrintToChat(client, "%s %s", tag, message);
}

void ClearEconomyClient(int client)
{
    int epoch = g_Economy[client].epoch + 1;
    EconomyClient clean; g_Economy[client] = clean; g_Economy[client].epoch = epoch;
    g_Economy[client].lastClock = GetEngineTime(); g_Economy[client].lastInput = GetEngineTime();
}

bool ShopIdValid(const char[] id)
{
    if (!id[0] || strlen(id) >= 31) return false;
    for (int i; id[i]; i++) if (!((id[i] >= 'a' && id[i] <= 'z') || (id[i] >= '0' && id[i] <= '9') || id[i] == '_')) return false;
    return true;
}
bool ReadShopInt(KeyValues file, const char[] key, int minimum, int maximum, int &value)
{
    char raw[32]; file.GetString(key, raw, sizeof(raw));
    if (!PremiumDigits(raw) || strlen(raw) > 9) return false;
    value = StringToInt(raw); return value >= minimum && value <= maximum;
}
bool LoadShopCatalog()
{
    g_ShopRevision++; g_ShopCount = 0; g_ShopValid = false;
    char path[PLATFORM_MAX_PATH]; BuildPath(Path_SM, path, sizeof(path), "configs/premium_shop.cfg");
    if (!FileExists(path) && !cvPoints.BoolValue) return false;
    KeyValues file = new KeyValues("PremiumShop"); bool valid = file.ImportFromFile(path);
    char root[64]; file.GetSectionName(root, sizeof(root)); valid = valid && StrEqual(root, "PremiumShop");
    if (valid && file.JumpToKey("Items") && file.GotoFirstSubKey()) do
    {
        ShopOffer offer; char feature[32], unit[16]; int enabled, amount;
        file.GetSectionName(offer.id, sizeof(offer.id)); file.GetString("name", offer.name, sizeof(offer.name));
        file.GetString("feature", feature, sizeof(feature)); file.GetString("unit", unit, sizeof(unit));
        offer.feature = StrEqual(feature, "trial") ? -1 : PremiumFeatureIndex(feature);
        if (!ShopIdValid(offer.id) || !offer.name[0] || strlen(offer.name) >= sizeof(offer.name) - 1
            || !ReadShopInt(file, "enabled", 0, 1, enabled) || !ReadShopInt(file, "price", 1, 1000000, offer.price)
            || !ReadShopInt(file, "duration", 1, 525600, amount) || (!StrEqual(feature, "trial") && offer.feature < 0)
            || !(offer.duration = EconomyDuration(amount, unit))) { valid = false; break; }
        if (!enabled) continue;
        for (int i; i < g_ShopCount; i++) if (StrEqual(offer.id, g_ShopOffers[i].id)) valid = false;
        if (!valid || g_ShopCount >= sizeof(g_ShopOffers)) { valid = false; break; }
        g_ShopOffers[g_ShopCount++] = offer;
    } while (file.GotoNextKey());
    delete file;
    g_ShopValid = valid;
    if (!valid) { g_ShopCount = 0; LogError("Invalid/missing Premium shop catalog: %s. Purchasing disabled; existing rentals and earning are separate.", path); }
    return valid;
}

bool EconomyAvailable(int client)
{
    if (!ValidClient(client) || IsFakeClient(client)) { Tell(client, "%t", "In Game Only"); return false; }
    if (!cvEnable.BoolValue || !cvPoints.BoolValue) { Tell(client, "%t", "Feature Disabled"); return false; }
    if (!g_EconomyReady || !g_Economy[client].loaded || !g_EconomyJournalHealthy || !EconomyBackendMatches())
    { Tell(client, "%t", "Points Pending"); if (!g_Economy[client].loading) LoadEconomyClient(client); return false; }
    return true;
}

bool ShopConfigured(int client, int feature)
{
    if (feature < 0) return cvPointsTrials.BoolValue;
    TFClassType class = TF2_GetPlayerClass(client);
    switch (view_as<PremiumFeature>(feature))
    {
        case Feature_Speed: return cvSpeed.BoolValue && EffectiveClassSpeed(client) > 1.0;
        case Feature_Gravity: return EffectiveGravity(client) != 1.0;
        case Feature_Health: return cvHealth.IntValue > 0;
        case Feature_Ammo: return EffectiveAmmoMultiplier(client) > 1.0;
        case Feature_KillHealth: return cvKillHealth.IntValue > 0;
        case Feature_AssistHealth: return cvAssistHealth.IntValue > 0;
        case Feature_Color: return cvColor.IntValue > 0;
        case Feature_Dark: return cvDark.BoolValue;
        case Feature_Glow: return cvGlow.BoolValue;
        case Feature_Fov: return cvFov.BoolValue;
        case Feature_Balls: return class == TFClass_Scout && cvBalls.IntValue > 1;
        case Feature_Jarate: return class == TFClass_Sniper && cvJarate.IntValue > 1;
        case Feature_Uber: return class == TFClass_Medic && cvUber.FloatValue > 0.0;
        case Feature_Respawn: return cvRespawn.BoolValue;
        case Feature_Swap: return cvSwap.BoolValue;
        case Feature_Cloak: return class == TFClass_Spy && cvCloak.BoolValue && cvCloakMultiplier.FloatValue > 1.0;
        case Feature_DeadRinger: return class == TFClass_Spy && cvCloak.BoolValue && cvDeadRingerMultiplier.FloatValue > 1.0;
        case Feature_Metal: return class == TFClass_Engineer && cvMetal.IntValue > 200;
        case Feature_Sentry: return class == TFClass_Engineer && cvSentryLimit.IntValue > 1 && cvTeamSentries.IntValue > 0 && !GameRules_GetProp("m_bPlayingMannVsMachine") && GameRules_GetProp("m_nGameType") != 4;
        case Feature_Dispenser: return class == TFClass_Engineer && cvDispenserLimit.IntValue > 1 && !GameRules_GetProp("m_bPlayingMannVsMachine") && GameRules_GetProp("m_nGameType") != 4;
        case Feature_Teleporter: return class == TFClass_Engineer && (cvEntranceLimit.IntValue > 1 || cvExitLimit.IntValue > 1) && !GameRules_GetProp("m_bPlayingMannVsMachine") && GameRules_GetProp("m_nGameType") != 4;
        case Feature_FireArrows: return class == TFClass_Sniper && cvArrows.BoolValue;
        case Feature_Bhop: return ConfiguredBhop(client);
        case Feature_DoubleJump: return ConfiguredBhop(client) && ConfiguredDoubleJump(client) && (ResolveFeatureMask(client) & PremiumFeatureBit(Feature_Bhop)) != 0;
        case Feature_Flight: return class == TFClass_Pyro && ConfiguredFlight(client);
        case Feature_Airblast: return class == TFClass_Pyro && ConfiguredAirblast(client);
        case Feature_Teleport: return ConfiguredTeleport(client);
        case Feature_Celebration: return cvCelebration.BoolValue;
        case Feature_FakeKill: return cvFakeKill.BoolValue;
    }
    return false;
}

bool ShopCanBuy(int client, int index, char[] reason, int size)
{
    if (!cvPointsShop.BoolValue || !g_ShopValid || index < 0 || index >= g_ShopCount)
    { strcopy(reason, size, "The points shop is disabled or this offer is unavailable."); return false; }
    if (g_Economy[client].purchasing) { strcopy(reason, size, "Your previous purchase is still settling."); return false; }
    if (!g_Player[client].loaded) { strcopy(reason, size, "Your membership record is still loading; nothing can be purchased yet."); return false; }
    ShopOffer offer; offer = g_ShopOffers[index];
    if (!ShopConfigured(client, offer.feature)) { strcopy(reason, size, "This feature is disabled for your current configuration, map or class."); return false; }
    if (offer.feature < 0)
    {
        if (HasPremium(client)) { strcopy(reason, size, "You already have active Premium/trial membership. Wait until it expires."); return false; }
        if (!EconomyCanBuyTrial(g_Economy[client].points, offer.price, g_Economy[client].trialUntil, GetTime(), g_Economy[client].trialPurchases, cvPointsTrialLimit.IntValue))
        { strcopy(reason, size, "Not enough points, or your lifetime purchased-trial limit has been reached."); return false; }
    }
    else
    {
        char overrideName[64]; FormatEx(overrideName, sizeof(overrideName), "premium_feature_%s", g_FeatureKeys[offer.feature]);
        if (!CheckCommandAccess(client, overrideName, 0, true)) { strcopy(reason, size, "Your permissions do not allow this feature."); return false; }
        if ((PremiumGameplayFeatures() & (1 << offer.feature)) && !PremiumTeamAllowed(GetClientTeam(client), cvGameplayTeams.IntValue))
        { strcopy(reason, size, "Gameplay features are disabled for your current team."); return false; }
        if (!cvPointsRentals.BoolValue) { strcopy(reason, size, "Purchased individual feature access is disabled."); return false; }
        if (ResolveBaseFeatureMask(client) & (1 << offer.feature))
        { strcopy(reason, size, "You already have this feature through membership, a free grant or a permission override."); return false; }
        if (!EconomyCanRent(g_Economy[client].points, offer.price, g_Economy[client].rentalUntil[offer.feature], GetTime(), offer.duration, cvPointsRenew.BoolValue, cvPointsMaxDays.IntValue))
        { strcopy(reason, size, "Not enough points, renewals are disabled, or the maximum rental duration would be exceeded."); return false; }
    }
    reason[0] = '\0'; return true;
}

void DisplayPointsShop(int client)
{
    UiShopHome(client);
}

bool ShopMenuItem(Menu menu, int selection, int &index)
{
    char data[64], parts[2][32]; menu.GetItem(selection, data, sizeof(data));
    if (ExplodeString(data, ":", parts, sizeof(parts), sizeof(parts[])) != 2 || !PremiumDigits(parts[0]) || !PremiumDigits(parts[1])
        || StringToInt(parts[0]) != g_ShopRevision) return false;
    index = StringToInt(parts[1]); return index >= 0 && index < g_ShopCount;
}
public int PointsShopSelected(Menu menu, MenuAction action, int client, int selection)
{
    if (action == MenuAction_End) delete menu;
    else if (action == MenuAction_Cancel && selection == MenuCancel_ExitBack && ValidClient(client)) UiShopHome(client);
    else if (action == MenuAction_Select && EconomyAvailable(client))
    {
        int index; char reason[192], time[32];
        if (!ShopMenuItem(menu, selection, index)) { Tell(client, "The shop changed. Please choose the offer again."); UiShopHome(client); return 0; }
        bool available = ShopCanBuy(client, index, reason, sizeof(reason));
        PremiumUiDuration(g_ShopOffers[index].duration, time, sizeof(time));
        Menu confirm = new Menu(PointsPurchaseSelected);
        confirm.SetTitle("%s\nPrice: %d points | Length: %s\nYour balance: %d points\nOffline time counts\n%s\n ",
            g_ShopOffers[index].name, g_ShopOffers[index].price, time, g_Economy[client].points,
            g_ShopOffers[index].feature < 0 ? "Full Premium package + Trial tag" : "This feature only; no Premium tag");
        char data[64]; FormatEx(data, sizeof(data), "%d:%d", g_ShopRevision, index);
        if (!available) confirm.AddItem("info", reason, ITEMDRAW_DISABLED);
        else { char after[64]; FormatEx(after, sizeof(after), "Balance afterwards: %d points", g_Economy[client].points - g_ShopOffers[index].price); confirm.AddItem("info", after, ITEMDRAW_DISABLED); }
        confirm.AddItem(data, "Confirm purchase", available ? ITEMDRAW_DEFAULT : ITEMDRAW_DISABLED);
        confirm.AddItem("cancel", "Back to shop"); confirm.ExitBackButton = true; confirm.Display(client, 30);
    }
    return 0;
}
public int PointsPurchaseSelected(Menu menu, MenuAction action, int client, int selection)
{
    if (action == MenuAction_End) delete menu;
    else if (action == MenuAction_Cancel && selection == MenuCancel_ExitBack && ValidClient(client)) UiShopHome(client);
    else if (action == MenuAction_Select && EconomyAvailable(client))
    {
        char data[64]; menu.GetItem(selection, data, sizeof(data));
        if (StrEqual(data, "cancel")) { UiShopHome(client); return 0; }
        int index; char reason[192];
        if (!ShopMenuItem(menu, selection, index)) { Tell(client, "The shop changed. Nothing purchased; please choose again."); UiShopHome(client); return 0; }
        if (!ShopCanBuy(client, index, reason, sizeof(reason))) { Tell(client, "%s", reason); UiShopHome(client); return 0; }
        ShopOffer offer; offer = g_ShopOffers[index]; EconomyOperation op;
        NewEconomyOperation(op, offer.feature < 0 ? Economy_Trial : Economy_Rent, g_Economy[client].steam);
        op.feature = offer.feature; op.price = offer.price; op.duration = offer.duration; op.buyerSerial = GetClientSerial(client);
        if (!QueueEconomyOperation(op)) { Tell(client, "%t", "Points Pending"); return 0; }
        g_Economy[client].purchasing = true; Tell(client, "Saving your purchase... Check My purchases shortly."); ProcessEconomyOutbox();
        UiWallet(client);
    }
    return 0;
}

public Action CommandPointsShop(int client, int args) { DisplayPointsShop(client); return Plugin_Handled; }
public Action CommandPoints(int client, int args)
{
    if (!EconomyAvailable(client)) return Plugin_Handled;
    EconomyChat(client, "%t", "Points Balance", g_Economy[client].points);
    char limit[32]; if (cvPointsTrialLimit.IntValue == 0) strcopy(limit, sizeof(limit), "unlimited"); else IntToString(cvPointsTrialLimit.IntValue, limit, sizeof(limit));
    Tell(client, "Trial purchases: %d / %s; trial remaining: %d minutes. Use !premium_shop to browse.", g_Economy[client].trialPurchases,
        limit, PremiumRemaining(g_Economy[client].trialUntil, GetTime()) / 60);
    return Plugin_Handled;
}
public Action CommandShopReload(int client, int args)
{ bool valid = LoadShopCatalog(); Tell(client, "%s", valid ? "Shop catalog reloaded. Existing quotes are cancelled; accepted transactions retain their original terms." : "Invalid shop file; purchases disabled. Check the error log."); return Plugin_Handled; }

public Action CommandPointsAdjust(int client, int args)
{
    char target[64], raw[32], steam[STEAM_LENGTH]; GetCmdArg(1, target, sizeof(target)); GetCmdArg(2, raw, sizeof(raw));
    bool negative = raw[0] == '-'; int amount = StringToInt(raw);
    if (args != 2 || strlen(raw) > 8 || !PremiumDigits(raw[negative ? 1 : 0]) || amount == 0 || amount < -1000000 || amount > 1000000)
    { Tell(client, "Usage: sm_premium_points_adjust <player|Steam ID> <-1000000..1000000>"); return Plugin_Handled; }
    if (!cvPoints.BoolValue || !g_EconomyReady) { Tell(client, "%t", "Points Pending"); return Plugin_Handled; }
    if (!PremiumNormalizeSteam(target, steam, sizeof(steam)))
    {
        int recipient = FindTarget(client, target, true, false); if (recipient <= 0) return Plugin_Handled;
        char auth[STEAM_LENGTH]; if (!GetClientAuthId(recipient, AuthId_Steam3, auth, sizeof(auth)) || !PremiumNormalizeSteam(auth, steam, sizeof(steam))) return Plugin_Handled;
    }
    EconomyOperation op; NewEconomyOperation(op, Economy_Adjust, steam); op.seconds = amount; op.actorSerial = ActorSerial(client);
    if (!QueueEconomyOperation(op)) Tell(client, "%t", "Points Pending"); else { Tell(client, "Points adjustment queued."); ProcessEconomyOutbox(); }
    return Plugin_Handled;
}

void FlushEconomyTime(int client)
{
    int seconds = RoundToFloor(g_Economy[client].pendingSeconds);
    if (!g_Economy[client].steam[0] || seconds < 1 || !g_EconomyOutbox || !g_Backend[0]) return;
    EconomyOperation op; NewEconomyOperation(op, Economy_Earn, g_Economy[client].steam); op.seconds = seconds > 1000000 ? 1000000 : seconds;
    if (QueueEconomyOperation(op)) g_Economy[client].pendingSeconds -= float(op.seconds);
}
void FlushEconomyClients()
{
    for (int client = 1; client <= MaxClients; client++) FlushEconomyTime(client);
    ProcessEconomyOutbox();
}
void EconomyInput(int client, int buttons, const float movement[3])
{
    if (!ValidClient(client) || cvPoints == null || !cvPoints.BoolValue) return;
    if (buttons != g_Economy[client].buttons || movement[0] != 0.0 || movement[1] != 0.0 || (buttons & (IN_ATTACK | IN_ATTACK2 | IN_JUMP))) g_Economy[client].lastInput = GetEngineTime();
    g_Economy[client].buttons = buttons;
}
bool EconomyEarnPending(const char[] steam)
{
    for (int i; i < g_EconomyOutbox.Length; i++)
    { EconomyOperation op; g_EconomyOutbox.GetArray(i, op, sizeof(op)); if (op.kind == Economy_Earn && StrEqual(op.steam, steam)) return true; }
    return false;
}
void EconomyHeartbeat()
{
    float clock = GetEngineTime();
    if (!g_EconomyJournalHealthy && !g_EconomyOutboxCorrupt && clock >= g_EconomyRetry)
    { g_EconomyJournalHealthy = SaveEconomyOutbox(); g_EconomyRetry = clock + 30.0; }
    if (!g_EconomyReady && clock >= g_EconomyRetry) InitializeEconomyStorage();
    if (!cvPoints.BoolValue || !cvEnable.BoolValue)
    { for (int client = 1; client <= MaxClients; client++) g_Economy[client].lastClock = clock; ProcessEconomyOutbox(); return; }
    int humans;
    for (int client = 1; client <= MaxClients; client++) if (ValidClient(client) && !IsFakeClient(client) && (cvPointsSpectators.BoolValue || GetClientTeam(client) >= 2)) humans++;
    for (int client = 1; client <= MaxClients; client++)
    {
        if (!ValidClient(client) || IsFakeClient(client)) continue;
        if (!g_Economy[client].loaded && !g_Economy[client].loading && g_EconomyReady) LoadEconomyClient(client);
        float elapsed = clock - g_Economy[client].lastClock; g_Economy[client].lastClock = clock;
        if (elapsed < 0.0) elapsed = 0.0; if (elapsed > 1.0) elapsed = 1.0; // No offline/stall catch-up.
        bool eligible = EconomyEarnEligible(cvPointsEarn.BoolValue && g_EconomyReady && g_EconomyJournalHealthy && EconomyBackendMatches(), g_Economy[client].loaded, true,
            cvPointsSpectators.BoolValue || GetClientTeam(client) >= 2, IsPlayerAlive(client), cvPointsDead.BoolValue,
            humans >= cvPointsMinPlayers.IntValue, RoundToFloor(clock - g_Economy[client].lastInput), cvPointsAfk.IntValue, g_Economy[client].points, cvPointsCap.IntValue);
        if (eligible) g_Economy[client].pendingSeconds += elapsed;
        if (eligible && g_Economy[client].pendingSeconds + float(g_Economy[client].progress) >= float(cvPointsMinutes.IntValue * 60) && !EconomyEarnPending(g_Economy[client].steam)) FlushEconomyTime(client);
        int rentals = EconomyRentalMask(client), ended = g_Economy[client].rentalNotices & ~rentals;
        bool trial = EconomyTrialActive(client);
        if (cvPointsExpiryNotice.BoolValue && !g_Options[client].quiet && g_EconomyReady && g_Economy[client].loaded)
        {
            for (int i; i < view_as<int>(Feature_Count); i++) if ((ended & (1 << i)) && g_Economy[client].rentalUntil[i] <= GetTime()) EconomyChat(client, "%t", "Rental Ended", g_FeatureKeys[i]);
            if (g_Economy[client].trialNotice && !trial && g_Economy[client].trialUntil <= GetTime()) EconomyChat(client, "%t", "Purchased Trial Ended");
        }
        g_Economy[client].rentalNotices = rentals; g_Economy[client].trialNotice = trial;
    }
    if (clock >= g_EconomyRetry) ProcessEconomyOutbox();
}
