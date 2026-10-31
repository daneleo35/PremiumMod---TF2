#include "chat_policy.inc"
UserMsg g_ChatMessage = INVALID_MESSAGE_ID;
bool g_SendingTaggedChat;
int g_ChatSeen[MAXPLAYERS + 1], g_ChatTagged[MAXPLAYERS + 1];
char g_LastChatFormat[MAXPLAYERS + 1][64];

void ResetChatDiagnostics(int client)
{
    g_ChatSeen[client] = 0; g_ChatTagged[client] = 0; g_LastChatFormat[client][0] = '\0';
}

void InitializeChatTags()
{
    g_ChatMessage = GetUserMessageId("SayText2");
    if (g_ChatMessage == INVALID_MESSAGE_ID || GetUserMessageType() != UM_BitBuf)
    {
        LogError("Premium chat tags require TF2's SayText2 bitbuffer message; tagging is unavailable.");
        return;
    }
    HookUserMessage(g_ChatMessage, InterceptPremiumChat, true);
}

bool PrefixPremiumName(int client, char[] name, int capacity)
{
    if (!HasPremium(client) || !cvChatTags.BoolValue || g_Options[client].hideTag) return false;
    char tag[96], plain[96];
    if (FlagPremium(client) || g_Player[client].member) cvMemberTag.GetString(tag, sizeof(tag));
    else cvTrialTag.GetString(tag, sizeof(tag));
    int style = g_Options[client].tagStyle;
    if (style > 0 && (cvTagStyles.IntValue & (1 << (style - 1))))
    {
        // Presets only recolour the owner's configured tag; they cannot spoof a tier.
        CReplaceColorCodes(tag, client, true, sizeof(tag));
        char text[64]; strcopy(text, sizeof(text), tag);
        if (style == 1) FormatEx(tag, sizeof(tag), "{teamcolor}%s{default}", text);
        else if (style == 2) FormatEx(tag, sizeof(tag), "{gold}%s{default}", text);
        else strcopy(tag, sizeof(tag), text);
    }
    strcopy(plain, sizeof(plain), tag);
    CReplaceColorCodes(plain, client, true, sizeof(plain));
    if (!plain[0] || StrContains(name, plain) >= 0) return false;
    ReplaceString(tag, sizeof(tag), "\n", "");
    ReplaceString(tag, sizeof(tag), "\r", "");
    CReplaceColorCodes(tag, client, false, sizeof(tag));
    // Never truncate an existing player name to make room for a tag.
    if (strlen(tag) + strlen(name) + 3 > capacity) return false;
    char original[128]; strcopy(original, sizeof(original), name);
    FormatEx(name, capacity, "%s \x03%s", tag, original);
    g_ChatTagged[client]++;
    return true;
}

// Prefer existing chat processors when present so their recipient/mute/format
// policies remain authoritative. Both forwards are optional public callbacks.
public Action OnChatMessage(int &author, Handle recipients, char[] name, char[] message)
{
    return PrefixPremiumName(author, name, 64) ? Plugin_Changed : Plugin_Continue;
}

public Action CP_OnChatMessage(int &author, ArrayList recipients, char[] flagstring,
    char[] name, char[] message, bool &processcolors, bool &removecolors)
{
    return PrefixPremiumName(author, name, 64) ? Plugin_Changed : Plugin_Continue;
}

bool ExistingChatProcessorActive()
{
    if (LibraryExists("scp")) return true;
    if (!LibraryExists("chat-processor")) return false;
    ConVar enabled = FindConVar("sm_chatprocessor_status");
    return enabled == null || enabled.BoolValue;
}

#if !defined PREMIUM_CHAT_TRANSLATOR_TEST
bool PremiumChatTranslate(const char[] phrase, int recipient, char[] layout, int size)
{
    if (!TranslationPhraseExists(phrase)) return false;
    FormatEx(layout, size, "%T", phrase, recipient);
    return true;
}
#endif

bool BuildPremiumChatFormat(int author, int recipient, const char[] nativeFormat, const char[] nameToken, char[] output, int size)
{
    char phrase[40], layout[256];
    if (!PremiumChatPhrase(nativeFormat, phrase, sizeof(phrase)) || !PremiumChatTranslate(phrase, recipient, layout, sizeof(layout))) return false;
    CReplaceColorCodes(layout, author, false, sizeof(layout));
    return PremiumChatCompose(layout, nameToken, output, size);
}

