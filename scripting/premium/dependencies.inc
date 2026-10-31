// TF2Attributes is linked optionally only so our own diagnostic can run when
// it is missing. Gameplay is still gated on successful dependency validation.
bool PremiumAttributeNativesAvailable()
{
    char names[][] = {"TF2Attrib_IsReady", "TF2Attrib_GetByName", "TF2Attrib_GetValue",
        "TF2Attrib_SetByName", "TF2Attrib_RemoveByName", "TF2Attrib_ClearCache", "TF2Attrib_HookValueFloat"};
    for (int i; i < sizeof(names); i++)
        if (GetFeatureStatus(FeatureType_Native, names[i]) != FeatureStatus_Available) return false;
    return true;
}

bool PremiumCheckDependencies(char[] reason, int capacity)
{
    if (!LibraryExists("tf2attributes"))
    {
        strcopy(reason, capacity, "TF2Attributes is missing. Install tf2attributes.smx in addons/sourcemod/plugins and tf2.attributes.txt in addons/sourcemod/gamedata, then reload Premium. Download: https://github.com/FlaminSarge/tf2attributes/releases");
        return false;
    }
    if (!PremiumAttributeNativesAvailable())
    {
        strcopy(reason, capacity, "TF2Attributes is installed but its required API is unavailable or incompatible. Update/reload the plugin and matching gamedata, then reload Premium. Download: https://github.com/FlaminSarge/tf2attributes/releases");
        return false;
    }
    // Never call an optional native until its binding has been verified.
    if (!TF2Attrib_IsReady())
    {
        strcopy(reason, capacity, "TF2Attributes failed to initialize. Check its SourceMod error log and install gamedata matching your TF2 server build, then reload Premium. Download: https://github.com/FlaminSarge/tf2attributes/releases");
        return false;
    }
    reason[0] = '\0';
    return true;
}
