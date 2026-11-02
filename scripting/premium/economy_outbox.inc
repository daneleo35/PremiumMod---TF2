bool SaveEconomyOutbox()
{
    char path[PLATFORM_MAX_PATH], stage[PLATFORM_MAX_PATH], backup[PLATFORM_MAX_PATH];
    BuildPath(Path_SM, path, sizeof(path), "data/premium_points_outbox.cfg");
    FormatEx(stage, sizeof(stage), "%s.new", path); FormatEx(backup, sizeof(backup), "%s.bak", path);
    KeyValues file = new KeyValues("PremiumOutbox");
    for (int i; i < g_EconomyOutbox.Length; i++)
    {
        EconomyOperation op; g_EconomyOutbox.GetArray(i, op, sizeof(op)); file.JumpToKey(op.token, true);
        file.SetString("steam", op.steam); file.SetString("backend", op.backend); file.SetNum("kind", view_as<int>(op.kind));
        file.SetString("recipient", op.recipient);
        file.SetNum("transfer_daily", op.transferDaily);
        file.SetString("feature", op.feature < 0 ? "trial" : g_FeatureKeys[op.feature]);
        file.SetNum("seconds", op.seconds); file.SetNum("interval", op.interval); file.SetNum("reward", op.reward);
        file.SetNum("cap", op.cap); file.SetNum("price", op.price); file.SetNum("duration", op.duration);
        file.SetNum("max_days", op.maxDays); file.SetNum("trial_limit", op.trialLimit); file.SetNum("renew", op.renew);
        file.GoBack();
    }
    file.Rewind(); bool saved = file.ExportToFile(stage); delete file;
    if (!saved) return false;
    bool existed = FileExists(path);
    if (existed && ((FileExists(backup) && !DeleteFile(backup)) || !RenameFile(backup, path))) { DeleteFile(stage); return false; }
    if (RenameFile(path, stage)) return true;
    if (existed && !RenameFile(path, backup)) LogError("Points outbox recovery required: %s", backup);
    return false;
}

void InitializeEconomyOutbox()
{
    g_EconomyOutbox = new ArrayList(sizeof(EconomyOperation));
    char path[PLATFORM_MAX_PATH], source[PLATFORM_MAX_PATH]; BuildPath(Path_SM, path, sizeof(path), "data/premium_points_outbox.cfg");
    strcopy(source, sizeof(source), path);
    if (!FileExists(source))
    {
        FormatEx(source, sizeof(source), "%s.new", path);
        if (!FileExists(source)) { FormatEx(source, sizeof(source), "%s.bak", path); if (!FileExists(source)) return; }
    }
    KeyValues file = new KeyValues("PremiumOutbox"); bool valid = file.ImportFromFile(source);
    char root[64]; file.GetSectionName(root, sizeof(root)); valid = valid && StrEqual(root, "PremiumOutbox");
    if (valid && file.GotoFirstSubKey()) do
    {
        EconomyOperation op; char steam[STEAM_LENGTH], feature[64];
        file.GetSectionName(op.token, sizeof(op.token)); file.GetString("steam", steam, sizeof(steam));
        file.GetString("backend", op.backend, sizeof(op.backend)); file.GetString("feature", feature, sizeof(feature));
        op.feature = StrEqual(feature, "trial") ? -1 : PremiumFeatureIndex(feature); op.kind = view_as<EconomyKind>(file.GetNum("kind", -1));
        file.GetString("recipient", op.recipient, sizeof(op.recipient));
        op.transferDaily = file.GetNum("transfer_daily");
        op.seconds = file.GetNum("seconds"); op.interval = file.GetNum("interval"); op.reward = file.GetNum("reward");
        op.cap = file.GetNum("cap"); op.price = file.GetNum("price"); op.duration = file.GetNum("duration");
        op.maxDays = file.GetNum("max_days"); op.trialLimit = file.GetNum("trial_limit"); op.renew = file.GetNum("renew") != 0;
        if (g_EconomyOutbox.Length >= 4096 || !PremiumNormalizeSteam(steam, op.steam, sizeof(op.steam)) || !EconomyOperationValid(op)) { valid = false; break; }
        g_EconomyOutbox.PushArray(op, sizeof(op));
    } while (file.GotoNextKey());
    delete file;
    if (!valid)
    { g_EconomyOutbox.Clear(); g_EconomyJournalHealthy = false; g_EconomyOutboxCorrupt = true; LogError("Invalid points outbox %s. Economy is blocked; preserve this file and recover its backup before reloading.", source); }
    else if (!StrEqual(source, path))
    { LogMessage("Recovered points outbox from %s; receipt tokens prevent repeat debits/rewards.", source); g_EconomyJournalHealthy = SaveEconomyOutbox(); }
}

void NewEconomyOperation(EconomyOperation op, EconomyKind kind, const char[] steam)
{
    FormatEx(op.token, sizeof(op.token), "%08x%08x%08x%08x", GetURandomInt(), GetURandomInt(), GetURandomInt(), GetURandomInt());
    strcopy(op.steam, sizeof(op.steam), steam); strcopy(op.backend, sizeof(op.backend), g_Backend);
    op.kind = kind; op.feature = -1; op.interval = cvPointsMinutes.IntValue * 60; op.reward = cvPointsReward.IntValue;
    op.cap = cvPointsCap.IntValue; op.maxDays = cvPointsMaxDays.IntValue; op.trialLimit = cvPointsTrialLimit.IntValue; op.renew = cvPointsRenew.BoolValue;
}

bool QueueEconomyOperation(EconomyOperation op)
{
    if (!g_EconomyJournalHealthy || !EconomyBackendMatches() || g_EconomyOutbox.Length >= 4096 || !EconomyOperationValid(op)) return false;
    g_EconomyOutbox.PushArray(op, sizeof(op));
    if (!SaveEconomyOutbox())
    {
        // A .new snapshot might contain this accepted operation. Retain its
        // token in memory, block processing, and retry persistence first.
        g_EconomyJournalHealthy = false; g_EconomyRetry = GetEngineTime() + 30.0;
        LogError("Points outbox could not be saved; economy paused. Pending intents retained. Check data/ write permission.");
    }
    return true;
}

bool EconomyBackendMatches()
{
    if (!g_EconomyOutbox || !g_EconomyOutbox.Length) return true;
    EconomyOperation head; g_EconomyOutbox.GetArray(0, head, sizeof(head));
    return StrEqual(head.backend, g_Backend);
}
