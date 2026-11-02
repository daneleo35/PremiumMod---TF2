#include "economy_queries.inc"

void EconomyStorageReset()
{
    g_EconomyEpoch++; g_EconomyReady = false; g_EconomySchemaLoading = false; g_EconomySending = false;
    for (int client = 1; client <= MaxClients; client++)
    { g_Economy[client].loaded = false; g_Economy[client].loading = false; g_Economy[client].epoch++; g_Economy[client].revision = 0; }
}
void EconomyStorageError(const char[] error)
{
    LogError("Points storage paused: %s. Retrying in 30 seconds; durable receipt tokens are retained.", error);
    EconomyStorageReset(); g_EconomyRetry = GetEngineTime() + 30.0;
    RefreshEverybody();
}

void InitializeEconomyStorage()
{
    if (!g_DatabaseReady || g_EconomyReady || g_EconomySchemaLoading || !cvPoints.BoolValue || !g_EconomyJournalHealthy) return;
    if (!g_ShopValid) LoadShopCatalog();
    Transaction schema = new Transaction(); char query[1024];
    strcopy(query, sizeof(query), SQL_ECON_WALLET); if (g_MySQL) StrCat(query, sizeof(query), " ENGINE=InnoDB DEFAULT CHARSET=utf8mb4"); schema.AddQuery(query);
    strcopy(query, sizeof(query), SQL_ECON_RENTALS); if (g_MySQL) StrCat(query, sizeof(query), " ENGINE=InnoDB DEFAULT CHARSET=utf8mb4"); schema.AddQuery(query);
    strcopy(query, sizeof(query), SQL_ECON_RECEIPTS); if (g_MySQL) StrCat(query, sizeof(query), " ENGINE=InnoDB DEFAULT CHARSET=utf8mb4"); schema.AddQuery(query);
    DataPack context = new DataPack(); context.WriteCell(g_Generation); context.WriteCell(g_EconomyEpoch);
    g_EconomySchemaLoading = true; g_Database.Execute(schema, EconomySchemaReady, EconomySchemaFailed, context);
}
bool EconomyContext(Database db, DataPack context)
{
    int generation = context.ReadCell(), epoch = context.ReadCell();
    return CurrentDatabase(db, generation) && epoch == g_EconomyEpoch;
}
public void EconomySchemaReady(Database db, any data, int count, DBResultSet[] results, any[] queryData)
{
    DataPack context = view_as<DataPack>(data); context.Reset(); bool current = EconomyContext(db, context); delete context;
    if (!current) return;
    g_EconomySchemaLoading = false; g_EconomyReady = true;
    for (int client = 1; client <= MaxClients; client++) if (ValidClient(client)) LoadEconomyClient(client);
    ProcessEconomyOutbox();
}
public void EconomySchemaFailed(Database db, any data, int count, const char[] error, int failIndex, any[] queryData)
{
    DataPack context = view_as<DataPack>(data); context.Reset(); bool current = EconomyContext(db, context); delete context;
    if (current) EconomyStorageError(error);
}

