#pragma semicolon 1
#pragma newdecls required
#include <sourcemod>
#include <sdktools>
enum struct TestSetting
{
    bool BoolValue;
    int IntValue;
    char Text[96];
    void GetString(char[] output, int size) { strcopy(output, size, this.Text); }
}
enum struct TestPlayer
{
    bool member;
}
enum struct TestOptions
{
    bool hideTag;
    int tagStyle;
}
TestSetting cvChatTags, cvTagStyles, cvMemberTag, cvTrialTag;
TestPlayer g_Player[MAXPLAYERS + 1];
TestOptions g_Options[MAXPLAYERS + 1];
bool g_Stopping, g_Access = true, g_Flag = true;
int g_Checks;
#define VERSION "test"
bool ValidClient(int client) { return client == 1; }
bool HasPremium(int client) { return ValidClient(client) && g_Access; }
bool FlagPremium(int client) { return ValidClient(client) && g_Flag; }
void Tell(int client, const char[] format, any ...) { if (client < 0 || !format[0]) ThrowError("bad reply"); }
void TestColors(char[] text, int client, bool strip = false, int size = 256)
{
    if (client != 1) ThrowError("bad colour author");
    // More Colors uses RGB hex even for named green; the old test's x04 mock
    // hid the exact six-digit leak reported from a real TF2 client.
    ReplaceString(text, size, "{green}", strip ? "" : "\x073EFF3E");
    ReplaceString(text, size, "{default}", strip ? "" : "\x01");
    ReplaceString(text, size, "{teamcolor}", strip ? "" : "\x03");
    ReplaceString(text, size, "{gold}", strip ? "" : "\x07FFD700");
}
bool g_TranslateAvailable = true, g_TranslateInvalid;
bool PremiumChatTranslate(const char[] phrase, int recipient, char[] layout, int size)
{
    if (recipient != 1 || !g_TranslateAvailable) return false;
    if (g_TranslateInvalid) { strcopy(layout, size, "@NAME@ without message marker"); return true; }
    if (StrEqual(phrase, "Chat Layout All")) strcopy(layout, size, "{default}@NAME@{default} : @TEXT@");
    else if (StrEqual(phrase, "Chat Layout All Dead")) strcopy(layout, size, "{default}*DEAD* @NAME@{default} : @TEXT@");
    else if (StrEqual(phrase, "Chat Layout All Spec")) strcopy(layout, size, "{default}*SPEC* @NAME@{default} : @TEXT@");
    else if (StrEqual(phrase, "Chat Layout Team")) strcopy(layout, size, "{default}(TEAM) @NAME@{default} : @TEXT@");
    else if (StrEqual(phrase, "Chat Layout Team Dead")) strcopy(layout, size, "{default}*DEAD*(TEAM) @NAME@{default} : @TEXT@");
    else if (StrEqual(phrase, "Chat Layout Team Location")) strcopy(layout, size, "{default}(TEAM) @NAME@{default} @LOCATION@ : @TEXT@");
    else if (StrEqual(phrase, "Chat Layout Spec")) strcopy(layout, size, "{default}(Spectator) @NAME@{default} : @TEXT@");
    else strcopy(layout, size, "{default}(Coach) @NAME@{default} : @TEXT@");
    return true;
}
#define PREMIUM_CHAT_TRANSLATOR_TEST
#define CReplaceColorCodes TestColors
#include "../scripting/premium/chat.inc"
#pragma unused InitializeChatTags
void Check(bool condition, const char[] label) { g_Checks++; if (!condition) ThrowError("FAIL: %s", label); }

