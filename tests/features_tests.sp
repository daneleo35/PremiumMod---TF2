#pragma semicolon 1
#pragma newdecls required
#include <sourcemod>
#include "../scripting/premium/core.inc"
#include "../scripting/premium/feature_policy.inc"
#pragma unused PremiumRemaining, PremiumCanRespawn, PremiumRespawnPhaseAllowed, PremiumFlameFlightActive
#pragma unused PremiumThrowableAmmoSlot, PremiumGroundJumpLatch, PremiumCanAddSentry, PremiumReserveAmmoIndex
#pragma unused PremiumCloakAfterDrain, PremiumSwapBalanced, PremiumFlightWeapon, PremiumFlightVertical

// Execute the production resolver against fake auth/admin boundaries. No host
// decides whether public/user/override sources grant a feature: production does.
enum struct TestSetting
{
    bool BoolValue;
    int IntValue;
}
TestSetting cvEnable, cvGameplayTeams;
bool g_Stopping, g_DependencyReady = true, g_Member, g_Bot, g_Authenticated = true;
int g_UserMask, g_Explicit, g_Permitted, g_Team = 2, g_Checks;
methodmap TestUsers < Handle
{
    public bool GetValue(const char[] steam, int &value)
    {
        if (!StrEqual(steam, "[U:1:85]")) return false;
        value = g_UserMask; return g_UserMask != 0;
    }
}
bool ValidClient(int client) { return client == 1 || client == 2; }
bool TestFake(int client) { return client == 1 && g_Bot; }
int g_TestRentals;
int EconomyRentalMask(int client) { return client > 0 ? g_TestRentals : 0; }
bool HasPremium(int client) { return ValidClient(client) && g_Member; }
bool TestAuth(int client, AuthIdType type, char[] output, int size, bool validate = true)
{
    if (!ValidClient(client) || !g_Authenticated || type != AuthId_Steam2 || !validate) return false;
    strcopy(output, size, client == 1 ? "STEAM_1:1:42" : "STEAM_0:0:43"); return true;
}
int TestTeam(int client) { return ValidClient(client) ? g_Team : 0; }
bool TestAccess(int client, const char[] command, int flags, bool only)
{
    if (!ValidClient(client) || flags || !only) return false;
    int index = PremiumFeatureIndex(command[16]);
    return index >= 0 && (g_Permitted & (1 << index)) != 0;
}
bool TestOverride(const char[] command, OverrideType type, int &flags)
{
    int index = PremiumFeatureIndex(command[16]); flags = 0;
    return type == Override_Command && index >= 0 && (g_Explicit & (1 << index)) != 0;
}
#define STEAM_LENGTH 32
#define StringMap TestUsers
#define IsFakeClient TestFake
#define GetClientAuthId TestAuth
#define GetClientTeam TestTeam
#define CheckCommandAccess TestAccess
#define GetCommandOverride TestOverride
#include "../scripting/premium/feature_runtime.inc"

