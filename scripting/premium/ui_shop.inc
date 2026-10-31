int UiShopCategory(int feature)
{
    if (feature < 0) return 3;
    if (feature == view_as<int>(Feature_Metal) || feature == view_as<int>(Feature_Sentry) || feature == view_as<int>(Feature_Dispenser) || feature == view_as<int>(Feature_Teleporter)) return 2;
    return (PremiumGameplayFeatures() & (1 << feature)) ? 1 : 0;
}

void UiShopHome(int client)
{
    if (!EconomyAvailable(client)) { if (ValidClient(client)) UiWallet(client); return; }
    if (!cvPointsShop.BoolValue || !g_ShopValid) { Tell(client, "The shop is currently unavailable."); UiWallet(client); return; }
    Menu menu = new Menu(UiShopHomeSelected); menu.SetTitle("Points shop\nYour balance: %d points\nChoose a category\n ", g_Economy[client].points);
    menu.AddItem("0", "Cosmetics"); menu.AddItem("1", "Gameplay features"); menu.AddItem("2", "Engineer features");
    menu.AddItem("3", cvPointsTrials.BoolValue ? "Premium trials" : "Premium trials (switched off)");
    menu.AddItem("wallet", "My balance & active purchases");
    menu.ExitBackButton = true; menu.Display(client, 60);
}
public int UiShopHomeSelected(Menu menu, MenuAction action, int client, int selection)
{
    if (action == MenuAction_End) delete menu;
    else if (action == MenuAction_Cancel && selection == MenuCancel_ExitBack && ValidClient(client)) UiHome(client);
    else if (action == MenuAction_Select && ValidClient(client))
    { char data[12]; int category; menu.GetItem(selection, data, sizeof(data)); if (StrEqual(data, "wallet")) UiWallet(client); else if (PremiumUiIndex(data, 4, category)) UiShopOffers(client, category); }
    return 0;
}
void UiShopOffers(int client, int category)
{
    if (!EconomyAvailable(client) || category < 0 || category > 3) return;
    if (!cvPointsShop.BoolValue || !g_ShopValid) { UiShopHome(client); return; }
    char categories[][] = {"Cosmetics", "Gameplay features", "Engineer features", "Premium trials"};
    Menu menu = new Menu(PointsShopSelected); menu.SetTitle("%s\nBalance: %d points\nChoose an offer to see its details\n ", categories[category], g_Economy[client].points);
    int shown;
    for (int i; i < g_ShopCount; i++)
    {
        if (UiShopCategory(g_ShopOffers[i].feature) != category) continue;
        char data[64], label[192], duration[32], reason[192]; PremiumUiDuration(g_ShopOffers[i].duration, duration, sizeof(duration));
        FormatEx(data, sizeof(data), "%d:%d", g_ShopRevision, i);
        bool available = ShopCanBuy(client, i, reason, sizeof(reason));
        FormatEx(label, sizeof(label), "%s | %d points / %s%s", g_ShopOffers[i].name, g_ShopOffers[i].price, duration, available ? "" : " (view limits)");
        menu.AddItem(data, label); shown++;
    }
    if (!shown) menu.AddItem("empty", "No offers in this category", ITEMDRAW_DISABLED);
    menu.ExitBackButton = true; menu.Display(client, 60);
}