public int main()
{
    char formats[][] = {"TF_Chat_All", "TF_Chat_AllDead", "TF_Chat_AllSpec", "TF_Chat_Team", "TF_Chat_Team_Dead", "TF_Chat_Team_Loc", "TF_Chat_Spec", "TF_Chat_Coach"};
    char marked[64];
    for (int i; i < sizeof(formats); i++)
    {
        Check(PremiumPlayerChatFormat(formats[i]), "actual Valve native format must pass, without localization marker");
        FormatEx(marked, sizeof(marked), "#%s", formats[i]);
        Check(PremiumPlayerChatFormat(marked), "optional localization marker also accepted");
    }
    Check(!PremiumPlayerChatFormat("") && !PremiumPlayerChatFormat("#"), "empty formats rejected");
    Check(!PremiumPlayerChatFormat("Hello TF_Chat_All") && !PremiumPlayerChatFormat("#TF_playerid_object"), "unrelated text/UI messages ignored");
    cvChatTags.BoolValue = true; cvTagStyles.IntValue = 7;
    strcopy(cvMemberTag.Text, sizeof(cvMemberTag.Text), "{green}[Premium]{default}");
    strcopy(cvTrialTag.Text, sizeof(cvTrialTag.Text), "{green}[Trial]{default}");
    char name[128] = "Dane";
    Check(PrefixPremiumName(1, name, sizeof(name)), "flag member receives a chat prefix");
    Check(StrContains(name, "[Premium]") >= 0 && StrContains(name, "Dane") >= 0, "prefix and original name are preserved");
    Check(!PrefixPremiumName(1, name, sizeof(name)), "already-prefixed name is not tagged twice");
    g_Flag = false; g_Player[1].member = true; strcopy(name, sizeof(name), "Dane");
    Check(PrefixPremiumName(1, name, sizeof(name)) && StrContains(name, "[Premium]") >= 0, "database member receives Premium tag");
    g_Player[1].member = false; strcopy(name, sizeof(name), "Dane");
    Check(PrefixPremiumName(1, name, sizeof(name)) && StrContains(name, "[Trial]") >= 0, "trial keeps distinct membership tag");
    g_Options[1].hideTag = true; strcopy(name, sizeof(name), "Dane");
    Check(!PrefixPremiumName(1, name, sizeof(name)) && StrEqual(name, "Dane"), "personal hide-tag choice respected");
    g_Options[1].hideTag = false; cvChatTags.BoolValue = false;
    Check(!PrefixPremiumName(1, name, sizeof(name)), "server tag disable respected");
    cvChatTags.BoolValue = true; g_Access = false;
    Check(!PrefixPremiumName(1, name, sizeof(name)), "non-members cannot acquire tags");
    g_Access = true; g_Flag = true; strcopy(name, sizeof(name), "Dane");
    Check(!PrefixPremiumName(1, name, 8) && StrEqual(name, "Dane"), "tight packet/name budget preserves full original name");
    for (int style = 1; style <= 3; style++)
    {
        g_Options[1].tagStyle = style; strcopy(name, sizeof(name), "Dane");
        Check(PrefixPremiumName(1, name, sizeof(name)) && StrContains(name, "[Premium]") >= 0, "approved styles preserve membership label");
    }
    cvTagStyles.IntValue = 0; strcopy(name, sizeof(name), "Dane");
    Check(PrefixPremiumName(1, name, sizeof(name)) && name[0] == '\x07', "disallowed style falls back to server default");
    g_Options[1].tagStyle = 0; cvTagStyles.IntValue = 7;
    char nameToken[128] = "%s1", layout[512];
    Check(PrefixPremiumName(1, nameToken, sizeof(nameToken)), "native fallback colours a server-owned name placeholder");
    Check(BuildPremiumChatFormat(1, 1, "TF_Chat_All", nameToken, layout, sizeof(layout)), "known all-chat layout builds");
    Check(StrContains(layout, "\x073EFF3E[Premium]") >= 0, "real six-digit More Colors value stays attached to its control byte in layout");
    Check(StrContains(layout, "%s1") >= 0 && StrContains(layout, "%s2") >= 0, "name and message remain separate native parameters");
    for (int i; i < sizeof(formats); i++)
    {
        Check(BuildPremiumChatFormat(1, 1, formats[i], nameToken, layout, sizeof(layout)), "all native chat contexts supported");
        FormatEx(marked, sizeof(marked), "#%s", formats[i]);
        Check(BuildPremiumChatFormat(1, 1, marked, nameToken, layout, sizeof(layout)), "localization-marker variants supported");
    }
    BuildPremiumChatFormat(1, 1, "TF_Chat_Team_Dead", nameToken, layout, sizeof(layout));
    Check(StrContains(layout, "*DEAD*(TEAM)") >= 0, "dead team context retained");
    BuildPremiumChatFormat(1, 1, "TF_Chat_Team_Loc", nameToken, layout, sizeof(layout));
    Check(StrContains(layout, "%s3") >= 0, "location remains a client-localized parameter");
    Check(!BuildPremiumChatFormat(1, 1, "TF_Chat_Unknown", nameToken, layout, sizeof(layout)), "unknown native format left unchanged");
    Check(!PremiumPlayerChatFormat("TF_Chat_All_extra"), "prefix lookalike is not a supported native chat context");
    Check(!BuildPremiumChatFormat(1, 1, "TF_Chat_All", nameToken, layout, 8), "small layout budget refuses truncation");
    Check(!PremiumChatCompose("@NAME@ @NAME@ @TEXT@", nameToken, layout, sizeof(layout)), "duplicate name markers rejected");
    Check(!PremiumChatCompose("@NAME@ @TEXT@ @TEXT@", nameToken, layout, sizeof(layout)), "duplicate text markers rejected");
    Check(!PremiumChatCompose("@NAME@ only", nameToken, layout, sizeof(layout)), "missing text marker rejected");
    g_TranslateAvailable = false;
    Check(BuildPremiumChatFormat(1, 1, "TF_Chat_All", nameToken, layout, sizeof(layout)), "old/missing translations retain the membership tag through built-in layout");
    Check(StrContains(layout, "\x073EFF3E[Premium]") >= 0 && StrContains(layout, "%s1") >= 0 && StrContains(layout, "%s2") >= 0, "fallback keeps colour in layout and user data in native parameters");
    for (int i; i < sizeof(formats); i++)
    {
        Check(BuildPremiumChatFormat(1, 1, formats[i], nameToken, layout, sizeof(layout)), "all eight contexts work without newly installed translations");
        Check(StrContains(layout, "[Premium]") >= 0, "fallback cannot silently discard the prepared tag");
    }
    Check(g_ChatDefaultLayouts[1] >= 9, "diagnostics count built-in layout use");
    Check(!BuildPremiumChatFormat(1, 1, "TF_Chat_Unknown", nameToken, layout, sizeof(layout)), "fallback does not rewrite unknown message formats");
    Check(!BuildPremiumChatFormat(1, 1, "TF_Chat_All", nameToken, layout, 8), "built-in fallback cannot bypass a small output budget");
    g_TranslateAvailable = true;
    g_TranslateInvalid = true;
    Check(BuildPremiumChatFormat(1, 1, "TF_Chat_All", nameToken, layout, sizeof(layout)) && StrContains(layout, "%s2") >= 0, "invalid optional layout falls back without losing chat text");
    g_TranslateInvalid = false;
    Check(!PrefixPremiumName(0, name, sizeof(name)) && !PrefixPremiumName(-1, name, sizeof(name)), "invalid chat authors cannot index diagnostic arrays");
    for (int style; style <= 3; style++)
    {
        g_Options[1].tagStyle = style; strcopy(nameToken, sizeof(nameToken), "%s1");
        Check(PrefixPremiumName(1, nameToken, sizeof(nameToken)) && BuildPremiumChatFormat(1, 1, "TF_Chat_All", nameToken, layout, sizeof(layout)), "all personal styles use colour-safe native layout");
    }
    strcopy(cvMemberTag.Text, sizeof(cvMemberTag.Text), ""); strcopy(name, sizeof(name), "Dane");
    Check(!PrefixPremiumName(1, name, sizeof(name)) && StrEqual(name, "Dane"), "empty configured tag remains empty");
    Check(g_ChatTagged[1] > 0, "prefix diagnostics record successful transformations");
    ResetChatDiagnostics(1);
    Check(g_ChatTagged[1] == 0 && g_ChatSeen[1] == 0 && !g_LastChatFormat[1][0] && g_ChatDefaultLayouts[1] == 0 && g_ChatSent[1] == 0 && !g_LastChatResult[1][0], "new client session resets all preparation/delivery diagnostics");
    PrintToServer("PASS: %d native chat tag assertions", g_Checks);
    return g_Checks;
}
