#pragma semicolon 1
#pragma newdecls required
#include <sourcemod>
#include "../scripting/premium/feature_policy.inc"
#include "../scripting/premium/economy_policy.inc"
methodmap TestDatabase
{
    public native int Format(char[] output, int size, const char[] format, any ...);
}
methodmap TestTransaction
{
    public native void AddQuery(const char[] query);
}
TestDatabase g_Database;
bool g_MySQL;
int g_Now;
int TestTime() { return g_Now; }
#define Transaction TestTransaction
#define GetTime TestTime
#include "../scripting/premium/economy_queries.inc"

public int Emit(int kind, int seconds, int interval, int reward, int cap, int price, int duration, int renew, int trialLimit, int days, int now, int nonce, int player, int feature, int mysql, int recipient, int daily)
{
    EconomyOperation op;
    op.kind = view_as<EconomyKind>(kind); op.feature = feature;
    op.seconds = seconds; op.interval = interval; op.reward = reward; op.cap = cap; op.price = price; op.duration = duration;
    op.renew = renew != 0; op.trialLimit = trialLimit; op.maxDays = days;
    FormatEx(op.steam, sizeof(op.steam), "[U:1:%d]", player);
    FormatEx(op.recipient, sizeof(op.recipient), "[U:1:%d]", recipient); op.transferDaily = daily;
    FormatEx(op.token, sizeof(op.token), "%032x", nonce);
    g_Now = now; g_MySQL = mysql != 0; g_Database = view_as<TestDatabase>(1);
    AddEconomyQueries(view_as<TestTransaction>(1), op);
    return 1;
}
