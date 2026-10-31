#define SQL_CREATE_MEMBERS "CREATE TABLE IF NOT EXISTS premium_v3_members (steamid VARCHAR(32) NOT NULL PRIMARY KEY)"
#define SQL_CREATE_TRIALS "CREATE TABLE IF NOT EXISTS premium_v3_trials (steamid VARCHAR(32) NOT NULL PRIMARY KEY, used INTEGER NOT NULL DEFAULT 0, trial_until INTEGER NOT NULL DEFAULT 0)"
char g_Backend[64];

enum StorageOperation
{
    Storage_Add,
    Storage_Remove,
    Storage_GrantTrial,
    Storage_ResetTrial
}

void InvalidateStorageClients()
{
    EconomyStorageReset();
    for (int client = 1; client <= MaxClients; client++)
    {
        g_Player[client].revision++;
        g_Player[client].loaded = false;
        g_Player[client].member = false;
        g_Player[client].usedTrial = true; // Never assume eligibility before a successful read.
        g_Player[client].trialUntil = 0;
        g_Player[client].trialPending = false;
        if (ValidClient(client)) RefreshClient(client);
    }
}

void RestartDatabase(bool force = false)
{
    char desired[64];
    if (cvMySQL.BoolValue) cvDatabase.GetString(desired, sizeof(desired));
    else strcopy(desired, sizeof(desired), "storage-local");
    if (!force && StrEqual(desired, g_Backend) && (g_DatabaseReady || g_Connecting)) return;
    FlushEconomyClients();
    strcopy(g_Backend, sizeof(g_Backend), desired);
    g_Generation++;
    g_Connecting = false;
    g_DatabaseReady = false;
    delete g_Database;
    InvalidateStorageClients();
    ConnectDatabase();
}

void ConnectDatabase()
{
    if (g_Connecting || g_Stopping || !g_Backend[0]) return;
    g_Generation++;
    delete g_Database;
    g_Connecting = true;
    g_NextDatabaseRetry = GetEngineTime() + 30.0;
    Database.Connect(DatabaseConnected, g_Backend, g_Generation);
}

void StorageFailed(const char[] error)
{
    LogError("Premium database unavailable (%s): %s. Retrying in 30 seconds; flag members retain access.", g_Backend, error);
    g_DatabaseReady = false;
    g_Connecting = false;
    g_Generation++; // Reject every outstanding result from this connection.
    g_NextDatabaseRetry = GetEngineTime() + 30.0;
    InvalidateStorageClients();
}

bool CurrentDatabase(Database db, int generation)
{
    return !g_Stopping && generation == g_Generation && g_Database != null
        && db != null && g_Database.IsSameConnection(db);
}

public void DatabaseConnected(Database db, const char[] error, any generation)
{
    if (g_Stopping || generation != g_Generation)
    {
        delete db; // Connection callbacks transfer ownership of this handle.
        return;
    }
    if (db == null)
    {
        StorageFailed(error);
        return;
    }
    g_Database = db;
    char driver[16], query[512];
    db.Driver.GetIdentifier(driver, sizeof(driver));
    g_MySQL = StrEqual(driver, "mysql");
    if (!g_MySQL && !StrEqual(driver, "sqlite"))
    {
        StorageFailed("Only MySQL and SQLite are supported.");
        return;
    }
    strcopy(query, sizeof(query), SQL_CREATE_MEMBERS);
    if (g_MySQL) StrCat(query, sizeof(query), " ENGINE=InnoDB DEFAULT CHARSET=utf8mb4");
    db.Query(MembersSchemaReady, query, generation);
}

public void MembersSchemaReady(Database db, DBResultSet results, const char[] error, any generation)
{
    if (!CurrentDatabase(db, generation)) return;
    if (results == null)
    {
        StorageFailed(error);
        return;
    }
    char query[512];
    strcopy(query, sizeof(query), SQL_CREATE_TRIALS);
    if (g_MySQL) StrCat(query, sizeof(query), " ENGINE=InnoDB DEFAULT CHARSET=utf8mb4");
    g_Database.Query(TrialsSchemaReady, query, generation);
}

public void TrialsSchemaReady(Database db, DBResultSet results, const char[] error, any generation)
{
    if (!CurrentDatabase(db, generation)) return;
    if (results == null)
    {
        StorageFailed(error);
        return;
    }
    g_Connecting = false;
    g_DatabaseReady = true;
    InitializeEconomyStorage();
    LogMessage("Premium schema ready on %s (%s).", g_Backend, g_MySQL ? "MySQL" : "SQLite");
    for (int client = 1; client <= MaxClients; client++)
        if (ValidClient(client) && IsClientAuthorized(client) && !IsFakeClient(client)) LoadClientRecord(client);
    // Threaded query result/connection handles are borrowed, auto-closed by SourceMod.
}

DataPack ClientContext(int client)
{
    DataPack context = new DataPack();
    context.WriteCell(GetClientSerial(client));
    context.WriteCell(g_Generation);
    context.WriteCell(g_Player[client].revision);
    return context;
}

