bool ShopMenuItem(Menu menu, int selection, int &index, bool confirmPrice = false)
{
    char data[64], parts[3][32]; menu.GetItem(selection, data, sizeof(data));
    if (ExplodeString(data, ":", parts, sizeof(parts), sizeof(parts[])) != 3 || !PremiumDigits(parts[0]) || !PremiumDigits(parts[1]) || !PremiumDigits(parts[2])
        || StringToInt(parts[0]) != g_ShopRevision) return false;
    index = StringToInt(parts[1]); return index >= 0 && index < g_ShopCount && (!confirmPrice || StringToInt(parts[2]) == ShopOfferPrice(index));
}
