bool PremiumPlayerChatFormat(const char[] format)
{
    // Valve's CTFGameRules::GetChatFormat returns TF_Chat_All, etc., without '#'.
    // Some processors include the optional localization marker; accept both.
    int offset = format[0] == '#' ? 1 : 0;
    if (StrContains(format[offset], "TF_Chat_") != 0) return false;
    char phrase[40];
    return PremiumChatPhrase(format, phrase, sizeof(phrase));
}

bool PremiumChatPhrase(const char[] format, char[] phrase, int size)
{
    int offset = format[0] == '#' ? 1 : 0;
    char keys[][] = {"TF_Chat_All", "TF_Chat_AllDead", "TF_Chat_AllSpec", "TF_Chat_Team", "TF_Chat_Team_Dead", "TF_Chat_Team_Loc", "TF_Chat_Spec", "TF_Chat_Coach"};
    char phrases[][] = {"Chat Layout All", "Chat Layout All Dead", "Chat Layout All Spec", "Chat Layout Team", "Chat Layout Team Dead", "Chat Layout Team Location", "Chat Layout Spec", "Chat Layout Coach"};
    for (int i; i < sizeof(keys); i++) if (StrEqual(format[offset], keys[i]))
    { strcopy(phrase, size, phrases[i]); return true; }
    phrase[0] = '\0'; return false;
}

// Put colour in SayText2's layout, never its filtered player-name/text fields.
// Only these server-owned tokens become localization placeholders. User strings
// remain separate message parameters, including names containing '%' or '{...}'.
bool PremiumChatCompose(const char[] layout, const char[] nameToken, char[] output, int size)
{
    int nameAt = StrContains(layout, "@NAME@"), textAt = StrContains(layout, "@TEXT@");
    if (nameAt < 0 || textAt < 0 || StrContains(layout[nameAt + 1], "@NAME@") >= 0 || StrContains(layout[textAt + 1], "@TEXT@") >= 0) return false;
    char assembled[512];
    if (strlen(layout) >= sizeof(assembled) || strlen(layout) + strlen(nameToken) >= sizeof(assembled)) return false;
    strcopy(assembled, sizeof(assembled), layout);
    ReplaceString(assembled, sizeof(assembled), "@NAME@", nameToken);
    ReplaceString(assembled, sizeof(assembled), "@TEXT@", "%s2");
    ReplaceString(assembled, sizeof(assembled), "@LOCATION@", "%s3");
    if (strlen(assembled) >= size) return false;
    strcopy(output, size, assembled); return true;
}