bool ReadClientContext(Database db, any data, int &client)
{
    DataPack context = view_as<DataPack>(data);
    context.Reset();
    int serial = context.ReadCell();
    int generation = context.ReadCell();
    int revision = context.ReadCell();
    delete context;
    client = GetClientFromSerial(serial);
    return CurrentDatabase(db, generation) && ValidClient(client)
        && g_Player[client].revision == revision;
}

void LoadClientRecord(int client)
{
    if (!g_DatabaseReady || !ValidClient(client) || IsFakeClient(client) || !IsClientAuthorized(client)) return;
    char auth[STEAM_LENGTH];
    if (!GetClientAuthId(client, AuthId_Steam3, auth, sizeof(auth))
        || !PremiumNormalizeSteam(auth, g_Player[client].steam, STEAM_LENGTH)) return;
    g_Player[client].revision++;
    char query[384];
    g_Database.Format(query, sizeof(query),
        "INSERT %s INTO premium_v3_trials (steamid, used, trial_until) VALUES ('%s', 0, 0)",
        g_MySQL ? "IGNORE" : "OR IGNORE", g_Player[client].steam);
    // SQL keywords are fixed internal strings, escaped formatting is safe for them too.
    g_Database.Query(ClientRowReady, query, ClientContext(client));
}

public void ClientRowReady(Database db, DBResultSet results, const char[] error, any data)
{
    int client;
    if (!ReadClientContext(db, data, client)) return;
    if (results == null)
    {
        StorageFailed(error);
        return;
    }
    char query[512];
    g_Database.Format(query, sizeof(query),
        "SELECT used, trial_until, (SELECT COUNT(*) FROM premium_v3_members WHERE steamid = '%s') FROM premium_v3_trials WHERE steamid = '%s'",
        g_Player[client].steam, g_Player[client].steam);
    g_Database.Query(ClientRecordLoaded, query, ClientContext(client));
}

public void ClientRecordLoaded(Database db, DBResultSet results, const char[] error, any data)
{
    int client;
    if (!ReadClientContext(db, data, client)) return;
    if (results == null)
    {
        StorageFailed(error);
        return;
    }
    if (!results.FetchRow())
    {
        StorageFailed("Trial identity row disappeared while loading.");
        return;
    }
    g_Player[client].usedTrial = results.FetchInt(0) != 0;
    g_Player[client].trialUntil = results.FetchInt(1);
    if (!PremiumRemaining(g_Player[client].trialUntil, GetTime())) g_Player[client].trialUntil = 0;
    g_Player[client].member = results.FetchInt(2) > 0;
    g_Player[client].trialPending = false;
    g_Player[client].loaded = true;
    RefreshClient(client);
}

void StartTrial(int client)
{
    if (!cvEnable.BoolValue || !cvTrial.BoolValue)
    {
        Tell(client, "%t", "Feature Disabled");
        return;
    }
    if (!g_DatabaseReady || !g_Player[client].loaded || g_Player[client].trialPending)
    {
        Tell(client, "%t", "Storage Pending");
        return;
    }
    if (g_Player[client].usedTrial)
    {
        Tell(client, "%t", "Trial Used");
        return;
    }
    int until = GetTime() + cvTrialSeconds.IntValue;
    char query[384];
    g_Database.Format(query, sizeof(query),
        "UPDATE premium_v3_trials SET used = 1, trial_until = %d WHERE steamid = '%s' AND used = 0", until, g_Player[client].steam);
    g_Player[client].trialPending = true;
    g_Player[client].revision++;
    g_Database.Query(TrialStarted, query, ClientContext(client));
}

public void TrialStarted(Database db, DBResultSet results, const char[] error, any data)
{
    int client;
    if (!ReadClientContext(db, data, client)) return;
    if (results == null)
    {
        StorageFailed(error);
        return;
    }
    if (results.AffectedRows == 1)
    {
        Tell(client, "%t", "Trial Activated", cvTrialSeconds.IntValue);
        // This read is queued after the atomic update. No entitlement is granted
        // until authoritative state has been successfully fetched.
        if (cvFeatures.BoolValue) DisplayFeatures(client);
    }
    else Tell(client, "%t", "Trial Used");
    LoadClientRecord(client);
}

void WriteMembership(int actor, const char[] steam, StorageOperation operation)
{
    if (!g_DatabaseReady)
    {
        Tell(actor, "%t", "Storage Pending");
        return;
    }
    char query[512];
    if (operation == Storage_Add)
        g_Database.Format(query, sizeof(query), "INSERT %s INTO premium_v3_members (steamid) VALUES ('%s')",
            g_MySQL ? "IGNORE" : "OR IGNORE", steam);
    else if (operation == Storage_Remove)
        g_Database.Format(query, sizeof(query), "DELETE FROM premium_v3_members WHERE steamid = '%s'", steam);
    else
        g_Database.Format(query, sizeof(query),
            "REPLACE INTO premium_v3_trials (steamid, used, trial_until) VALUES ('%s', %d, %d)",
            steam, operation == Storage_GrantTrial ? 1 : 0,
            operation == Storage_GrantTrial ? GetTime() + cvTrialSeconds.IntValue : 0);
    DataPack context = new DataPack();
    context.WriteCell(ActorSerial(actor));
    context.WriteCell(g_Generation);
    context.WriteString(steam);
    context.WriteCell(operation);
    g_Database.Query(MembershipWritten, query, context);
}

