#pragma semicolon 1
#pragma newdecls required
#include <sourcemod>
#include "../scripting/premium/preferences.inc"
#include "../scripting/premium/rule_policy.inc"
int g_Checks;
void Check(bool condition, const char[] label) { g_Checks++; if (!condition) ThrowError("FAIL: %s", label); }

public int main()
{
    PremiumOptions chosen, read;
    char value[64];
    for (int flags; flags < 32; flags++)
        for (int tag; tag < 4; tag++)
            for (int glow; glow < 4; glow++)
            {
                chosen.quiet = (flags & 1) != 0; chosen.hideTag = (flags & 2) != 0;
                chosen.cosmetics = (flags & 4) != 0; chosen.reminders = (flags & 8) != 0; chosen.engineerHud = (flags & 16) != 0;
                chosen.tagStyle = tag; chosen.glowStyle = glow;
                EncodePremiumOptions(chosen, value, sizeof(value));
                Check(DecodePremiumOptions(value, read), "every supported preference combination round-trips");
                Check(chosen.quiet == read.quiet && chosen.hideTag == read.hideTag && chosen.cosmetics == read.cosmetics
                    && chosen.reminders == read.reminders && chosen.engineerHud == read.engineerHud
                    && chosen.tagStyle == read.tagStyle && chosen.glowStyle == read.glowStyle, "saved preferences retain all choices");
            }
    char bad[][] = {"", "2 0 0 0 0 1 1 1", "1 0 0 4 0 1 1 1", "1 0 0 0 4 1 1 1", "1 2 0 0 0 1 1 1",
        "1 0 0 0 0 1 1", "1 0 0 0 0 1 1 1 9", "1 0 0 -1 0 1 1 1", "1 0 0 0 0 1 1 x"};
    for (int i; i < sizeof(bad); i++)
    {
        Check(!DecodePremiumOptions(bad[i], read), "bad cookie version, count or range rejected");
        Check(read.cosmetics && read.reminders && read.engineerHud && !read.quiet && !read.hideTag, "bad data restores safe defaults");
    }
    int mask;
    Check(PremiumReminder(301, 300, mask) == 300, "five-minute crossing gives one reminder");
    Check(PremiumReminder(300, 299, mask) == 0, "reminder does not repeat on every heartbeat");
    Check(PremiumReminder(61, 60, mask) == 60, "one-minute reminder");
    Check(PremiumReminder(11, 10, mask) == 10, "ten-second reminder");
    Check(PremiumReminder(10, 0, mask) == 0, "expiry is not a reminder");
    mask = 0;
    Check(PremiumReminder(400, 5, mask) == 10 && mask == 7, "clock jump coalesces crossed thresholds into one notice");
    mask = 0;
    Check(PremiumReminder(50, 49, mask) == 0, "reconnect near expiry does not replay past reminders");
    Check(PremiumRuleNumber("0") && PremiumRuleNumber("-1") && PremiumRuleNumber("1.25"), "valid rule numbers");
    Check(!PremiumRuleNumber("nan") && !PremiumRuleNumber("inf") && !PremiumRuleNumber("1e3"), "nonfinite/exponent text rejected");
    Check(!PremiumRuleNumber("1;quit") && !PremiumRuleNumber(".") && !PremiumRuleNumber("--1") && !PremiumRuleNumber("1.2.3"), "malformed numbers rejected");
    Check(PremiumMapMatches("cp_badlands", "cp_badlands"), "exact map match");
    Check(!PremiumMapMatches("cp_badlands", "cp_badlands_other"), "exact match cannot leak to another map");
    Check(PremiumMapMatches("jump_*", "jump_example"), "trailing wildcard match");
    Check(!PremiumMapMatches("jump_*", "cp_jump_example"), "wildcard is anchored to map prefix");
    Check(PremiumMapMatches("CP_*", "cp_badlands"), "map matching is case insensitive");
    Check(PremiumMapMatches("*", "workshop/123/map"), "explicit all-map rule");
    Check(!PremiumMapMatches("", "map"), "empty pattern ignored");
    Check(PremiumToggle(true, -1) && !PremiumToggle(false, -1), "class toggle inheritance");
    Check(!PremiumToggle(true, 0) && PremiumToggle(false, 1), "explicit class toggle override");
    Check(PremiumRuleFloat(2.0, 0.0) == 2.0 && PremiumRuleFloat(2.0, 1.5) == 1.5, "class numeric inheritance and override");
    Check(PremiumTeamExtraRoom(3, 4) && !PremiumTeamExtraRoom(4, 4), "last team slot and full budget");
    Check(!PremiumTeamExtraRoom(0, 0) && !PremiumTeamExtraRoom(-1, 4), "zero/invalid team budgets");
    PrintToServer("PASS: %d preferences and rules assertions", g_Checks);
    return g_Checks;
}
