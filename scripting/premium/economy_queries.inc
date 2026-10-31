// Every balance mutation and its receipt/rental are one DB transaction.
// The same durable token can be retried after an uncertain commit safely.
void AddEconomyQueries(Transaction transaction, EconomyOperation op)
{
    char query[4096], feature[32], balance[512], credit[256];
    strcopy(feature, sizeof(feature), op.feature < 0 ? "trial" : g_FeatureKeys[op.feature]);
    g_Database.Format(query, sizeof(query), "INSERT %s INTO premium_v3_wallets (steamid) VALUES ('%s')", g_MySQL ? "IGNORE" : "OR IGNORE", op.steam);
    transaction.AddQuery(query);
    int now = GetTime();
    if (op.kind == Economy_Rent)
    {
        g_Database.Format(query, sizeof(query), "INSERT %s INTO premium_v3_rentals (steamid, feature, expires) VALUES ('%s', '%s', 0)", g_MySQL ? "IGNORE" : "OR IGNORE", op.steam, feature);
        transaction.AddQuery(query);
        g_Database.Format(query, sizeof(query),
            "UPDATE premium_v3_wallets SET last_delta = -%d, points = points - %d, revision = revision + 1, last_token = '%s' WHERE steamid = '%s' AND points >= %d AND revision < 2147483647 AND NOT EXISTS (SELECT 1 FROM premium_v3_receipts WHERE token = '%s') AND (%d = 1 OR (SELECT expires FROM premium_v3_rentals WHERE steamid = '%s' AND feature = '%s') <= %d) AND (SELECT CASE WHEN expires > %d THEN expires ELSE %d END FROM premium_v3_rentals WHERE steamid = '%s' AND feature = '%s') <= %d",
            op.price, op.price, op.token, op.steam, op.price, op.token, op.renew, op.steam, feature, now, now, now, op.steam, feature, EconomyExpiryLimit(now, op.maxDays) - op.duration);
        transaction.AddQuery(query);
        g_Database.Format(query, sizeof(query),
            "UPDATE premium_v3_rentals SET expires = CASE WHEN expires > %d THEN expires ELSE %d END + %d WHERE steamid = '%s' AND feature = '%s' AND EXISTS (SELECT 1 FROM premium_v3_wallets WHERE steamid = '%s' AND last_token = '%s') AND NOT EXISTS (SELECT 1 FROM premium_v3_receipts WHERE token = '%s')",
            now, now, op.duration, op.steam, feature, op.steam, op.token, op.token);
        transaction.AddQuery(query);
        g_Database.Format(query, sizeof(query),
            "INSERT INTO premium_v3_receipts (token, steamid, kind, feature, point_change, expires) SELECT '%s', w.steamid, %d, '%s', w.last_delta, r.expires FROM premium_v3_wallets w JOIN premium_v3_rentals r ON r.steamid = w.steamid AND r.feature = '%s' WHERE w.steamid = '%s' AND w.last_token = '%s' AND NOT EXISTS (SELECT 1 FROM premium_v3_receipts WHERE token = '%s')",
            op.token, view_as<int>(op.kind), feature, feature, op.steam, op.token, op.token);
    }
    else
    {
        if (op.kind == Economy_Trial)
        {
            g_Database.Format(query, sizeof(query),
                "UPDATE premium_v3_wallets SET last_delta = -%d, points = points - %d, trial_until = %d, trial_purchases = trial_purchases + 1, revision = revision + 1, last_token = '%s' WHERE steamid = '%s' AND points >= %d AND trial_until <= %d AND %d = 1 AND trial_purchases < 2147483647 AND revision < 2147483647 AND (%d = 0 OR trial_purchases < %d) AND NOT EXISTS (SELECT 1 FROM premium_v3_receipts WHERE token = '%s')",
                op.price, op.price, now <= 2147483647 - op.duration ? now + op.duration : now, op.token, op.steam, op.price, now, now <= 2147483647 - op.duration, op.trialLimit, op.trialLimit, op.token);
        }
        else
        {
            if (op.kind == Economy_Earn)
            {
                if (g_MySQL) FormatEx(credit, sizeof(credit), "(((play_seconds + %d) DIV %d) * %d)", op.seconds, op.interval, op.reward);
                else FormatEx(credit, sizeof(credit), "(CAST((play_seconds + %d) / %d AS INTEGER) * %d)", op.seconds, op.interval, op.reward);
            }
            else IntToString(op.seconds, credit, sizeof(credit));
            if (op.kind == Economy_Adjust && op.seconds < 0)
                FormatEx(balance, sizeof(balance), "CASE WHEN points + (%s) < 0 THEN 0 ELSE points + (%s) END", credit, credit);
            else FormatEx(balance, sizeof(balance), "CASE WHEN points >= %d THEN points WHEN points + (%s) > %d THEN %d ELSE points + (%s) END", op.cap, credit, op.cap, op.cap, credit);
            if (op.kind == Economy_Earn)
                g_Database.Format(query, sizeof(query),
                    "UPDATE premium_v3_wallets SET last_delta = (%s) - points, points = (%s), play_seconds = (play_seconds + %d) %% %d, revision = revision + 1, last_token = '%s' WHERE steamid = '%s' AND revision < 2147483647 AND NOT EXISTS (SELECT 1 FROM premium_v3_receipts WHERE token = '%s')",
                    balance, balance, op.seconds, op.interval, op.token, op.steam, op.token);
            else g_Database.Format(query, sizeof(query),
                "UPDATE premium_v3_wallets SET last_delta = (%s) - points, points = (%s), revision = revision + 1, last_token = '%s' WHERE steamid = '%s' AND revision < 2147483647 AND NOT EXISTS (SELECT 1 FROM premium_v3_receipts WHERE token = '%s')",
                balance, balance, op.token, op.steam, op.token);
        }
        transaction.AddQuery(query);
        g_Database.Format(query, sizeof(query),
            "INSERT INTO premium_v3_receipts (token, steamid, kind, feature, point_change, expires) SELECT '%s', steamid, %d, '%s', last_delta, %s FROM premium_v3_wallets WHERE steamid = '%s' AND last_token = '%s' AND NOT EXISTS (SELECT 1 FROM premium_v3_receipts WHERE token = '%s')",
            op.token, view_as<int>(op.kind), feature, op.kind == Economy_Trial ? "trial_until" : "0", op.steam, op.token, op.token);
    }
    transaction.AddQuery(query);
    // A declined intent also gets a receipt. Otherwise an old journal could
    // replay a formerly declined purchase after the account earns more points.
    g_Database.Format(query, sizeof(query),
        "INSERT INTO premium_v3_receipts (token, steamid, kind, feature, point_change, expires, accepted) SELECT '%s', steamid, %d, '%s', 0, 0, 0 FROM premium_v3_wallets WHERE steamid = '%s' AND NOT EXISTS (SELECT 1 FROM premium_v3_receipts WHERE token = '%s')",
        op.token, view_as<int>(op.kind), feature, op.steam, op.token);
    transaction.AddQuery(query);
    g_Database.Format(query, sizeof(query),
        "SELECT points, play_seconds, trial_until, trial_purchases, revision, COALESCE((SELECT expires FROM premium_v3_rentals WHERE steamid = '%s' AND feature = '%s'), 0), COALESCE((SELECT point_change FROM premium_v3_receipts WHERE token = '%s'), 0), COALESCE((SELECT accepted FROM premium_v3_receipts WHERE token = '%s'), 0) FROM premium_v3_wallets WHERE steamid = '%s'",
        op.steam, feature, op.token, op.token, op.steam);
    transaction.AddQuery(query);
}
