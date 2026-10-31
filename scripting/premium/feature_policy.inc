#if defined _premium_feature_policy_included
 #endinput
#endif
#define _premium_feature_policy_included

enum PremiumFeature
{
    Feature_Speed, Feature_Gravity, Feature_Health, Feature_Ammo,
    Feature_KillHealth, Feature_AssistHealth, Feature_Color, Feature_Dark,
    Feature_Glow, Feature_Fov, Feature_Balls, Feature_Jarate, Feature_Uber,
    Feature_Respawn, Feature_Swap, Feature_Cloak, Feature_DeadRinger,
    Feature_Metal, Feature_Sentry, Feature_Dispenser, Feature_Teleporter,
    Feature_FireArrows, Feature_Bhop, Feature_DoubleJump, Feature_Flight,
    Feature_Airblast, Feature_Teleport, Feature_Celebration, Feature_FakeKill,
    Feature_Count
}
char g_FeatureKeys[][] = {
    "speed", "gravity", "health", "ammo", "kill_health", "assist_health", "color", "dark",
    "glow", "fov", "balls", "jarate", "uber", "respawn", "swap", "cloak", "deadringer",
    "metal", "sentry", "dispenser", "teleporter", "firearrows", "bhop", "doublejump",
    "flight", "airblast", "teleport", "celebration", "fakekill"
};

stock int PremiumFeatureBit(PremiumFeature feature) { return 1 << view_as<int>(feature); }
stock int PremiumAllFeatures() { return (1 << view_as<int>(Feature_Count)) - 1; }
stock int PremiumGameplayFeatures()
{
    return PremiumAllFeatures() & ~(PremiumFeatureBit(Feature_Color) | PremiumFeatureBit(Feature_Dark)
        | PremiumFeatureBit(Feature_Glow) | PremiumFeatureBit(Feature_Fov)
        | PremiumFeatureBit(Feature_Celebration) | PremiumFeatureBit(Feature_FakeKill));
}
stock int PremiumFeatureIndex(const char[] key)
{
    for (int i; i < view_as<int>(Feature_Count); i++) if (StrEqual(key, g_FeatureKeys[i], false)) return i;
    return -1;
}
stock bool PremiumFeatureValue(const char[] value) { return StrEqual(value, "0") || StrEqual(value, "1"); }
stock int PremiumFeatureSources(bool member, int publicFeatures, int userFeatures, int explicitOverrides, int permitted)
{
    return ((member ? PremiumAllFeatures() : 0) | publicFeatures | userFeatures | explicitOverrides)
        & permitted & PremiumAllFeatures();
}
stock bool PremiumMembership(bool flag, bool loaded, bool member, bool trialsEnabled, int trialUntil, int now)
{
    return flag || (loaded && (member || (trialsEnabled && trialUntil > now)));
}
