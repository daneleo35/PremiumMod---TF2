public Action CommandGiftMenu(int client, int args)
{
    UiGiftPlayers(client); return Plugin_Handled;
}

bool CanTransferPoints(int buyer, int target, int amount, char[] reason, int size)
{
    if (!ValidClient(target) || target == buyer || IsFakeClient(target) || !g_Economy[target].loaded)
    { strcopy(reason, size, "Choose another connected player whose account has loaded."); return false; }
    if (!cvPointsTransfers.BoolValue || g_Economy[buyer].purchasing || g_Economy[target].purchasing)
    { strcopy(reason, size, "Points gifting is disabled or a purchase is still settling."); return false; }
    if (amount < 1 || amount > cvTransferMax.IntValue || amount > g_Economy[buyer].points
        || (cvTransferDaily.IntValue > 0 && amount > cvTransferDaily.IntValue))
    { strcopy(reason, size, "The amount exceeds your balance or a configured transfer limit."); return false; }
    if (g_Economy[target].points > cvPointsCap.IntValue - amount)
    { strcopy(reason, size, "That gift would exceed the recipient's points cap."); return false; }
    reason[0] = '\0'; return true;
}

public Action CommandGiftPoints(int client, int args)
{
    if (!EconomyAvailable(client)) return Plugin_Handled;
    if (args == 0) { UiTransferPlayers(client); return Plugin_Handled; }
    char name[64], raw[24]; GetCmdArg(1, name, sizeof(name)); GetCmdArg(2, raw, sizeof(raw));
    if (args != 2 || !PremiumDigits(raw) || strlen(raw) > 7)
    { Tell(client, "Usage: !premium_giftpoints <player> <amount>"); return Plugin_Handled; }
    int target = FindTarget(client, name, true, false);
    if (target > 0) ConfirmPointsTransfer(client, target, StringToInt(raw));
    return Plugin_Handled;
}

void UiTransferPlayers(int client)
{
    if (!EconomyAvailable(client)) return;
    if (!cvPointsTransfers.BoolValue) { Tell(client, "Points gifting is switched off."); return; }
    Menu menu = new Menu(TransferPlayerSelected); menu.SetTitle("Gift points\nBalance: %d | Maximum per gift: %d\nDaily UTC limit: %d (0 unlimited)\n ", g_Economy[client].points, cvTransferMax.IntValue, cvTransferDaily.IntValue);
    int shown;
    for (int target = 1; target <= MaxClients; target++)
    {
        if (target == client || !ValidClient(target) || IsFakeClient(target) || !g_Economy[target].loaded) continue;
        char data[16], name[MAX_NAME_LENGTH]; IntToString(GetClientSerial(target), data, sizeof(data)); GetClientName(target, name, sizeof(name)); menu.AddItem(data, name); shown++;
    }
    if (!shown) menu.AddItem("info", "No other loaded players available", ITEMDRAW_DISABLED);
    menu.ExitBackButton = true; menu.Display(client, 45);
}

public int TransferPlayerSelected(Menu menu, MenuAction action, int client, int selection)
{
    if (action == MenuAction_End) delete menu;
    else if (action == MenuAction_Cancel && selection == MenuCancel_ExitBack && ValidClient(client)) UiShopHome(client);
    else if (action == MenuAction_Select && EconomyAvailable(client))
    {
        char data[16]; menu.GetItem(selection, data, sizeof(data)); int target = GetClientFromSerial(StringToInt(data));
        if (!ValidClient(target)) { Tell(client, "That player left."); return 0; }
        Menu amounts = new Menu(TransferAmountSelected); amounts.SetTitle("Gift points to %N\nChoose amount\nCustom: !premium_giftpoints name amount\n ", target);
        int choices[] = {1, 5, 10, 25, 50, 100, 250, 500, 1000}; int shown;
        for (int i; i <= sizeof(choices); i++)
        {
            int amount = i == sizeof(choices) ? cvTransferMax.IntValue : choices[i];
            bool duplicate;
            for (int j; j < sizeof(choices); j++) if (i == sizeof(choices) && choices[j] == amount) duplicate = true;
            if (duplicate) continue;
            char reason[192], item[40], label[48];
            if (!CanTransferPoints(client, target, amount, reason, sizeof(reason))) continue;
            FormatEx(item, sizeof(item), "%d:%d", GetClientSerial(target), amount); FormatEx(label, sizeof(label), "%d points", amount); amounts.AddItem(item, label); shown++;
        }
        if (!shown) amounts.AddItem("info", "No affordable amount within the limits", ITEMDRAW_DISABLED);
        amounts.ExitBackButton = true; amounts.Display(client, 45);
    }
    return 0;
}

