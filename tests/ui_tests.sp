#pragma semicolon 1
#pragma newdecls required
#include <sourcemod>
#include "../scripting/premium/ui_policy.inc"

int checks;
void Check(bool condition, const char[] label)
{
    checks++;
    if (!condition) ThrowError("UI check failed: %s", label);
}
public int main()
{
    int index;
    Check(PremiumUiIndex("0", 29, index) && index == 0, "first feature");
    Check(PremiumUiIndex("28", 29, index) && index == 28, "last feature");
    Check(!PremiumUiIndex("29", 29, index) && index == -1, "out of range");
    Check(!PremiumUiIndex("-1", 29, index), "negative menu ID");
    Check(!PremiumUiIndex("", 29, index), "empty ID");
    Check(!PremiumUiIndex("1x", 29, index), "partial integer");
    Check(!PremiumUiIndex("999999999999", 29, index), "overflow input");
    Check(!PremiumUiIndex("0", 0, index), "empty catalog");
    Check(!PremiumUiIndex("1:2", 29, index), "malformed ID");
    Check(!PremiumUiIndex(" 1", 29, index), "whitespace ID");
    Check(PremiumUiTargetAllowed(true, 500, 500, true, true, true), "valid target");
    Check(!PremiumUiTargetAllowed(true, 500, 501, true, true, true), "reused slot cannot receive old change");
    Check(!PremiumUiTargetAllowed(true, 500, 0, true, true, true), "disconnected target");
    Check(!PremiumUiTargetAllowed(true, 0, 0, true, true, true), "no target");
    for (int bits; bits < 16; bits++)
    {
        Check(PremiumUiTargetAllowed((bits & 1) != 0, 500, 500, (bits & 2) != 0, (bits & 4) != 0, (bits & 8) != 0) == (bits == 15), "permission/human/auth/immunity gates");
    }
    Check(PremiumUiSettingCurrent(1.25, 1.25), "matching setting quote");
    Check(!PremiumUiSettingCurrent(1.0, 1.25), "changed setting rejects stale confirmation");
    Check(PremiumUiSettingCurrent(1000000000.0, 1000000000.0), "large bounded value");
    bool inside, seen;
    Check(PremiumUiConfigLine("// user comment", inside, seen) == 1, "keep comments");
    Check(PremiumUiConfigLine("exec sourcemod/premium_profiles/fun.cfg", inside, seen) == 1, "keep existing preset");
    Check(PremiumUiConfigLine("// BEGIN PREMIUM MENU SETTINGS", inside, seen) == 0 && inside && seen, "open old block");
    Check(PremiumUiConfigLine("premium_ammo_multiplier 2", inside, seen) == 0, "replace old saved values");
    Check(PremiumUiConfigLine("// BEGIN PREMIUM MENU SETTINGS", inside, seen) == -1, "reject nested block");
    Check(PremiumUiConfigLine("// END PREMIUM MENU SETTINGS", inside, seen) == 0 && !inside, "close old block");
    Check(PremiumUiConfigLine("// keep following user text", inside, seen) == 1, "keep following text");
    Check(PremiumUiConfigLine("// BEGIN PREMIUM MENU SETTINGS", inside, seen) == -1, "reject duplicate block");
    Check(PremiumUiConfigLine("// END PREMIUM MENU SETTINGS", inside, seen) == -1, "reject orphan closing marker");
    char output[32];
    PremiumUiDuration(0, output, sizeof(output)); Check(StrEqual(output, "Expired"), "expiry text");
    PremiumUiDuration(15, output, sizeof(output)); Check(StrEqual(output, "15s"), "seconds");
    PremiumUiDuration(61, output, sizeof(output)); Check(StrEqual(output, "1m 1s"), "minute remainder");
    PremiumUiDuration(3600, output, sizeof(output)); Check(StrEqual(output, "1h 0m"), "hour");
    PremiumUiDuration(90000, output, sizeof(output)); Check(StrEqual(output, "1d 1h"), "day");
    char empty[96];
    PremiumUiEmptyPurchases(false, empty, sizeof(empty)); Check(!StrEqual(empty, "No active purchases"), "loading is not an empty purchase history");
    PremiumUiEmptyPurchases(true, empty, sizeof(empty)); Check(StrEqual(empty, "No active purchases"), "confirmed empty purchase history");
    PrintToServer("PASS: %d in-game UI safety assertions", checks);
    return checks;
}