void Check(bool condition, const char[] label) { g_Checks++; if (!condition) ThrowError("FAIL: %s", label); }
void Resolve(int client = 1) { g_ClientFeatures[client] = ResolveFeatureMask(client); }
public int main()
{
    cvEnable.BoolValue = true; cvGameplayTeams.IntValue = 3;
    g_UserFeatures = view_as<TestUsers>(1); g_Permitted = PremiumAllFeatures();
    Check(view_as<int>(Feature_Count) == sizeof(g_FeatureKeys) && view_as<int>(Feature_Count) < 31, "every named feature fits independently in the mask");
    Check(ResolveBaseFeatureMask(1) == 0, "base grants exclude economy rentals");
    Resolve(); Check(!HasAnyFeature(1) && !HasPremium(1), "undefined default-allow feature overrides do not create free access");
    g_TestRentals = PremiumFeatureBit(Feature_Glow); Resolve();
    Check(HasFeature(1, Feature_Glow) && !HasPremium(1) && ResolveBaseFeatureMask(1) == 0, "purchased feature merges without membership or a permanent grant");
    g_TestRentals = 0; Resolve(); Check(!HasAnyFeature(1), "expired purchased access disappears when no other source remains");
    for (int i; i < view_as<int>(Feature_Count); i++)
    {
        PremiumFeature feature = view_as<PremiumFeature>(i); int bit = 1 << i;
        Check(PremiumFeatureIndex(g_FeatureKeys[i]) == i, "feature names map uniquely to their stable indices");
        g_PublicFeatures = bit; g_UserMask = 0; g_Explicit = 0; Resolve();
        Check(g_ClientFeatures[1] == bit && HasFeature(1, feature), "public grant enables exactly its selected feature");
        Check(!HasPremium(1), "public feature access never changes membership");
        g_PublicFeatures = 0; g_UserMask = bit; Resolve();
        Check(g_ClientFeatures[1] == bit && HasAnyFeature(1), "Steam-ID grant enables exactly one feature");
        Resolve(2); Check(!HasAnyFeature(2), "individual grants do not leak into another account");
        g_Authenticated = false; Resolve(); Check(!HasAnyFeature(1), "unverified auth cannot claim a user grant");
        g_Authenticated = true;
        g_UserMask = 0; g_Explicit = bit; Resolve();
        Check(g_ClientFeatures[1] == bit && !HasPremium(1), "explicit feature override grants only its feature, not Premium");
        g_PublicFeatures = bit; g_UserMask = bit; g_Permitted &= ~bit; Resolve();
        Check(!HasAnyFeature(1), "permission denial overrides all independent grant sources");
        g_Member = true; Resolve();
        Check(!HasFeature(1, feature) && g_ClientFeatures[1] == (PremiumAllFeatures() & ~bit), "same feature denial also applies to Premium members");
        g_Member = false; g_Permitted = PremiumAllFeatures(); g_Explicit = 0; Resolve();
        Check(g_ClientFeatures[1] == bit, "removing one grant source does not revoke another source");
        g_PublicFeatures = 0; Resolve(); Check(g_ClientFeatures[1] == bit, "personal grant survives public access removal");
        g_UserMask = 0; Resolve(); Check(!HasAnyFeature(1), "removing the final source revokes its feature");
    }
    Check(PremiumFeatureIndex("GLOW") == view_as<int>(Feature_Glow) && PremiumFeatureIndex("premium") == -1
        && PremiumFeatureIndex("") == -1, "named grants accept known keys, never a synthetic Premium key");
    char bad[][] = {"", "2", "-1", "1abc", "true", " 1", "01"};
    Check(PremiumFeatureValue("0") && PremiumFeatureValue("1"), "exact binary grant values accepted");
    for (int i; i < sizeof(bad); i++) Check(!PremiumFeatureValue(bad[i]), "invalid config values cannot silently enable features");
    g_PublicFeatures = PremiumFeatureBit(Feature_Glow); Resolve();
    Check(HasFeature(1, Feature_Glow) && (g_ClientFeatures[1] & PremiumGameplayFeatures()) == 0, "glow alone cannot unlock any combat feature");
    Check(!GameplayFeatureAllowed(1, Feature_Ammo) && !GameplayFeatureAllowed(1, Feature_Health), "cosmetic access does not leak ammo or health");
    g_PublicFeatures = PremiumFeatureBit(Feature_Dispenser); Resolve();
    Check(GameplayFeatureAllowed(1, Feature_Dispenser) && !HasFeature(1, Feature_Sentry) && !HasFeature(1, Feature_Metal), "free dispensers do not grant sentries or metal");
    cvGameplayTeams.IntValue = 1; g_Team = 3;
    Check(HasFeature(1, Feature_Dispenser) && !GameplayFeatureAllowed(1, Feature_Dispenser), "gameplay team restrictions still gate granted features");
    g_PublicFeatures |= PremiumFeatureBit(Feature_Glow); Resolve(); Check(HasFeature(1, Feature_Glow), "cosmetics retain access despite gameplay team restriction");
    g_Team = 2; cvGameplayTeams.IntValue = 3;
    g_Member = true; g_PublicFeatures = PremiumFeatureBit(Feature_Airblast); Resolve();
    Check(g_ClientFeatures[1] == PremiumAllFeatures(), "Premium retains configured access to all features");
    g_Member = false; Resolve();
    Check(g_ClientFeatures[1] == PremiumFeatureBit(Feature_Airblast) && !HasPremium(1), "Premium loss preserves the separately public airblast feature only");
    cvEnable.BoolValue = false; Resolve(); Check(!HasAnyFeature(1), "master disable blocks independent grants");
    cvEnable.BoolValue = true; g_DependencyReady = false; Resolve(); Check(!HasAnyFeature(1), "unready native dependency blocks independent grants");
    g_DependencyReady = true; g_Bot = true; Resolve(); Check(!HasAnyFeature(1), "human feature grants do not apply to bots");
    g_Bot = false; g_Stopping = true; Resolve(); Check(!HasAnyFeature(1), "shutdown blocks all new feature use");
    g_Stopping = false; Resolve(); Check(HasAnyFeature(1), "reenabling a grant uses its existing source without extending membership");
    Check(!PerksAvailable(0) && !PerksAvailable(-1) && !PerksAvailable(MAXPLAYERS + 1), "invalid client indices are rejected before array access");
    for (int flags; flags < 16; flags++)
    {
        bool flag = (flags & 1) != 0, loaded = (flags & 2) != 0, member = (flags & 4) != 0, trials = (flags & 8) != 0;
        bool expected = flag || (loaded && (member || trials));
        Check(PremiumMembership(flag, loaded, member, trials, 200, 100) == expected, "trial/flag/database membership sources remain independent");
    }
    Check(!PremiumMembership(false, true, false, false, 200, 100), "disabling trials removes existing trial membership");
    Check(PremiumMembership(false, true, false, true, 200, 100), "reenabling an unexpired trial retains original deadline");
    Check(!PremiumMembership(false, true, false, true, 200, 200) && !PremiumMembership(false, true, false, true, 200, 201), "trial disable/re-enable cannot extend expired access");
    Check(PremiumMembership(true, false, false, false, 0, 100) && PremiumMembership(false, true, true, false, 0, 100), "disabling trials does not revoke flag or database memberships");
    PrintToServer("PASS: %d independent feature access assertions", g_Checks);
    return g_Checks;
}