bool TransferMenuItem(Menu menu, int selection, int &target, int &amount)
{
    char data[40], parts[2][20]; menu.GetItem(selection, data, sizeof(data));
    if (ExplodeString(data, ":", parts, sizeof(parts), sizeof(parts[])) != 2 || !PremiumDigits(parts[0]) || !PremiumDigits(parts[1])) return false;
    target = GetClientFromSerial(StringToInt(parts[0])); amount = StringToInt(parts[1]); return ValidClient(target);
}

public int TransferAmountSelected(Menu menu, MenuAction action, int client, int selection)
{
    if (action == MenuAction_End) delete menu;
    else if (action == MenuAction_Cancel && selection == MenuCancel_ExitBack && ValidClient(client)) UiTransferPlayers(client);
    else if (action == MenuAction_Select && EconomyAvailable(client))
    { int target, amount; if (TransferMenuItem(menu, selection, target, amount)) ConfirmPointsTransfer(client, target, amount); else Tell(client, "That player left. Nothing transferred."); }
    return 0;
}

void ConfirmPointsTransfer(int client, int target, int amount)
{
    char reason[192];
    if (!CanTransferPoints(client, target, amount, reason, sizeof(reason))) { Tell(client, "%s", reason); return; }
    Menu menu = new Menu(TransferConfirmed); char data[40];
    menu.SetTitle("Send %d points to %N?\nYour balance afterwards: %d\nDaily UTC limits are checked when saved\n ", amount, target, g_Economy[client].points - amount);
    FormatEx(data, sizeof(data), "%d:%d", GetClientSerial(target), amount); menu.AddItem(data, "Confirm points gift"); menu.ExitBackButton = true; menu.Display(client, 30);
}

public int TransferConfirmed(Menu menu, MenuAction action, int client, int selection)
{
    if (action == MenuAction_End) delete menu;
    else if (action == MenuAction_Cancel && selection == MenuCancel_ExitBack && ValidClient(client)) UiTransferPlayers(client);
    else if (action == MenuAction_Select && EconomyAvailable(client))
    {
        int target, amount; char reason[192];
        if (!TransferMenuItem(menu, selection, target, amount)) { Tell(client, "That player left. Nothing transferred."); return 0; }
        if (!CanTransferPoints(client, target, amount, reason, sizeof(reason))) { Tell(client, "%s", reason); return 0; }
        EconomyOperation op; NewEconomyOperation(op, Economy_Transfer, g_Economy[client].steam);
        strcopy(op.recipient, sizeof(op.recipient), g_Economy[target].steam); op.price = amount; op.transferDaily = cvTransferDaily.IntValue; op.buyerSerial = GetClientSerial(client);
        if (!QueueEconomyOperation(op)) { Tell(client, "%t", "Points Pending"); return 0; }
        g_Economy[client].purchasing = true; Tell(client, "Saving your points gift..."); ProcessEconomyOutbox();
    }
    return 0;
}

void UiGiftPlayers(int client)
{
    if (!EconomyAvailable(client)) return;
    if (!cvPointsShop.BoolValue || !cvPointsGifts.BoolValue || !cvPointsRentals.BoolValue || !g_ShopValid)
    { Tell(client, "Feature gifts are switched off or the shop is unavailable."); return; }
    Menu menu = new Menu(GiftPlayerSelected); menu.SetTitle("Gift a feature rental\nChoose a connected player\nYour balance: %d points\n ", g_Economy[client].points);
    int shown;
    for (int target = 1; target <= MaxClients; target++)
    {
        if (target == client || !ValidClient(target) || IsFakeClient(target) || !g_Economy[target].loaded || !g_Player[target].loaded) continue;
        char data[16], name[MAX_NAME_LENGTH]; IntToString(GetClientSerial(target), data, sizeof(data)); GetClientName(target, name, sizeof(name));
        menu.AddItem(data, name); shown++;
    }
    if (!shown) menu.AddItem("info", "No other loaded players are available", ITEMDRAW_DISABLED);
    menu.ExitBackButton = true; menu.Display(client, 45);
}