public void MembershipWritten(Database db, DBResultSet results, const char[] error, any data)
{
    DataPack context = view_as<DataPack>(data);
    context.Reset();
    int actor = ResolveActor(context.ReadCell());
    int generation = context.ReadCell();
    char steam[STEAM_LENGTH];
    context.ReadString(steam, sizeof(steam));
    StorageOperation operation = view_as<StorageOperation>(context.ReadCell());
    delete context;
    if (!CurrentDatabase(db, generation)) return;
    if (results == null)
    {
        Tell(actor, "%t", "Database Write Failed");
        StorageFailed(error);
        return;
    }
    Tell(actor, "%t", "Database Updated", steam, results.AffectedRows);
    LogAction(actor >= 0 ? actor : 0, -1, "Premium operation %d committed for %s", operation, steam);
    for (int client = 1; client <= MaxClients; client++)
    {
        if (!ValidClient(client) || !StrEqual(g_Player[client].steam, steam)) continue;
        // Make revocation immediate while the fresh authoritative read is pending.
        if (operation == Storage_Remove) g_Player[client].member = false;
        if (operation == Storage_ResetTrial) g_Player[client].trialUntil = 0;
        RefreshClient(client);
        LoadClientRecord(client);
    }
}

void ImportLegacy(int actor, int offset)
{
    if (!g_DatabaseReady)
    {
        Tell(actor, "%t", "Storage Pending");
        return;
    }
    char query[192];
    FormatEx(query, sizeof(query), "SELECT steamid FROM premium ORDER BY steamid LIMIT 100 OFFSET %d", offset);
    DataPack context = new DataPack();
    context.WriteCell(ActorSerial(actor));
    context.WriteCell(g_Generation);
    context.WriteCell(offset);
    g_Database.Query(LegacyRead, query, context);
}

public void LegacyRead(Database db, DBResultSet results, const char[] error, any data)
{
    DataPack context = view_as<DataPack>(data);
    context.Reset();
    int serial = context.ReadCell();
    int generation = context.ReadCell();
    int offset = context.ReadCell();
    delete context;
    if (!CurrentDatabase(db, generation)) return;
    if (results == null)
    {
        Tell(ResolveActor(serial), "%t", "Legacy Missing");
        LogError("Legacy premium import: %s", error);
        return; // A missing optional legacy table is not a backend outage.
    }
    Transaction batch = new Transaction();
    int count, invalid;
    while (results.FetchRow())
    {
        char input[64], steam[STEAM_LENGTH], query[256];
        results.FetchString(0, input, sizeof(input));
        if (!PremiumNormalizeSteam(input, steam, sizeof(steam)))
        {
            invalid++;
            continue;
        }
        g_Database.Format(query, sizeof(query), "INSERT %s INTO premium_v3_members (steamid) VALUES ('%s')",
            g_MySQL ? "IGNORE" : "OR IGNORE", steam);
        batch.AddQuery(query);
        count++;
    }
    if (!count)
    {
        delete batch;
        Tell(ResolveActor(serial), "%t", "Legacy Empty", offset, invalid);
        return;
    }
    DataPack completion = new DataPack();
    completion.WriteCell(serial);
    completion.WriteCell(generation);
    completion.WriteCell(offset + 100);
    completion.WriteCell(invalid);
    g_Database.Execute(batch, LegacyCommitted, LegacyFailed, completion);
}

public void LegacyCommitted(Database db, any data, int count, DBResultSet[] results, any[] queryData)
{
    DataPack context = view_as<DataPack>(data);
    context.Reset();
    int actor = ResolveActor(context.ReadCell());
    int generation = context.ReadCell();
    int next = context.ReadCell();
    int invalid = context.ReadCell();
    delete context;
    if (!CurrentDatabase(db, generation)) return;
    Tell(actor, "%t", "Legacy Imported", count, invalid, next);
    for (int client = 1; client <= MaxClients; client++)
        if (ValidClient(client)) LoadClientRecord(client);
}

public void LegacyFailed(Database db, any data, int count, const char[] error, int failIndex, any[] queryData)
{
    DataPack context = view_as<DataPack>(data);
    context.Reset();
    int actor = ResolveActor(context.ReadCell());
    int generation = context.ReadCell();
    delete context;
    if (!CurrentDatabase(db, generation)) return;
    Tell(actor, "%t", "Database Write Failed");
    LogError("Legacy import rolled back at query %d: %s", failIndex, error);
}
