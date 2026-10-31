// Versioned, validated preference data. No permissions or membership live here.
enum struct PremiumOptions
{
    bool quiet;
    bool hideTag;
    int tagStyle;
    int glowStyle; // 0 server, 1 classic, 2 team, 3 subtle
    bool cosmetics;
    bool reminders;
    bool engineerHud;
}

void DefaultPremiumOptions(PremiumOptions options)
{
    PremiumOptions clean;
    options = clean;
    options.cosmetics = true;
    options.reminders = true;
    options.engineerHud = true;
}

bool DecodePremiumOptions(const char[] value, PremiumOptions options)
{
    DefaultPremiumOptions(options);
    char fields[8][8];
    if (strlen(value) != 15 || ExplodeString(value, " ", fields, sizeof(fields), sizeof(fields[])) != 8 || !StrEqual(fields[0], "1")) return false;
    int values[8];
    for (int i = 1; i < 8; i++)
    {
        if (strlen(fields[i]) != 1 || fields[i][0] < '0' || fields[i][0] > ((i == 3 || i == 4) ? '3' : '1')) return false;
        values[i] = fields[i][0] - '0';
    }
    options.quiet = values[1] != 0;
    options.hideTag = values[2] != 0;
    options.tagStyle = values[3];
    options.glowStyle = values[4];
    options.cosmetics = values[5] != 0;
    options.reminders = values[6] != 0;
    options.engineerHud = values[7] != 0;
    return true;
}

void EncodePremiumOptions(PremiumOptions options, char[] output, int size)
{
    FormatEx(output, size, "1 %d %d %d %d %d %d %d", options.quiet, options.hideTag,
        options.tagStyle, options.glowStyle, options.cosmetics, options.reminders, options.engineerHud);
}

int PremiumReminder(int previous, int remaining, int &mask)
{
    int thresholds[] = {300, 60, 10};
    int notice;
    for (int i; i < sizeof(thresholds); i++)
        if (previous > thresholds[i] && remaining <= thresholds[i] && remaining > 0 && !(mask & (1 << i)))
        { mask |= 1 << i; notice = thresholds[i]; }
    return notice;
}
