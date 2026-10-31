#include "feature_policy.inc"
#include "economy_policy.inc"
#define SQL_ECON_WALLET "CREATE TABLE IF NOT EXISTS premium_v3_wallets (steamid VARCHAR(32) NOT NULL PRIMARY KEY, points INTEGER NOT NULL DEFAULT 0, play_seconds INTEGER NOT NULL DEFAULT 0, trial_until INTEGER NOT NULL DEFAULT 0, trial_purchases INTEGER NOT NULL DEFAULT 0, revision INTEGER NOT NULL DEFAULT 0, last_token VARCHAR(64) NOT NULL DEFAULT '', last_delta INTEGER NOT NULL DEFAULT 0)"
#define SQL_ECON_RENTALS "CREATE TABLE IF NOT EXISTS premium_v3_rentals (steamid VARCHAR(32) NOT NULL, feature VARCHAR(32) NOT NULL, expires INTEGER NOT NULL DEFAULT 0, PRIMARY KEY (steamid, feature))"
#define SQL_ECON_RECEIPTS "CREATE TABLE IF NOT EXISTS premium_v3_receipts (token VARCHAR(64) NOT NULL PRIMARY KEY, steamid VARCHAR(32) NOT NULL, kind INTEGER NOT NULL, feature VARCHAR(32) NOT NULL, point_change INTEGER NOT NULL, expires INTEGER NOT NULL DEFAULT 0, accepted INTEGER NOT NULL DEFAULT 1)"

enum struct EconomyClient
{
    char steam[STEAM_LENGTH];
    bool loaded;
    bool loading;
    int epoch;
    int points;
    int progress;
    int revision;
    int trialUntil;
    int trialPurchases;
    int rentalUntil[Feature_Count];
    bool trialNotice;
    int rentalNotices;
    float pendingSeconds;
    float lastClock;
    float lastInput;
    int buttons;
    bool purchasing;
}
enum struct ShopOffer
{
    char id[32];
    char name[64];
    int feature; // -1 is a purchased Premium trial, never an individual feature.
    int price;
    int duration;
}
EconomyClient g_Economy[MAXPLAYERS + 1];
ShopOffer g_ShopOffers[64];
int g_ShopCount, g_ShopRevision;
bool g_ShopValid, g_EconomyReady, g_EconomySchemaLoading, g_EconomySending, g_EconomyJournalHealthy = true, g_EconomyOutboxCorrupt;
int g_EconomyEpoch;
float g_EconomyRetry;
ArrayList g_EconomyOutbox;
ConVar cvPoints, cvPointsEarn, cvPointsShop, cvPointsRentals, cvPointsTrials, cvPointsTrialLimit;
ConVar cvPointsMinutes, cvPointsReward, cvPointsCap, cvPointsMinPlayers, cvPointsSpectators, cvPointsDead, cvPointsAfk;
ConVar cvPointsEarnNotice, cvPointsBuyNotice, cvPointsExpiryNotice, cvPointsRenew, cvPointsMaxDays;

int EconomyRentalMask(int client)
{
    if (cvPoints == null || !cvPoints.BoolValue || !cvPointsRentals.BoolValue || !g_EconomyReady || !g_Economy[client].loaded) return 0;
    int mask, now = GetTime();
    for (int i; i < view_as<int>(Feature_Count); i++) if (g_Economy[client].rentalUntil[i] > now) mask |= 1 << i;
    return mask;
}
bool EconomyTrialActive(int client)
{
    return cvPoints != null && cvPoints.BoolValue && cvPointsTrials.BoolValue && g_EconomyReady
        && g_Economy[client].loaded && g_Economy[client].trialUntil > GetTime();
}
