#include "preferences.inc"
PremiumOptions g_Options[MAXPLAYERS + 1];
Cookie g_OptionsCookie;
bool g_OptionsDirty[MAXPLAYERS + 1];
int g_ReminderExpiry[MAXPLAYERS + 1], g_ReminderPrevious[MAXPLAYERS + 1], g_ReminderMask[MAXPLAYERS + 1];
float g_NextCosmeticToggle[MAXPLAYERS + 1];

void InitializeOptions()
{
    g_OptionsCookie = new Cookie("premium_options_v1", "Premium display and notification preferences", CookieAccess_Private);
    for (int client = 1; client <= MaxClients; client++) ResetOptions(client);
}

void ResetOptions(int client)
{
    DefaultPremiumOptions(g_Options[client]);
    g_OptionsDirty[client] = false;
    g_ReminderExpiry[client] = 0;
    g_ReminderPrevious[client] = -1;
    g_ReminderMask[client] = 0;
    g_NextCosmeticToggle[client] = 0.0;
}

void SaveOptions(int client)
{
    g_OptionsDirty[client] = true;
    if (!AreClientCookiesCached(client)) return;
    char value[64];
    EncodePremiumOptions(g_Options[client], value, sizeof(value));
    g_OptionsCookie.Set(client, value);
    g_OptionsDirty[client] = false;
}

public void OnClientCookiesCached(int client)
{
    if (!ValidClient(client) || IsFakeClient(client)) return;
    // A choice made before the async cookie read wins over the saved value.
    if (g_OptionsDirty[client]) { SaveOptions(client); return; }
    char value[64];
    g_OptionsCookie.Get(client, value, sizeof(value));
    bool wasVisible = g_Options[client].cosmetics;
    DecodePremiumOptions(value, g_Options[client]);
    if (wasVisible != g_Options[client].cosmetics) RebuildPremiumCosmetics();
    RefreshGlow(client);
}

void DisplayOptions(int client)
{
    Menu menu = new Menu(OptionsSelected);
    menu.SetTitle("Premium preferences");
    char label[128];
    FormatEx(label, sizeof(label), "Quiet mode: %s", g_Options[client].quiet ? "on" : "off"); menu.AddItem("quiet", label);
    FormatEx(label, sizeof(label), "My Premium tag: %s", g_Options[client].hideTag ? "hidden" : "shown"); menu.AddItem("tag", label);
    char tags[][] = {"server default", "team colour", "gold", "plain"};
    FormatEx(label, sizeof(label), "Tag style: %s", tags[g_Options[client].tagStyle]); menu.AddItem("tagstyle", label);
    char glows[][] = {"server default", "classic", "team", "subtle team"};
    FormatEx(label, sizeof(label), "Glow style: %s", glows[g_Options[client].glowStyle]); menu.AddItem("glowstyle", label);
    FormatEx(label, sizeof(label), "Show Premium particle effects: %s", g_Options[client].cosmetics ? "yes" : "no"); menu.AddItem("cosmetics", label);
    FormatEx(label, sizeof(label), "Trial reminders: %s", g_Options[client].reminders ? "on" : "off"); menu.AddItem("reminders", label);
    FormatEx(label, sizeof(label), "Turret tooltip: %s", g_Options[client].engineerHud ? "on" : "off"); menu.AddItem("hud", label);
    menu.ExitBackButton = true;
    menu.Display(client, 30);
}

public int OptionsSelected(Menu menu, MenuAction action, int client, int selection)
{
    if (action == MenuAction_End) delete menu;
    else if (action == MenuAction_Cancel && selection == MenuCancel_ExitBack && ValidClient(client)) DisplayFeatures(client);
    else if (action == MenuAction_Select && ValidClient(client))
    {
        char key[20]; menu.GetItem(selection, key, sizeof(key));
        if (StrEqual(key, "quiet")) g_Options[client].quiet = !g_Options[client].quiet;
        else if (StrEqual(key, "tag")) g_Options[client].hideTag = !g_Options[client].hideTag;
        else if (StrEqual(key, "tagstyle"))
        {
            int next = g_Options[client].tagStyle;
            do { next = (next + 1) % 4; } while (next != 0 && !(cvTagStyles.IntValue & (1 << (next - 1))));
            g_Options[client].tagStyle = next;
        }
        else if (StrEqual(key, "glowstyle"))
        {
            g_Options[client].glowStyle = cvPersonalGlow.BoolValue ? (g_Options[client].glowStyle + 1) % 4 : 0;
            StopGlow(client); RefreshGlow(client);
        }
        else if (StrEqual(key, "cosmetics"))
        {
            if (GetEngineTime() >= g_NextCosmeticToggle[client])
            {
                g_Options[client].cosmetics = !g_Options[client].cosmetics;
                g_NextCosmeticToggle[client] = GetEngineTime() + 2.0;
                RebuildPremiumCosmetics();
            }
        }
        else if (StrEqual(key, "reminders")) g_Options[client].reminders = !g_Options[client].reminders;
        else if (StrEqual(key, "hud")) g_Options[client].engineerHud = !g_Options[client].engineerHud;
        SaveOptions(client);
        DisplayOptions(client);
    }
    return 0;
}

public Action CommandOptions(int client, int args)
{
    if (ValidClient(client)) DisplayOptions(client); else Tell(client, "%t", "In Game Only");
    return Plugin_Handled;
}

void TrialReminder(int client, int now)
{
    int expiry = g_Player[client].trialUntil;
    int remaining = PremiumRemaining(expiry, now);
    if (g_ReminderExpiry[client] != expiry)
    {
        g_ReminderExpiry[client] = expiry;
        g_ReminderPrevious[client] = remaining;
        g_ReminderMask[client] = 0;
    }
    int notice = PremiumReminder(g_ReminderPrevious[client], remaining, g_ReminderMask[client]);
    g_ReminderPrevious[client] = remaining;
    if (notice && cvReminders.BoolValue && g_Options[client].reminders && !g_Options[client].quiet
        && !FlagPremium(client) && !g_Player[client].member && HasPremium(client))
        Tell(client, "%t", "Trial Reminder", remaining);
}
