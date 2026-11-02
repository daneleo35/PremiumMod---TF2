public Action CommandBuildingOverview(int client, int args)
{
    DisplayBuildingOverview(client); return Plugin_Handled;
}

void DisplayBuildingOverview(int client)
{
    if (!ValidClient(client)) { Tell(client, "%t", "In Game Only"); return; }
    if (!cvEnable.BoolValue || !cvBuildingOverview.BoolValue) { Tell(client, "%t", "Feature Disabled"); return; }
    Menu menu = new Menu(BuildingOverviewSelected);
    menu.SetTitle("My buildings | %d/6\nHealth, level and sapper status\n ", CountOwnedBuildings(client, false));
    char classes[][] = {"obj_sentrygun", "obj_dispenser", "obj_teleporter"};
    int shown;
    for (int type; type < sizeof(classes); type++)
    {
        int entity = -1;
        while ((entity = FindEntityByClassname(entity, classes[type])) != -1)
        {
            if (!HasEntProp(entity, Prop_Send, "m_hBuilder") || GetEntPropEnt(entity, Prop_Send, "m_hBuilder") != client) continue;
            if (HasEntProp(entity, Prop_Send, "m_bPlacing") && GetEntProp(entity, Prop_Send, "m_bPlacing")) continue;
            char label[192], name[48];
            if (type == 0) strcopy(name, sizeof(name), IsBonusSentry(client, entity) ? "Extra sentry" : "Primary sentry");
            else if (type == 1) strcopy(name, sizeof(name), "Dispenser");
            else strcopy(name, sizeof(name), GetEntProp(entity, Prop_Send, "m_iObjectMode") == 0 ? "Teleporter entrance" : "Teleporter exit");
            int health = GetEntProp(entity, Prop_Send, "m_iHealth");
            int maximum = GetEntProp(entity, Prop_Send, "m_iMaxHealth");
            int level = GetEntProp(entity, Prop_Send, "m_iUpgradeLevel");
            bool sapped = HasEntProp(entity, Prop_Send, "m_bHasSapper") && GetEntProp(entity, Prop_Send, "m_bHasSapper");
            FormatEx(label, sizeof(label), "%s | %d/%d HP | L%d%s", name, health, maximum, level, sapped ? " | SAPPED" : " | No sapper");
            menu.AddItem("info", label, ITEMDRAW_DISABLED); shown++;
        }
    }
    if (!shown) menu.AddItem("info", "You have no placed buildings", ITEMDRAW_DISABLED);
    menu.AddItem("refresh", "Refresh status"); menu.ExitBackButton = true; menu.Display(client, 30);
}

public int BuildingOverviewSelected(Menu menu, MenuAction action, int client, int selection)
{
    if (action == MenuAction_End) delete menu;
    else if (action == MenuAction_Cancel && selection == MenuCancel_ExitBack && ValidClient(client)) UiHome(client);
    else if (action == MenuAction_Select && ValidClient(client)) DisplayBuildingOverview(client);
    return 0;
}