public Action InterceptPremiumChat(UserMsg id, BfRead message, const int[] players, int count, bool reliable, bool init)
{
    if (g_SendingTaggedChat || !cvChatTags.BoolValue
        || ExistingChatProcessorActive()) return Plugin_Continue;
    int client = message.ReadByte();
    int chat = message.ReadByte();
    char format[64]; message.ReadString(format, sizeof(format));
    if (ValidClient(client)) { g_ChatSeen[client]++; strcopy(g_LastChatFormat[client], sizeof(g_LastChatFormat[]), format); }
    if (!HasPremium(client) || g_Options[client].hideTag || !PremiumPlayerChatFormat(format) || count <= 0) return Plugin_Continue;
    char parameters[4][256];
    for (int i; i < 4; i++) message.ReadString(parameters[i], sizeof(parameters[]));
    DataPack pending = new DataPack();
    pending.WriteCell(GetClientSerial(client));
    pending.WriteCell(chat);
    pending.WriteCell(reliable);
    pending.WriteString(format);
    for (int i; i < 4; i++) pending.WriteString(parameters[i]);
    pending.WriteCell(count);
    for (int i; i < count; i++) pending.WriteCell(ValidClient(players[i]) ? GetClientSerial(players[i]) : 0);
    RequestFrame(SendPremiumChat, pending);
    return Plugin_Handled;
}

public void SendPremiumChat(any data)
{
    DataPack pending = view_as<DataPack>(data);
    pending.Reset();
    int client = GetClientFromSerial(pending.ReadCell());
    int chat = pending.ReadCell();
    bool reliable = pending.ReadCell() != 0;
    char format[64], parameters[4][256];
    pending.ReadString(format, sizeof(format));
    for (int i; i < 4; i++) pending.ReadString(parameters[i], sizeof(parameters[]));
    int count = pending.ReadCell(), recipients[MAXPLAYERS], valid;
    for (int i; i < count; i++)
    {
        int serial = pending.ReadCell();
        int recipient = serial ? GetClientFromSerial(serial) : 0;
        if (ValidClient(recipient) && valid < sizeof(recipients)) recipients[valid++] = recipient;
    }
    delete pending;
    if (g_Stopping || !ValidClient(client) || valid == 0) return;
    // TF2 sanitizes colour controls from parameters[0] and parameters[1].
    // Keep them byte-for-byte; colour the layout around its %s1 placeholder.
    char nameToken[128] = "%s1";
    bool tagged = PrefixPremiumName(client, nameToken, sizeof(nameToken));
    int overhead = 2 + 1;
    for (int i; i < 4; i++) overhead += strlen(parameters[i]) + 1;
    int layoutBudget = 255 - overhead;
    g_SendingTaggedChat = true;
    // Resolve our chat labels in each recipient's language. Recipient serials,
    // chat flag, speaker and all four original parameters remain authoritative.
    for (int i; i < valid; i++)
    {
        char taggedFormat[512];
        bool useTag = tagged && BuildPremiumChatFormat(client, recipients[i], format, nameToken, taggedFormat, sizeof(taggedFormat)) && strlen(taggedFormat) <= layoutBudget;
        int recipient[1]; recipient[0] = recipients[i];
        SendPremiumChatPayload(client, chat, useTag ? taggedFormat : format, parameters, recipient, 1, reliable);
    }
    g_SendingTaggedChat = false;
}

void SendPremiumChatPayload(int client, int chat, const char[] format, const char[][] parameters, const int[] recipients, int valid, bool reliable)
{
    BfWrite output = view_as<BfWrite>(StartMessageEx(g_ChatMessage, recipients, valid, reliable ? USERMSG_RELIABLE : 0));
    if (output == null) return;
    output.WriteByte(client);
    output.WriteByte(chat);
    output.WriteString(format);
    for (int i; i < 4; i++) output.WriteString(parameters[i]);
    EndMessage();
}

public Action CommandChatDiagnostics(int client, int args)
{
    if (!ValidClient(client)) { Tell(client, "%t", "In Game Only"); return Plugin_Handled; }
    PrintToConsole(client, "Premium %s chat: premium=%d enabled=%d hidden=%d style=%d processor=%d message-id=%d seen=%d tagged=%d last-format=%s",
        VERSION, HasPremium(client), cvChatTags.BoolValue, g_Options[client].hideTag, g_Options[client].tagStyle,
        ExistingChatProcessorActive(), g_ChatMessage, g_ChatSeen[client], g_ChatTagged[client],
        g_LastChatFormat[client][0] ? g_LastChatFormat[client] : "none");
    Tell(client, "Chat diagnostics printed in your console. Send a normal chat message first.");
    return Plugin_Handled;
}
