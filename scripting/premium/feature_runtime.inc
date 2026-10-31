#include "feature_policy.inc"
int g_PublicFeatures, g_ClientFeatures[MAXPLAYERS + 1];
StringMap g_UserFeatures;

bool PerksAvailable(int client)
{
    return !g_Stopping && g_DependencyReady && cvEnable.BoolValue && ValidClient(client) && !IsFakeClient(client);
}
bool HasFeature(int client, PremiumFeature feature)
{
    return PerksAvailable(client) && (g_ClientFeatures[client] & PremiumFeatureBit(feature)) != 0;
}
bool GameplayFeatureAllowed(int client, PremiumFeature feature)
{
    return HasFeature(client, feature) && PremiumTeamAllowed(GetClientTeam(client), cvGameplayTeams.IntValue);
}
bool HasAnyFeature(int client) { return PerksAvailable(client) && g_ClientFeatures[client] != 0; }

int ResolveFeatureSources(int client, bool rentals)
{
    if (!PerksAvailable(client)) return 0;
    int user, explicitOverrides, permitted;
    char steam[STEAM_LENGTH], normalized[STEAM_LENGTH], overrideName[64];
    if (g_UserFeatures != null && GetClientAuthId(client, AuthId_Steam2, steam, sizeof(steam))
        && PremiumNormalizeSteam(steam, normalized, sizeof(normalized))) g_UserFeatures.GetValue(normalized, user);
    if (rentals) user |= EconomyRentalMask(client);
    for (int i; i < view_as<int>(Feature_Count); i++)
    {
        FormatEx(overrideName, sizeof(overrideName), "premium_feature_%s", g_FeatureKeys[i]);
        if (!CheckCommandAccess(client, overrideName, 0, true)) continue;
        permitted |= 1 << i;
        int flags;
        // An undefined override's default allow is not an independent grant.
        if (GetCommandOverride(overrideName, Override_Command, flags)) explicitOverrides |= 1 << i;
    }
    return PremiumFeatureSources(HasPremium(client), g_PublicFeatures, user, explicitOverrides, permitted);
}


int ResolveFeatureMask(int client) { return ResolveFeatureSources(client, true); }
int ResolveBaseFeatureMask(int client) { return ResolveFeatureSources(client, false); }
