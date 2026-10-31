#include "ui_settings_data.inc"

bool UiSetting(int index, ConVar &setting)
{
    if (index < 0 || index >= sizeof(g_UiSettingNames)) return false;
    setting = FindConVar(g_UiSettingNames[index]);
    return setting != null;
}

void UiSettings(int client)
{
    if (!UiAdmin(client)) return;
    Menu menu = new Menu(UiSettingsSelected); menu.SetTitle("Server settings\nApply a value, then Save to keep it\n ");
    menu.AddItem("save", "SAVE current settings to config");
    for (int i; i < sizeof(g_UiSettingSections); i++) { char data[12]; IntToString(i, data, sizeof(data)); menu.AddItem(data, g_UiSettingSections[i]); }
    menu.ExitBackButton = true; menu.Display(client, 60);
}
public int UiSettingsSelected(Menu menu, MenuAction action, int client, int selection)
{
    if (action == MenuAction_End) delete menu;
    else if (action == MenuAction_Cancel && selection == MenuCancel_ExitBack && UiAdmin(client)) UiAdminHome(client);
    else if (action == MenuAction_Select && UiAdmin(client))
    {
        char key[24]; menu.GetItem(selection, key, sizeof(key)); int section;
        if (StrEqual(key, "save")) { char operation[48]; FormatEx(operation, sizeof(operation), "savecfg:%d", g_UiSettingsRevision); UiAdminConfirm(client, operation, "Save current numeric settings?\nThey will override matching main / preset values.\nYour comments and other settings are kept.\nA .menu-bak backup will be made."); }
        else if (PremiumUiIndex(key, sizeof(g_UiSettingSections), section)) UiSettingSection(client, section);
    }
    return 0;
}
void UiSettingSection(int client, int section)
{
    if (!UiAdmin(client) || section < 0 || section >= sizeof(g_UiSettingSections)) return;
    Menu menu = new Menu(UiSettingSectionSelected); menu.SetTitle("%s\nChoose a setting\n ", g_UiSettingSections[section]);
    for (int i; i < sizeof(g_UiSettingNames); i++) if (g_UiSettingSection[i] == section)
    {
        ConVar setting; if (!UiSetting(i, setting)) continue;
        char data[16], label[192], value[96]; IntToString(i, data, sizeof(data)); setting.GetString(value, sizeof(value));
        FormatEx(label, sizeof(label), "%s: %s%s", g_UiSettingLabels[i], value, g_UiSettingNumeric[i] ? "" : " (edit cfg)");
        menu.AddItem(data, label, g_UiSettingNumeric[i] ? ITEMDRAW_DEFAULT : ITEMDRAW_DISABLED);
    }
    menu.ExitBackButton = true; menu.Display(client, 60);
}
public int UiSettingSectionSelected(Menu menu, MenuAction action, int client, int selection)
{
    if (action == MenuAction_End) delete menu;
    else if (action == MenuAction_Cancel && selection == MenuCancel_ExitBack && UiAdmin(client)) UiSettings(client);
    else if (action == MenuAction_Select && UiAdmin(client))
    { char key[16]; int index; menu.GetItem(selection, key, sizeof(key)); if (PremiumUiIndex(key, sizeof(g_UiSettingNames), index)) UiSettingEditor(client, index); }
    return 0;
}
void UiSettingEditor(int client, int index)
{
    ConVar setting; if (!UiAdmin(client) || !UiSetting(index, setting) || !g_UiSettingNumeric[index]) return;
    float minimum, maximum; if (!setting.GetBounds(ConVarBound_Lower, minimum) || !setting.GetBounds(ConVarBound_Upper, maximum)) return;
    float current = setting.FloatValue;
    Menu menu = new Menu(UiSettingValueSelected); menu.SetTitle("%s\n%s\nCurrent: %.2f | Allowed: %.2f to %.2f\nApply live, then Save to keep\n ", g_UiSettingLabels[index], g_UiSettingNames[index], current, minimum, maximum);
    if (minimum == 0.0 && maximum == 1.0)
    {
        char data[96]; FormatEx(data, sizeof(data), "%d:0:%.6f", index, current); menu.AddItem(data, "OFF", current == 0.0 ? ITEMDRAW_DISABLED : ITEMDRAW_DEFAULT);
        FormatEx(data, sizeof(data), "%d:1:%.6f", index, current); menu.AddItem(data, "ON", current == 1.0 ? ITEMDRAW_DISABLED : ITEMDRAW_DEFAULT);
    }
    else
    {
        int amounts[] = {-100, -10, -1, 1, 10, 100}; float previous = -10000000.0;
        for (int i; i < sizeof(amounts); i++)
        {
            float proposed = current + float(amounts[i]) * g_UiSettingStep[index];
            if (proposed < minimum) proposed = minimum; if (proposed > maximum) proposed = maximum;
            if (FloatAbs(proposed - current) < 0.000001 || FloatAbs(proposed - previous) < 0.000001) continue;
            char data[96], label[64]; FormatEx(data, sizeof(data), "%d:%.6f:%.6f", index, proposed, current); FormatEx(label, sizeof(label), "Set to %.2f", proposed); menu.AddItem(data, label); previous = proposed;
        }
    }
    char original[64], data[96], label[96]; setting.GetDefault(original, sizeof(original));
    FormatEx(data, sizeof(data), "%d:%.6f:%.6f", index, StringToFloat(original), current); FormatEx(label, sizeof(label), "Restore default (%s)", original); menu.AddItem(data, label);
    menu.ExitBackButton = true; menu.Display(client, 60);
}
public int UiSettingValueSelected(Menu menu, MenuAction action, int client, int selection)
{
    if (action == MenuAction_End) delete menu;
    else if (action == MenuAction_Cancel && selection == MenuCancel_ExitBack && UiAdmin(client)) UiSettings(client);
    else if (action == MenuAction_Select && UiAdmin(client))
    {
        char data[96], parts[3][32]; int index; menu.GetItem(selection, data, sizeof(data));
        if (ExplodeString(data, ":", parts, sizeof(parts), sizeof(parts[])) != 3 || !PremiumUiIndex(parts[0], sizeof(g_UiSettingNames), index)) return 0;
        Menu confirm = new Menu(UiSettingConfirmSelected); confirm.SetTitle("Change %s?\n%s -> %s\nTakes effect now. Save separately to keep.\nMay change bonuses / remove extra buildings.\n ", g_UiSettingLabels[index], parts[2], parts[1]);
        confirm.AddItem(data, "Apply now"); confirm.AddItem("cancel", "Cancel"); confirm.ExitBackButton = true; confirm.Display(client, 30);
    }
    return 0;
}
public int UiSettingConfirmSelected(Menu menu, MenuAction action, int client, int selection)
{
    if (action == MenuAction_End) delete menu;
    else if (action == MenuAction_Cancel && selection == MenuCancel_ExitBack && UiAdmin(client)) UiSettings(client);
    else if (action == MenuAction_Select && UiAdmin(client))
    {
        char data[96], parts[3][32]; int index; menu.GetItem(selection, data, sizeof(data));
        if (StrEqual(data, "cancel")) { UiSettings(client); return 0; }
        if (ExplodeString(data, ":", parts, sizeof(parts), sizeof(parts[])) != 3 || !PremiumUiIndex(parts[0], sizeof(g_UiSettingNames), index) || !g_UiSettingNumeric[index]) return 0;
        ConVar setting; if (!UiSetting(index, setting)) return 0;
        if (!PremiumUiSettingCurrent(setting.FloatValue, StringToFloat(parts[2])))
        { Tell(client, "This setting changed while your menu was open. Review its new value."); UiSettingEditor(client, index); return 0; }
        float minimum, maximum, value = StringToFloat(parts[1]);
        if (!setting.GetBounds(ConVarBound_Lower, minimum) || !setting.GetBounds(ConVarBound_Upper, maximum) || value < minimum || value > maximum) return 0;
        LogAction(client, -1, "Premium menu: %s changed from %.2f to %.2f", g_UiSettingNames[index], setting.FloatValue, value);
        setting.SetFloat(value); Tell(client, "Applied. Choose Save in Server settings to keep this after restart."); UiSettingEditor(client, index);
    }
    return 0;
}