void LoadEconomyClient(int client)
{
    if (!cvPoints.BoolValue || !g_EconomyReady || !ValidClient(client) || IsFakeClient(client)
        || g_Economy[client].loading || !IsClientAuthorized(client)) return;
    char steam[STEAM_LENGTH]; if (!GetClientAuthId(client, AuthId_Steam3, steam, sizeof(steam))
        || !PremiumNormalizeSteam(steam, g_Economy[client].steam, STEAM_LENGTH)) return;
    g_Economy[client].epoch++; g_Economy[client].loading = true;
    Transaction load = new Transaction(); char query[512];
    g_Database.Format(query, sizeof(query), "INSERT %s INTO premium_v3_wallets (steamid) VALUES ('%s')", g_MySQL ? "IGNORE" : "OR IGNORE", g_Economy[client].steam); load.AddQuery(query);
    g_Database.Format(query, sizeof(query), "SELECT points, play_seconds, trial_until, trial_purchases, revision FROM premium_v3_wallets WHERE steamid = '%s'", g_Economy[client].steam); load.AddQuery(query);
    g_Database.Format(query, sizeof(query), "SELECT feature, expires FROM premium_v3_rentals WHERE steamid = '%s' AND expires > %d", g_Economy[client].steam, GetTime()); load.AddQuery(query);
    DataPack context = new DataPack(); context.WriteCell(g_Generation); context.WriteCell(g_EconomyEpoch);
    context.WriteCell(GetClientSerial(client)); context.WriteCell(g_Economy[client].epoch);
    g_Database.Execute(load, EconomyClientLoaded, EconomyClientLoadFailed, context);
}
public void EconomyClientLoaded(Database db, any data, int count, DBResultSet[] results, any[] queryData)
{
    DataPack context = view_as<DataPack>(data); context.Reset(); bool current = EconomyContext(db, context);
    int client = GetClientFromSerial(context.ReadCell()), epoch = context.ReadCell(); delete context;
    if (!current || !ValidClient(client) || epoch != g_Economy[client].epoch) return;
    g_Economy[client].loading = false;
    if (count != 3 || !results[1].FetchRow()) { EconomyStorageError("Wallet row disappeared during load."); return; }
    int revision = results[1].FetchInt(4);
    if (revision < g_Economy[client].revision) { LoadEconomyClient(client); return; }
    g_Economy[client].points = results[1].FetchInt(0); g_Economy[client].progress = results[1].FetchInt(1);
    g_Economy[client].trialUntil = results[1].FetchInt(2); g_Economy[client].trialPurchases = results[1].FetchInt(3); g_Economy[client].revision = revision;
    if (g_Economy[client].points < 0 || g_Economy[client].points > 1000000000 || g_Economy[client].progress < 0 || g_Economy[client].trialPurchases < 0)
    { EconomyStorageError("Invalid wallet values; inspect the database before retrying."); return; }
    for (int i; i < view_as<int>(Feature_Count); i++) g_Economy[client].rentalUntil[i] = 0;
    while (results[2].FetchRow())
    { char feature[32]; results[2].FetchString(0, feature, sizeof(feature)); int index = PremiumFeatureIndex(feature); if (index >= 0) g_Economy[client].rentalUntil[index] = results[2].FetchInt(1); }
    g_Economy[client].loaded = true; g_Economy[client].lastClock = GetEngineTime(); g_Economy[client].lastInput = GetEngineTime();
    g_Economy[client].rentalNotices = EconomyRentalMask(client); g_Economy[client].trialNotice = EconomyTrialActive(client);
    RefreshClient(client);
}
public void EconomyClientLoadFailed(Database db, any data, int count, const char[] error, int failIndex, any[] queryData)
{
    DataPack context = view_as<DataPack>(data); context.Reset(); bool current = EconomyContext(db, context); delete context;
    if (current) EconomyStorageError(error);
}

