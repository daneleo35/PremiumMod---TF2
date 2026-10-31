bool PremiumPlayerChatFormat(const char[] format)
{
    // Valve's CTFGameRules::GetChatFormat returns TF_Chat_All, etc., without '#'.
    // Some processors include the optional localization marker; accept both.
    int offset = format[0] == '#' ? 1 : 0;
    return StrContains(format[offset], "TF_Chat_") == 0;
}
