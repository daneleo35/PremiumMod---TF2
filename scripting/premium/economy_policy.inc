#if defined _premium_economy_policy_included
 #endinput
#endif
#define _premium_economy_policy_included
#include "feature_policy.inc"

enum EconomyKind { Economy_Earn, Economy_Rent, Economy_Trial, Economy_Adjust, Economy_Gift, Economy_Transfer }

stock bool EconomyWindow(int now, int from, int until)
{
    return (from == 0 || now >= from) && (until == 0 || now < until);
}
stock int EconomySalePrice(int price, int percent)
{
    if (price < 1 || price > 1000000 || percent < 0 || percent > 99) return 0;
    return (price * (100 - percent) + 99) / 100;
}
stock bool EconomyMapAllows(int feature, bool flight, bool teleport, bool pause)
{
    if (!pause) return true;
    if (feature == view_as<int>(Feature_Flight) || feature == view_as<int>(Feature_Airblast)) return flight;
    if (feature == view_as<int>(Feature_Teleport)) return teleport;
    return true;
}
stock bool EconomyGiftSteam(const char[] steam)
{
    int length = strlen(steam);
    if (length < 7 || length > 16 || StrContains(steam, "[U:1:") != 0 || steam[length - 1] != ']') return false;
    if (steam[5] == '0' || (length == 16 && strcmp(steam[5], "4294967295]") > 0)) return false;
    for (int i = 5; i < length - 1; i++) if (steam[i] < '0' || steam[i] > '9') return false;
    return true;
}
stock bool EconomyTokenValid(const char[] token)
{
    if (strlen(token) != 32) return false;
    for (int i; i < 32; i++) if (!((token[i] >= '0' && token[i] <= '9') || (token[i] >= 'a' && token[i] <= 'f'))) return false;
    return true;
}
stock int EconomyDuration(int amount, const char[] unit)
{
    if (amount < 1) return 0;
    int scale;
    if (StrEqual(unit, "minutes")) scale = 60;
    else if (StrEqual(unit, "hours")) scale = 3600;
    else if (StrEqual(unit, "days")) scale = 86400;
    if (!scale || amount > 31536000 / scale) return 0;
    return amount * scale;
}
stock int EconomyExpiryLimit(int now, int days)
{
    if (now <= 0 || days < 1 || days > 365) return 0;
    int seconds = days * 86400;
    return now > 2147483647 - seconds ? 2147483647 : now + seconds;
}
stock bool EconomyCanRent(int balance, int price, int oldExpiry, int now, int duration, bool renew, int maximumDays)
{
    if (balance < price || price < 1 || duration < 60 || duration > 31536000 || (!renew && oldExpiry > now)) return false;
    int start = oldExpiry > now ? oldExpiry : now;
    return start > 0 && start <= EconomyExpiryLimit(now, maximumDays) - duration;
}
stock bool EconomyCanBuyTrial(int balance, int price, int oldExpiry, int now, int purchases, int limit)
{
    return price > 0 && balance >= price && oldExpiry <= now && purchases >= 0 && (limit == 0 || purchases < limit);
}
stock bool EconomyEarnEligible(bool active, bool loaded, bool human, bool team, bool alive, bool countDead,
    bool enoughPlayers, int idleSeconds, int afkLimit, int balance, int cap)
{
    return active && loaded && human && team && (alive || countDead) && enoughPlayers
        && (afkLimit == 0 || idleSeconds < afkLimit) && balance < cap;
}

enum struct EconomyOperation
{
    char token[33];
    char steam[32];
    char recipient[32]; // Empty for pre-gift journal entries and self purchases.
    char backend[64];
    EconomyKind kind;
    int feature;
    int seconds;
    int interval;
    int reward;
    int cap;
    int price;
    int duration;
    int maxDays;
    int trialLimit;
    bool renew;
    int actorSerial;
    int buyerSerial;
    int transferDaily;
}

stock bool EconomyOperationValid(EconomyOperation op)
{
    if (!EconomyTokenValid(op.token) || !op.backend[0] || op.kind < Economy_Earn || op.kind > Economy_Transfer
        || op.cap < 1 || op.cap > 1000000000 || op.price < 0 || op.price > 1000000
        || op.seconds < -1000000 || op.seconds > 1000000 || op.interval < 60 || op.interval > 86400
        || op.reward < 1 || op.reward > 1000000 || op.duration < 0 || op.duration > 31536000
        || op.maxDays < 1 || op.maxDays > 365 || op.trialLimit < 0 || op.trialLimit > 10000
        || op.transferDaily < 0 || op.transferDaily > 1000000000) return false;
    if (op.kind != Economy_Rent && op.kind != Economy_Gift && op.feature != -1) return false;
    if (op.kind == Economy_Earn && op.seconds < 0) return false;
    if ((op.kind == Economy_Rent || op.kind == Economy_Gift) && (op.feature < 0 || op.feature >= view_as<int>(Feature_Count))) return false;
    if ((op.kind == Economy_Rent || op.kind == Economy_Trial || op.kind == Economy_Gift) && (op.price < 1 || op.duration < 60)) return false;
    if (op.kind == Economy_Gift || op.kind == Economy_Transfer)
    {
        if (!EconomyGiftSteam(op.recipient) || StrEqual(op.steam, op.recipient)) return false;
    }
    if (op.kind == Economy_Transfer && (op.price < 1 || op.feature != -1)) return false;
    return true;
}