void ProcessEconomyOutbox()
{
    if (g_Stopping || !g_DatabaseReady || !g_EconomyReady || g_EconomySending || !g_EconomyJournalHealthy || !g_EconomyOutbox.Length) return;
    EconomyOperation op; g_EconomyOutbox.GetArray(0, op, sizeof(op));
    if (!StrEqual(op.backend, g_Backend))
    { LogError("Points outbox belongs to backend %s, current backend is %s. Queue preserved; restore the original backend to settle it.", op.backend, g_Backend); g_EconomyRetry = GetEngineTime() + 30.0; return; }
    Transaction transaction = new Transaction(); AddEconomyQueries(transaction, op);
    DataPack context = new DataPack(); context.WriteCell(g_Generation); context.WriteCell(g_EconomyEpoch); context.WriteString(op.token);
    g_EconomySending = true; g_Database.Execute(transaction, EconomyCommitted, EconomyCommitFailed, context);
}
public void EconomyCommitted(Database db, any data, int count, DBResultSet[] results, any[] queryData)
{
    DataPack context = view_as<DataPack>(data); context.Reset(); bool current = EconomyContext(db, context);
    char token[33]; context.ReadString(token, sizeof(token)); delete context;
    if (!current || !g_EconomyOutbox.Length) return;
    EconomyOperation op; g_EconomyOutbox.GetArray(0, op, sizeof(op)); if (!StrEqual(token, op.token)) return;
    g_EconomySending = false;
    if (!results[count - 1].FetchRow()) { EconomyStorageError("Missing settlement wallet row."); return; }
    DBResultSet result = results[count - 1];
    int points = result.FetchInt(0), progress = result.FetchInt(1), trial = result.FetchInt(2), purchases = result.FetchInt(3), revision = result.FetchInt(4);
    int expiry = result.FetchInt(5), delta = result.FetchInt(6); bool committed = result.FetchInt(7) == 1;
    g_EconomyOutbox.Erase(0);
    if (!SaveEconomyOutbox()) { g_EconomyJournalHealthy = false; g_EconomyRetry = GetEngineTime() + 30.0; LogError("Points committed but outbox cleanup could not be saved; replay remains safe through its receipt token."); }
    for (int client = 1; client <= MaxClients; client++)
    {
        if (!ValidClient(client) || !StrEqual(g_Economy[client].steam, op.steam)) continue;
        if (revision >= g_Economy[client].revision)
        {
            g_Economy[client].points = points; g_Economy[client].progress = progress; g_Economy[client].revision = revision;
            g_Economy[client].trialUntil = trial; g_Economy[client].trialPurchases = purchases;
            if (op.kind == Economy_Rent && committed) g_Economy[client].rentalUntil[op.feature] = expiry;
        }
        if (GetClientSerial(client) == op.buyerSerial) g_Economy[client].purchasing = false;
        if (committed && op.kind == Economy_Earn && delta > 0 && cvPoints.BoolValue && cvEnable.BoolValue && cvPointsEarnNotice.BoolValue && !g_Options[client].quiet)
            EconomyChat(client, "%t", "Points Earned", delta, points);
        if (op.kind == Economy_Rent || op.kind == Economy_Trial || op.kind == Economy_Gift)
        {
            if (committed && op.kind == Economy_Gift && cvPointsBuyNotice.BoolValue) EconomyChat(client, "Gift saved: %s for %s. Your balance: %d points.", g_FeatureKeys[op.feature], op.recipient, points);
            else if (committed && cvPoints.BoolValue && cvEnable.BoolValue && cvPointsBuyNotice.BoolValue) EconomyChat(client, "%t", "Shop Purchased", op.kind == Economy_Trial ? "Premium trial" : g_FeatureKeys[op.feature], points);
            else if (!committed) EconomyChat(client, "%t", "Shop Declined");
        }
        if (op.kind == Economy_Transfer)
        {
            if (committed) EconomyChat(client, "Sent %d points to %s. Your balance: %d.", op.price, op.recipient, points);
            else EconomyChat(client, "Points gift declined: balance, recipient capacity or daily limit changed. Nothing was transferred.");
        }
        if (committed && op.kind == Economy_Rent) g_Economy[client].rentalNotices |= PremiumFeatureBit(view_as<PremiumFeature>(op.feature));
        if (committed && op.kind == Economy_Trial) g_Economy[client].trialNotice = true;
        RefreshClient(client);
    }
    if (committed && (op.kind == Economy_Gift || op.kind == Economy_Transfer))
    {
        for (int recipient = 1; recipient <= MaxClients; recipient++)
        {
            if (!ValidClient(recipient) || !StrEqual(g_Economy[recipient].steam, op.recipient)) continue;
            // Cancel any pre-commit load so it cannot overwrite the new grant.
            g_Economy[recipient].epoch++; g_Economy[recipient].loading = false;
            if (op.kind == Economy_Gift)
            {
                g_Economy[recipient].rentalUntil[op.feature] = expiry;
                g_Economy[recipient].rentalNotices |= 1 << op.feature;
            }
            // Keep existing rentals available while the wallet reloads. Marking
            // the account unloaded here would briefly revoke unrelated perks.
            LoadEconomyClient(recipient); RefreshClient(recipient);
            if (op.kind == Economy_Transfer && !g_Options[recipient].quiet) EconomyChat(recipient, "You received %d points from %s.", op.price, op.steam);
            else if (cvPointsBuyNotice.BoolValue && !g_Options[recipient].quiet)
                EconomyChat(recipient, "You received a %s rental from %s. Use !premium to view it.", g_FeatureKeys[op.feature], op.steam);
        }
    }
    int actor = ResolveActor(op.actorSerial);
    if (op.kind == Economy_Adjust && actor >= 0) Tell(actor, "Points adjustment settled for %s: change %d, balance %d.", op.steam, delta, points);
    ProcessEconomyOutbox();
}
public void EconomyCommitFailed(Database db, any data, int count, const char[] error, int failIndex, any[] queryData)
{
    DataPack context = view_as<DataPack>(data); context.Reset(); bool current = EconomyContext(db, context); delete context;
    if (current) EconomyStorageError(error); // Keep the same durable token for retry.
}
