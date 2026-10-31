bool PremiumRuleNumber(const char[] value)
{
    int start = value[0] == '-' ? 1 : 0;
    int digits, dots;
    for (int i = start; value[i]; i++)
    {
        if (value[i] >= '0' && value[i] <= '9') digits++;
        else if (value[i] == '.') dots++;
        else return false;
    }
    return digits > 0 && dots <= 1;
}

bool PremiumMapMatches(const char[] pattern, const char[] map)
{
    int size = strlen(pattern);
    if (!size || size >= 128) return false;
    if (pattern[size - 1] != '*') return StrEqual(pattern, map, false);
    char prefix[128]; strcopy(prefix, sizeof(prefix), pattern);
    prefix[size - 1] = '\0';
    return StrContains(map, prefix, false) == 0;
}

bool PremiumToggle(bool global, int specific)
{
    return specific < 0 ? global : specific != 0;
}

float PremiumRuleFloat(float global, float specific)
{
    return specific == 0.0 ? global : specific;
}

bool PremiumTeamExtraRoom(int existing, int cap)
{
    return existing >= 0 && cap >= 0 && existing < cap;
}
