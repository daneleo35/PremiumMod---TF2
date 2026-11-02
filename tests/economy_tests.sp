#pragma semicolon 1
#pragma newdecls required
#include <sourcemod>
#include "../scripting/premium/economy_policy.inc"
int g_Checks;
void Check(bool condition, const char[] label) { g_Checks++; if (!condition) ThrowError("FAIL: %s", label); }
public int main()
{
    Check(EconomyWindow(100,100,101) && !EconomyWindow(101,100,101),"schedule includes start and excludes end");
    Check(EconomyWindow(100,0,0) && !EconomyWindow(99,100,0),"optional unbounded windows and future start");
    Check(EconomySalePrice(101,25)==76 && EconomySalePrice(1,99)==1,"discounts round up and never become free");
    Check(EconomySalePrice(1000000,99)==10000 && EconomySalePrice(100,0)==100,"price arithmetic remains within bounds");
    Check(!EconomySalePrice(1,100) && !EconomySalePrice(2147483647,1),"invalid discount and overflowing base price rejected");
    for (int i; i<view_as<int>(Feature_Count);i++)
    {
        bool blocked=i==view_as<int>(Feature_Flight)||i==view_as<int>(Feature_Airblast)||i==view_as<int>(Feature_Teleport);
        Check(EconomyMapAllows(i,false,false,true)==!blocked,"map pause covers only map-disabled features");
        Check(EconomyMapAllows(i,false,false,false),"owner can disable map purchase pause");
    }
    Check(EconomyGiftSteam("[U:1:4294967295]") && !EconomyGiftSteam("[U:1:4294967296]") && !EconomyGiftSteam("[U:1:0]") && !EconomyGiftSteam("[U:1:01]"),"gift identity is canonical and bounded");
    Check(EconomyDuration(1, "minutes") == 60 && EconomyDuration(2, "hours") == 7200 && EconomyDuration(3, "days") == 259200, "configured time units convert exactly");
    Check(EconomyDuration(365, "days") == 31536000 && EconomyDuration(525600, "minutes") == 31536000, "one-year duration accepted without overflow");
    Check(EconomyDuration(366, "days") == 0 && EconomyDuration(2147483647, "hours") == 0, "overflowing or over-year offers rejected");
    Check(EconomyDuration(0, "hours") == 0 && EconomyDuration(-1, "days") == 0 && EconomyDuration(1, "years") == 0, "invalid quantities and units rejected");
    Check(EconomyTokenValid("0123456789abcdef0123456789abcdef"), "128-bit hex journal token valid");
    Check(!EconomyTokenValid("") && !EconomyTokenValid("0123456789abcdef0123456789abcdeg") && !EconomyTokenValid("0123456789abcdef0123456789abcdefff0"), "malformed token cannot identify a transaction");
    for (int balance; balance <= 20; balance++)
    {
        Check(EconomyCanRent(balance, 10, 0, 100000, 3600, false, 1) == (balance >= 10), "rental price is enforced at each balance boundary");
        Check(EconomyCanBuyTrial(balance, 10, 0, 100000, 0, 1) == (balance >= 10), "trial price is enforced separately");
    }
    Check(!EconomyCanRent(100, 10, 103600, 100000, 3600, false, 1), "active rentals cannot renew when disabled");
    Check(EconomyCanRent(100, 10, 103600, 100000, 3600, true, 1), "configured extensions preserve existing remaining time");
    Check(!EconomyCanRent(100, 10, 103600, 100000, 86400, true, 1), "stacked duration cap rejects additional extension");
    Check(EconomyCanRent(100, 10, 99999, 100000, 86400, false, 1), "expired rental restarts from now");
    Check(!EconomyCanRent(100, 0, 0, 100000, 3600, true, 1), "zero-price journal mutation rejected");
    Check(!EconomyCanRent(100, 10, 0, 2147483600, 3600, true, 365), "rental deadline cannot wrap 2038 timestamp");
    for (int limit = 1; limit <= 5; limit++) for (int used; used <= 6; used++)
        Check(EconomyCanBuyTrial(100, 10, 0, 100000, used, limit) == (used < limit), "lifetime purchase counts gate every trial offer");
    Check(EconomyCanBuyTrial(100, 10, 0, 100000, 100, 0), "zero purchase limit explicitly means unlimited");
    Check(!EconomyCanBuyTrial(100, 10, 100001, 100000, 0, 0), "active trial cannot be bought again even with unlimited count");
    for (int flags; flags < 128; flags++)
    {
        bool active = (flags & 1) != 0, loaded = (flags & 2) != 0, human = (flags & 4) != 0, team = (flags & 8) != 0;
        bool alive = (flags & 16) != 0, dead = (flags & 32) != 0, population = (flags & 64) != 0;
        Check(EconomyEarnEligible(active, loaded, human, team, alive, dead, population, 0, 0, 0, 100)
            == (active && loaded && human && team && (alive || dead) && population), "all optional time-earning gates compose independently");
    }
    Check(EconomyEarnEligible(true, true, true, true, true, false, true, 9999, 0, 0, 100), "AFK filter can be disabled independently");
    Check(!EconomyEarnEligible(true, true, true, true, true, false, true, 300, 300, 0, 100), "enabled AFK cutoff excludes idle time");
    Check(!EconomyEarnEligible(true, true, true, true, true, false, true, 0, 0, 100, 100), "full wallet does not accumulate more earning time");
    EconomyOperation op;
    strcopy(op.token, sizeof(op.token), "0123456789abcdef0123456789abcdef"); strcopy(op.backend, sizeof(op.backend), "storage-local");
    op.kind = Economy_Earn; op.feature = -1; op.cap = 1000000; op.interval = 600; op.reward = 5; op.maxDays = 365;
    Check(EconomyOperationValid(op), "valid saved earnings intent can be recovered");
    op.seconds = -1; Check(!EconomyOperationValid(op), "negative earning time cannot be recovered"); op.seconds = 0;
    op.kind = Economy_Rent; op.feature = view_as<int>(Feature_Glow); op.price = 10; op.duration = 3600;
    Check(EconomyOperationValid(op), "valid rental intent retains quoted terms");
    op.feature = view_as<int>(Feature_Count); Check(!EconomyOperationValid(op), "unknown feature indices cannot be recovered");
    op.kind = Economy_Trial; op.feature = -1; Check(EconomyOperationValid(op), "trial intent is a distinct purchase kind");
    op.feature = view_as<int>(Feature_Glow); Check(!EconomyOperationValid(op), "trial receipt cannot impersonate a rental"); op.feature = -1;
    op.duration = 2147483647; Check(!EconomyOperationValid(op), "corrupt journal duration cannot overflow deadlines"); op.duration = 3600;
    op.price = -1; Check(!EconomyOperationValid(op), "corrupt journal prices cannot create points");
    op.price=10;op.kind=Economy_Gift;op.feature=view_as<int>(Feature_Glow);strcopy(op.steam,sizeof(op.steam),"[U:1:85]");strcopy(op.recipient,sizeof(op.recipient),"[U:1:86]");
    Check(EconomyOperationValid(op),"gift intent preserves beneficiary");
    strcopy(op.recipient,sizeof(op.recipient),op.steam);Check(!EconomyOperationValid(op),"self gifts rejected at durable boundary");
    strcopy(op.recipient,sizeof(op.recipient),"[U:1:86]");op.kind=Economy_Transfer;op.feature=-1;op.transferDaily=500;
    Check(EconomyOperationValid(op),"points transfer has distinct operation without feature access");
    op.transferDaily=-1;Check(!EconomyOperationValid(op),"negative persisted daily limit rejected");
    PrintToServer("PASS: %d points economy policy assertions", g_Checks); return g_Checks;
}