bool UiSaveSettings(int client)
{
    if (!UiAdmin(client)) return false;
    char path[PLATFORM_MAX_PATH] = "cfg/sourcemod/premium_rebuilt.cfg", temporary[PLATFORM_MAX_PATH], backup[PLATFORM_MAX_PATH];
    FormatEx(temporary, sizeof(temporary), "%s.menu-new", path); FormatEx(backup, sizeof(backup), "%s.menu-bak", path);
    File input = OpenFile(path, "r"); if (input == null) { Tell(client, "Could not read the main config. Nothing saved."); return false; }
    File output = OpenFile(temporary, "w"); if (output == null) { delete input; Tell(client, "Could not write the config. Live settings remain; nothing saved."); return false; }
    bool inside, seen, valid = true; char line[2048], trimmed[2048];
    while (input.ReadLine(line, sizeof(line)))
    {
        if (strlen(line) >= sizeof(line) - 1) { valid = false; break; }
        strcopy(trimmed, sizeof(trimmed), line); TrimString(trimmed);
        int handling = PremiumUiConfigLine(trimmed, inside, seen);
        if (handling < 0) { valid = false; break; }
        if (!handling) continue;
        ReplaceString(line, sizeof(line), "\n", ""); ReplaceString(line, sizeof(line), "\r", "");
        if (!output.WriteLine("%s", line)) { valid = false; break; }
    }
    if (inside || !input.EndOfFile()) valid = false;
    if (valid)
    {
        valid = output.WriteLine("\n// BEGIN PREMIUM MENU SETTINGS\n// Saved from !premium_admin. Values below override earlier numeric settings and presets.");
        for (int i; valid && i < sizeof(g_UiSettingNames); i++) if (g_UiSettingNumeric[i])
        { ConVar setting; if (!UiSetting(i, setting)) { valid = false; break; } valid = output.WriteLine("%s \"%.6f\"", g_UiSettingNames[i], setting.FloatValue); }
        if (valid) valid = output.WriteLine("// END PREMIUM MENU SETTINGS");
    }
    delete input; delete output;
    if (!valid) { DeleteFile(temporary); Tell(client, "Could not safely save this config. Original file kept; check its menu markers and disk space."); return false; }
    if ((FileExists(backup) && !DeleteFile(backup)) || !RenameFile(backup, path)) { DeleteFile(temporary); Tell(client, "Could not back up the config. Original file kept."); return false; }
    if (!RenameFile(path, temporary))
    { if (!RenameFile(path, backup)) LogError("Premium config replacement failed; restore %s to %s", backup, path); DeleteFile(temporary); Tell(client, "Could not replace the config. Check premium_rebuilt.cfg.menu-bak."); return false; }
    LogAction(client, -1, "Premium menu: saved numeric settings to %s", path);
    Tell(client, "Saved current numeric settings. They run last in the main cfg; previous file is .menu-bak. Storage/text settings were not changed.");
    return true;
}
