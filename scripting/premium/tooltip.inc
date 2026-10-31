// Owner-only native annotation controller. No world entities or large HUD text.
enum struct PremiumTooltip
{
    bool shown;
    int target;
    char text[96];
    float refreshAt;
}
PremiumTooltip g_Tooltip[MAXPLAYERS + 1];

void StopTurretTooltip(int client)
{
    bool shown = g_Tooltip[client].shown;
    g_Tooltip[client].shown = false;
    g_Tooltip[client].target = INVALID_ENT_REFERENCE;
    g_Tooltip[client].text[0] = '\0';
    g_Tooltip[client].refreshAt = 0.0;
    if (shown) TooltipHostHide(client);
}

void RefreshTurretTooltip(int client)
{
    int target = TooltipHostTarget(client);
    if (target == INVALID_ENT_REFERENCE) { StopTurretTooltip(client); return; }
    char text[96]; TooltipHostText(client, target, text, sizeof(text));
    float now = TooltipHostClock();
    if (g_Tooltip[client].shown && g_Tooltip[client].target == target
        && StrEqual(g_Tooltip[client].text, text) && now < g_Tooltip[client].refreshAt) return;
    if (!TooltipHostShow(client, target, text)) { StopTurretTooltip(client); return; }
    g_Tooltip[client].shown = true;
    g_Tooltip[client].target = target;
    strcopy(g_Tooltip[client].text, sizeof(g_Tooltip[].text), text);
    g_Tooltip[client].refreshAt = now + 0.75;
}
