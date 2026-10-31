// Owner-only native annotation controller. No world entities or large HUD text.
enum struct PremiumTooltip
{
    bool shown;
    int target;
    char text[96];
}
PremiumTooltip g_Tooltip[MAXPLAYERS + 1];

void StopTurretTooltip(int client)
{
    bool shown = g_Tooltip[client].shown;
    g_Tooltip[client].shown = false;
    g_Tooltip[client].target = INVALID_ENT_REFERENCE;
    g_Tooltip[client].text[0] = '\0';
    if (shown) TooltipHostHide(client);
}

void RefreshTurretTooltip(int client)
{
    int target = TooltipHostTarget(client);
    if (target == INVALID_ENT_REFERENCE) { StopTurretTooltip(client); return; }
    char text[96]; TooltipHostText(client, target, text, sizeof(text));
    if (g_Tooltip[client].shown && g_Tooltip[client].target == target
        && StrEqual(g_Tooltip[client].text, text)) return;
    // show_annotation calls client Touch(), resetting the callout to the left
    // and restarting its interpolation. The client follows the entity itself;
    // re-send only for a changed target/text, never as a periodic keepalive.
    if (!TooltipHostShow(client, target, text)) { StopTurretTooltip(client); return; }
    g_Tooltip[client].shown = true;
    g_Tooltip[client].target = target;
    strcopy(g_Tooltip[client].text, sizeof(g_Tooltip[].text), text);
}

void PremiumSentryTooltip(const char[] role, int count, int cap, int total, char[] text, int size)
{
    if (role[0]) FormatEx(text, size, "%s\nSentries %d/%d | Total %d/6", role, count, cap, total);
    else FormatEx(text, size, "Sentries %d/%d | Total %d/6", count, cap, total);
}

void PremiumUtilityTooltip(int kind, int count, int cap, int total, char[] text, int size)
{
    FormatEx(text, size, "%s %d/%d\nTotal %d/6", kind == 0 ? "Dispensers" : (kind == 1 ? "Entrances" : "Exits"), count, cap, total);
}
