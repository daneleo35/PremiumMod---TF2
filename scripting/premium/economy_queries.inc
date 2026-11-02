// Every balance mutation and its receipt/rental are one DB transaction.
// The same durable token can be retried after an uncertain commit safely.
void AddEconomyQueries(Transaction transaction, EconomyOperation op)
{
    char query[4096], feature[32], balance[512], credit[256], recipient[32];
    strcopy(recipient, sizeof(recipient), op.kind == Economy_Gift ? op.recipient : op.steam);
    strcopy(feature, sizeof(feature), op.feature < 0 ? "trial" : g_FeatureKeys[op.feature]);
    if (op.kind == Economy_Transfer) strcopy(feature, sizeof(feature), "points");
    g_Database.Format(query, sizeof(query), "INSERT %s INTO premium_v3_wallets (steamid) VALUES ('%s')", g_MySQL ? "IGNORE" : "OR IGNORE", op.steam);
    transaction.AddQuery(query);
    int now = GetTime();
    if (op.kind == Economy_Gift || op.kind == Economy_Transfer)
    {
        g_Database.Format(query, sizeof(query), "INSERT %s INTO premium_v3_wallets (steamid) VALUES ('%s')", g_MySQL ? "IGNORE" : "OR IGNORE", op.recipient); transaction.AddQuery(query);
        // Hold the beneficiary row through debit, grant/credit and receipt.
        // This serializes two different senders targeting the same wallet.
        g_Database.Format(query, sizeof(query), "UPDATE premium_v3_wallets SET revision = revision WHERE steamid = '%s'", op.recipient); transaction.AddQuery(query);
    }
    if (op.kind == Economy_Transfer)
    {
        g_Database.Format(query, sizeof(query), "UPDATE premium_v3_wallets SET points = points - %d, last_delta = -%d, last_token = '%s', revision = revision + 1 WHERE steamid = '%s' AND points >= %d AND revision < 2147483647 AND (SELECT points FROM premium_v3_wallets r WHERE r.steamid = '%s') <= %d AND (SELECT revision FROM premium_v3_wallets r WHERE r.steamid = '%s') < 2147483647 AND (%d = 0 OR COALESCE((SELECT SUM(-point_change) FROM premium_v3_receipts WHERE steamid = '%s' AND kind = %d AND accepted = 1 AND expires >= %d), 0) <= %d) AND NOT EXISTS (SELECT 1 FROM premium_v3_receipts WHERE token = '%s')", op.price, op.price, op.token, op.steam, op.price, op.recipient, op.cap - op.price, op.recipient, op.transferDaily, op.steam, view_as<int>(Economy_Transfer), now - (now % 86400), op.transferDaily - op.price, op.token);
        if (g_MySQL) g_Database.Format(query, sizeof(query), "UPDATE premium_v3_wallets b JOIN premium_v3_wallets r ON r.steamid = '%s' SET b.points = b.points - %d, b.last_delta = -%d, b.last_token = '%s', b.revision = b.revision + 1 WHERE b.steamid = '%s' AND b.points >= %d AND b.revision < 2147483647 AND r.points <= %d AND r.revision < 2147483647 AND (%d = 0 OR COALESCE((SELECT SUM(-point_change) FROM premium_v3_receipts WHERE steamid = '%s' AND kind = %d AND accepted = 1 AND expires >= %d), 0) <= %d) AND NOT EXISTS (SELECT 1 FROM premium_v3_receipts WHERE token = '%s')", op.recipient, op.price, op.price, op.token, op.steam, op.price, op.cap - op.price, op.transferDaily, op.steam, view_as<int>(Economy_Transfer), now - (now % 86400), op.transferDaily - op.price, op.token);
        transaction.AddQuery(query);
        g_Database.Format(query, sizeof(query), "UPDATE premium_v3_wallets SET points = points + %d, revision = revision + 1 WHERE steamid = '%s' AND EXISTS (SELECT 1 FROM premium_v3_wallets b WHERE b.steamid = '%s' AND b.last_token = '%s') AND NOT EXISTS (SELECT 1 FROM premium_v3_receipts WHERE token = '%s')", op.price, op.recipient, op.steam, op.token, op.token);
        if (g_MySQL) g_Database.Format(query, sizeof(query), "UPDATE premium_v3_wallets r JOIN premium_v3_wallets b ON b.steamid = '%s' SET r.points = r.points + %d, r.revision = r.revision + 1 WHERE r.steamid = '%s' AND b.last_token = '%s' AND NOT EXISTS (SELECT 1 FROM premium_v3_receipts WHERE token = '%s')", op.steam, op.price, op.recipient, op.token, op.token);
        transaction.AddQuery(query);
        g_Database.Format(query, sizeof(query), "INSERT INTO premium_v3_receipts (token, steamid, kind, feature, point_change, expires) SELECT '%s', steamid, %d, 'points', last_delta, %d FROM premium_v3_wallets WHERE steamid = '%s' AND last_token = '%s' AND NOT EXISTS (SELECT 1 FROM premium_v3_receipts WHERE token = '%s')", op.token, view_as<int>(op.kind), now, op.steam, op.token, op.token);
    }
    else if (op.kind == Economy_Rent || op.kind == Economy_Gift)
    {
        g_Database.Format(query, sizeof(query), "INSERT %s INTO premium_v3_rentals (steamid, feature, expires) VALUES ('%s', '%s', 0)", g_MySQL ? "IGNORE" : "OR IGNORE", recipient, feature);
        transaction.AddQuery(query);
        g_Database.Format(query, sizeof(query),
            "UPDATE premium_v3_wallets SET last_delta = -%d, points = points - %d, revision = revision + 1, last_token = '%s' WHERE steamid = '%s' AND points >= %d AND revision < 2147483647 AND NOT EXISTS (SELECT 1 FROM premium_v3_receipts WHERE token = '%s') AND (%d = 1 OR (SELECT expires FROM premium_v3_rentals WHERE steamid = '%s' AND feature = '%s') <= %d) AND (SELECT CASE WHEN expires > %d THEN expires ELSE %d END FROM premium_v3_rentals WHERE steamid = '%s' AND feature = '%s') <= %d",
            op.price, op.price, op.token, op.steam, op.price, op.token, op.renew, recipient, feature, now, now, now, recipient, feature, EconomyExpiryLimit(now, op.maxDays) - op.duration);
        transaction.AddQuery(query);
        g_Database.Format(query, sizeof(query),
            "UPDATE premium_v3_rentals SET expires = CASE WHEN expires > %d THEN expires ELSE %d END + %d WHERE steamid = '%s' AND feature = '%s' AND EXISTS (SELECT 1 FROM premium_v3_wallets WHERE steamid = '%s' AND last_token = '%s') AND NOT EXISTS (SELECT 1 FROM premium_v3_receipts WHERE token = '%s')",
            now, now, op.duration, recipient, feature, op.steam, op.token, op.token);
        transaction.AddQuery(query);
        if (op.kind == Economy_Gift)
        {
            // Invalidate older recipient loads without changing their balance.
            // Receipt absence makes both the grant and revision bump replay-safe.
            g_Database.Format(query, sizeof(query), "UPDATE premium_v3_wallets SET revision = revision + 1 WHERE steamid = '%s' AND revision < 2147483647 AND EXISTS (SELECT 1 FROM premium_v3_wallets w WHERE w.steamid = '%s' AND w.last_token = '%s') AND NOT EXISTS (SELECT 1 FROM premium_v3_receipts WHERE token = '%s')", recipient, op.steam, op.token, op.token);
            // MySQL disallows selecting the table being updated. The sender is
            // a different row; use a joined update in that backend.
            if (g_MySQL) g_Database.Format(query, sizeof(query), "UPDATE premium_v3_wallets r JOIN premium_v3_wallets b ON b.steamid = '%s' SET r.revision = r.revision + 1 WHERE r.steamid = '%s' AND r.revision < 2147483647 AND b.last_token = '%s' AND NOT EXISTS (SELECT 1 FROM premium_v3_receipts WHERE token = '%s')", op.steam, recipient, op.token, op.token);
            transaction.AddQuery(query);
        }
        g_Database.Format(query, sizeof(query),
            "INSERT INTO premium_v3_receipts (token, steamid, kind, feature, point_change, expires) SELECT '%s', w.steamid, %d, '%s', w.last_delta, r.expires FROM premium_v3_wallets w JOIN premium_v3_rentals r ON r.steamid = '%s' AND r.feature = '%s' WHERE w.steamid = '%s' AND w.last_token = '%s' AND NOT EXISTS (SELECT 1 FROM premium_v3_receipts WHERE token = '%s')",
            op.token, view_as<int>(op.kind), feature, recipient, feature, op.steam, op.token, op.token);
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
        recipient, feature, op.token, op.token, op.steam);
    transaction.AddQuery(query);
}
