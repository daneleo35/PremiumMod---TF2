#if defined _premium_ui_policy_included
 #endinput
#endif
#define _premium_ui_policy_included

// IDs come from server-created menus, but are still parsed strictly before use.
stock bool PremiumUiIndex(const char[] text, int count, int &index)
{
    index = -1;
    int length = strlen(text);
    if (length < 1 || length > 8 || count < 1) return false;
    for (int i; i < length; i++) if (text[i] < '0' || text[i] > '9') return false;
    int value = StringToInt(text);
    if (value < 0 || value >= count) return false;
    index = value;
    return true;
}

stock bool PremiumUiTargetAllowed(bool actorAllowed, int expectedSerial, int currentSerial, bool human, bool authenticated, bool targetable)
{
    return actorAllowed && expectedSerial > 0 && expectedSerial == currentSerial && human && authenticated && targetable;
}

stock bool PremiumUiSettingCurrent(float current, float quoted)
{
    return FloatAbs(current - quoted) <= 0.000001;
}

// -1 rejects damaged/nested blocks, 0 skips our old block, 1 keeps user text.
stock int PremiumUiConfigLine(const char[] trimmed, bool &inside, bool &seen)
{
    if (StrEqual(trimmed, "// BEGIN PREMIUM MENU SETTINGS"))
    { if (inside || seen) return -1; inside = true; seen = true; return 0; }
    if (StrEqual(trimmed, "// END PREMIUM MENU SETTINGS"))
    { if (!inside) return -1; inside = false; return 0; }
    return inside ? 0 : 1;
}

stock void PremiumUiDuration(int seconds, char[] output, int size)
{
    if (seconds < 1) { strcopy(output, size, "Expired"); return; }
    if (seconds >= 86400) FormatEx(output, size, "%dd %dh", seconds / 86400, (seconds % 86400) / 3600);
    else if (seconds >= 3600) FormatEx(output, size, "%dh %dm", seconds / 3600, (seconds % 3600) / 60);
    else if (seconds >= 60) FormatEx(output, size, "%dm %ds", seconds / 60, seconds % 60);
    else FormatEx(output, size, "%ds", seconds);
}

stock void PremiumUiEmptyPurchases(bool loaded, char[] output, int size)
{
    strcopy(output, size, loaded ? "No active purchases" : "Purchases unavailable until your account loads");
}