public int GiftPlayerSelected(Menu menu, MenuAction action, int client, int selection)
{
    if (action == MenuAction_End) delete menu;
    else if (action == MenuAction_Cancel && selection == MenuCancel_ExitBack && ValidClient(client)) UiShopHome(client);
    else if (action == MenuAction_Select && EconomyAvailable(client))
    {
        char data[16]; menu.GetItem(selection, data, sizeof(data));
        int target = GetClientFromSerial(StringToInt(data));
        if (!ValidClient(target)) { Tell(client, "That player left. Nothing purchased."); UiGiftPlayers(client); return 0; }
        Menu offers = new Menu(GiftOfferSelected); offers.SetTitle("Gift to %N\nChoose a rental\n ", target);
        int shown;
        for (int index; index < g_ShopCount; index++)
        {
            if (g_ShopOffers[index].feature < 0) continue;
            char item[80], label[192], duration[32], reason[192];
            int price = ShopOfferPrice(index); PremiumUiDuration(g_ShopOffers[index].duration, duration, sizeof(duration));
            bool available = ShopCanBuyFor(client, target, index, true, reason, sizeof(reason));
            FormatEx(item, sizeof(item), "%d:%d:%d:%d", GetClientSerial(target), g_ShopRevision, index, price);
            FormatEx(label, sizeof(label), "%s | %d points / %s%s", g_ShopOffers[index].name, price, duration, available ? "" : " (view limits)");
            offers.AddItem(item, label); shown++;
        }
        if (!shown) offers.AddItem("info", "No individual rentals are configured", ITEMDRAW_DISABLED);
        offers.ExitBackButton = true; offers.Display(client, 45);
    }
    return 0;
}

bool GiftMenuItem(Menu menu, int selection, int &target, int &index, int &price)
{
    char data[80], parts[4][20]; menu.GetItem(selection, data, sizeof(data));
    if (ExplodeString(data, ":", parts, sizeof(parts), sizeof(parts[])) != 4) return false;
    for (int i; i < 4; i++) if (!PremiumDigits(parts[i])) return false;
    target = GetClientFromSerial(StringToInt(parts[0])); index = StringToInt(parts[2]); price = StringToInt(parts[3]);
    return ValidClient(target) && StringToInt(parts[1]) == g_ShopRevision && index >= 0 && index < g_ShopCount && price == ShopOfferPrice(index);
}

public int GiftOfferSelected(Menu menu, MenuAction action, int client, int selection)
{
    if (action == MenuAction_End) delete menu;
    else if (action == MenuAction_Cancel && selection == MenuCancel_ExitBack && ValidClient(client)) UiGiftPlayers(client);
    else if (action == MenuAction_Select && EconomyAvailable(client))
    {
        int target, index, price;
        if (!GiftMenuItem(menu, selection, target, index, price)) { Tell(client, "The recipient, offer or price changed. Choose again."); UiGiftPlayers(client); return 0; }
        char reason[192], duration[32], data[80];
        bool available = ShopCanBuyFor(client, target, index, true, reason, sizeof(reason));
        PremiumUiDuration(g_ShopOffers[index].duration, duration, sizeof(duration));
        Menu confirm = new Menu(GiftPurchaseSelected);
        confirm.SetTitle("Gift %s to %N?\n%d points for %s\nYour balance afterwards: %d\nOnly this feature; no Premium tag\nOffline time counts\n ", g_ShopOffers[index].name, target, price, duration, g_Economy[client].points - price);
        if (!available) confirm.AddItem("info", reason, ITEMDRAW_DISABLED);
        FormatEx(data, sizeof(data), "%d:%d:%d:%d", GetClientSerial(target), g_ShopRevision, index, price);
        confirm.AddItem(data, "Confirm gift purchase", available ? ITEMDRAW_DEFAULT : ITEMDRAW_DISABLED);
        confirm.ExitBackButton = true; confirm.Display(client, 30);
    }
    return 0;
}

public int GiftPurchaseSelected(Menu menu, MenuAction action, int client, int selection)
{
    if (action == MenuAction_End) delete menu;
    else if (action == MenuAction_Cancel && selection == MenuCancel_ExitBack && ValidClient(client)) UiGiftPlayers(client);
    else if (action == MenuAction_Select && EconomyAvailable(client))
    {
        int target, index, price; char reason[192];
        if (!GiftMenuItem(menu, selection, target, index, price)) { Tell(client, "The recipient, offer or price changed. Nothing purchased."); UiGiftPlayers(client); return 0; }
        if (!ShopCanBuyFor(client, target, index, true, reason, sizeof(reason))) { Tell(client, "%s", reason); UiGiftPlayers(client); return 0; }
        EconomyOperation op; NewEconomyOperation(op, Economy_Gift, g_Economy[client].steam);
        strcopy(op.recipient, sizeof(op.recipient), g_Economy[target].steam);
        op.feature = g_ShopOffers[index].feature; op.price = price; op.duration = g_ShopOffers[index].duration; op.buyerSerial = GetClientSerial(client);
        if (!QueueEconomyOperation(op)) { Tell(client, "%t", "Points Pending"); return 0; }
        g_Economy[client].purchasing = true;
        Tell(client, "Saving your gift for %N...", target); ProcessEconomyOutbox();
    }
    return 0;
}
