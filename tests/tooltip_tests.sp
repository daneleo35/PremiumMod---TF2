#pragma semicolon 1
#pragma newdecls required
#include <sourcemod>
int g_Target[MAXPLAYERS + 1], g_Shows[MAXPLAYERS + 1], g_Hides[MAXPLAYERS + 1];
char g_Text[MAXPLAYERS + 1][96];
float g_Clock = 100.0;
bool g_ShowWorks = true;
int g_Checks;
#include "../scripting/premium/tooltip.inc"
int TooltipHostTarget(int client) { return g_Target[client]; }
float TooltipHostClock() { return g_Clock; }
void TooltipHostText(int client, int target, char[] text, int size)
{
    if (target == INVALID_ENT_REFERENCE) ThrowError("invalid tooltip target");
    strcopy(text, size, g_Text[client]);
}
bool TooltipHostShow(int client, int target, const char[] text)
{
    if (!g_ShowWorks) return false;
    if (target == INVALID_ENT_REFERENCE || !text[0]) ThrowError("invalid show");
    g_Shows[client]++; return true;
}
void TooltipHostHide(int client)
{
    if (g_Tooltip[client].shown) ThrowError("ownership must clear before engine hide");
    g_Hides[client]++;
    StopTurretTooltip(client); // Reentrant cleanup must remain idempotent.
}
void Check(bool condition, const char[] label) { g_Checks++; if (!condition) ThrowError("FAIL: %s", label); }

public int main()
{
    for (int client = 1; client <= MAXPLAYERS; client++) g_Target[client] = INVALID_ENT_REFERENCE;
    RefreshTurretTooltip(1); Check(g_Shows[1] == 0 && g_Hides[1] == 0, "no target means no overlay or label");
    g_Target[1] = 4136; strcopy(g_Text[1], sizeof(g_Text[]), "Primary | 2/3 sentries");
    RefreshTurretTooltip(1);
    Check(g_Shows[1] == 1 && g_Tooltip[1].shown, "looking at a valid own turret creates one tooltip");
    RefreshTurretTooltip(1); Check(g_Shows[1] == 1, "unchanged tooltip is not resent each heartbeat");
    g_Clock = 100.74; RefreshTurretTooltip(1); Check(g_Shows[1] == 1, "refresh throttled before deadline");
    g_Clock = 100.75; RefreshTurretTooltip(1); Check(g_Shows[1] == 2, "bounded native lifetime refreshed before expiry");
    strcopy(g_Text[1], sizeof(g_Text[]), "Primary | 3/3 sentries"); RefreshTurretTooltip(1);
    Check(g_Shows[1] == 3 && StrContains(g_Tooltip[1].text, "3/3") >= 0, "counter change updates immediately");
    g_Target[1] = 4137; strcopy(g_Text[1], sizeof(g_Text[]), "Extra 1 (L1) | 3/3 sentries"); RefreshTurretTooltip(1);
    Check(g_Shows[1] == 4 && g_Tooltip[1].target == 4137, "aim switch reuses one annotation with new target and role");
    g_Target[1] = INVALID_ENT_REFERENCE; RefreshTurretTooltip(1);
    Check(!g_Tooltip[1].shown && g_Hides[1] == 1, "look away, obstruction or access/feature loss hides the tooltip");
    StopTurretTooltip(1); Check(g_Hides[1] == 1, "repeated cleanup cannot duplicate hide events");
    g_Target[1] = 4136; RefreshTurretTooltip(1); g_ShowWorks = false;
    strcopy(g_Text[1], sizeof(g_Text[]), "Primary | 1/3 sentries"); RefreshTurretTooltip(1);
    Check(!g_Tooltip[1].shown && g_Hides[1] == 2, "failed refresh releases any previous tooltip");
    g_ShowWorks = true; RefreshTurretTooltip(1);
    Check(g_Tooltip[1].shown, "later valid refresh recovers from event creation failure");
    for (int client = 32; client <= MAXPLAYERS; client += MAXPLAYERS - 32)
    {
        g_Target[client] = 5000 + client; strcopy(g_Text[client], sizeof(g_Text[]), "Primary | 1/3 sentries");
        RefreshTurretTooltip(client);
        Check(g_Tooltip[client].shown && g_Shows[client] == 1, "client slots 32 and highest supported slot are independent");
        StopTurretTooltip(client);
        Check(!g_Tooltip[client].shown && g_Hides[client] == 1, "high-slot cleanup is safe without a visibility bitmask");
    }
    StopTurretTooltip(1); Check(!g_Tooltip[1].shown && !g_Tooltip[1].text[0], "session/map/unload cleanup clears tooltip state");
    PrintToServer("PASS: %d turret tooltip assertions", g_Checks);
    return g_Checks;
}
