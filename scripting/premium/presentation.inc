void ResetPresentation(int client) { StopTurretTooltip(client); }
void RefreshEngineerHud(int client) { RefreshTurretTooltip(client); }

int TooltipAnnotationId(int client) { return 0x50520000 + client; }
float TooltipHostClock() { return GetEngineTime(); }

int TooltipHostTarget(int client)
{
    if ((!HasFeature(client, Feature_Sentry) && !HasFeature(client, Feature_Dispenser) && !HasFeature(client, Feature_Teleporter)) || !IsPlayerAlive(client) || TF2_GetPlayerClass(client) != TFClass_Engineer
        || !cvEngineerHud.BoolValue || !g_Options[client].engineerHud) return INVALID_ENT_REFERENCE;
    int target = GetClientAimTarget(client, false);
    if (target <= MaxClients || !IsValidEntity(target)) return INVALID_ENT_REFERENCE;
    int kind = OwnedUtilityKind(client, EntIndexToEntRef(target));
    if (!PlacedSentry(client, target) && (kind < 0 || !PlacedUtility(client, target, kind))) return INVALID_ENT_REFERENCE;
    float eye[3], origin[3]; GetClientEyePosition(client, eye); GetEntPropVector(target, Prop_Send, "m_vecOrigin", origin);
    return GetVectorDistance(eye, origin) <= 600.0 ? EntIndexToEntRef(target) : INVALID_ENT_REFERENCE;
}

void TooltipHostText(int client, int reference, char[] text, int size)
{
    int target = EntRefToEntIndex(reference);
    int kind = OwnedUtilityKind(client, reference);
    if (kind >= 0)
    {
        FormatEx(text, size, "%s: %d/%d | Buildings: %d/6", kind == 0 ? "Dispensers" : (kind == 1 ? "Entrances" : "Exits"),
            CountUtilities(client, kind), UtilityLimit(client, kind), CountOwnedBuildings(client, false));
        return;
    }
    int cap = ExtraSentryAllowed(client) ? cvSentryLimit.IntValue : 1;
    char role[32];
    if (cvSentryLabels.BoolValue)
    {
        if (!IsBonusSentry(client, target)) strcopy(role, sizeof(role), "Primary | ");
        else for (int slot; slot < 3; slot++) if (g_ExtraSentries[client][slot] == reference)
            FormatEx(role, sizeof(role), "Extra %d (L1) | ", slot + 1);
    }
    FormatEx(text, size, "%s%d/%d sentries | Buildings: %d/6", role, CountOwnedBuildings(client), cap, CountOwnedBuildings(client, false));
}

bool TooltipHostShow(int client, int reference, const char[] text)
{
    if (!ValidClient(client) || (!OwnedExtraSentry(client, reference) && OwnedUtilityKind(client, reference) < 0)) return false;
    int target = EntRefToEntIndex(reference);
    Event event = CreateEvent("show_annotation", true);
    if (event == null) return false;
    float origin[3]; GetEntPropVector(target, Prop_Send, "m_vecOrigin", origin);
    event.SetInt("id", TooltipAnnotationId(client));
    event.SetString("text", text);
    event.SetInt("follow_entindex", target);
    event.SetFloat("worldPosX", origin[0]); event.SetFloat("worldPosY", origin[1]); event.SetFloat("worldPosZ", origin[2] + 66.0);
    event.SetFloat("lifetime", 2.0);
    // Only this owner receives the event. No 32-bit client visibility mask,
    // so client slots above 31 work without shifting/overflowing a bitfield.
    event.SetInt("visibilityBitfield", 0);
    event.SetBool("show_distance", false); event.SetBool("show_effect", false); event.SetString("play_sound", "");
    event.FireToClient(client); event.Cancel();
    return true;
}

void TooltipHostHide(int client)
{
    if (!ValidClient(client)) return;
    Event event = CreateEvent("hide_annotation", true);
    if (event == null) return;
    event.SetInt("id", TooltipAnnotationId(client));
    event.FireToClient(client); event.Cancel();
}

bool SwapAvailability(int client, char[] reason, int size)
{
    if (!ValidClient(client)) { strcopy(reason, size, "Use this in game."); return false; }
    if (!HasFeature(client, Feature_Swap))
    { strcopy(reason, size, "Team-swap feature access is required."); return false; }
    if (!GameplayAllowed(client)) { strcopy(reason, size, "Gameplay perks are disabled for your team."); return false; }
    if (!cvSwap.BoolValue) { strcopy(reason, size, "Premium team swapping is disabled by the server."); return false; }
    int team = GetClientTeam(client);
    if (team < 2 || GameRules_GetProp("m_bPlayingMannVsMachine"))
    { strcopy(reason, size, "Team swapping is unavailable in your team/game mode."); return false; }
    if (!PremiumRespawnPhaseAllowed(GameRules_GetRoundState(), g_RoundActive))
    { strcopy(reason, size, "Wait for an active round or its preparation period."); return false; }
    float remaining = g_Player[client].nextSwap - GetGameTime();
    if (remaining > 0.0) { FormatEx(reason, size, "Team swap available in %d seconds.", RoundToCeil(remaining)); return false; }
    if (!PremiumSwapBalanced(GetTeamClientCount(team), GetTeamClientCount(team == 2 ? 3 : 2), cvSwapDifference.IntValue))
    { FormatEx(reason, size, "Swap blocked: it would exceed the team-size difference of %d.", cvSwapDifference.IntValue); return false; }
    strcopy(reason, size, "Team swap is available now. Use !swapteam.");
    return true;
}

public Action CommandSwapStatus(int client, int args)
{
    char reason[192]; SwapAvailability(client, reason, sizeof(reason)); Tell(client, "%s", reason);
    return Plugin_Handled;
}

void AddNumericBenefit(Menu menu, const char[] format, any ...)
{
    char text[192]; VFormat(text, sizeof(text), format, 3);
    menu.AddItem("info", text, ITEMDRAW_DISABLED);
}

void DisplayConfiguredBenefits(int client)
{
    Menu menu = new Menu(BenefitsSelected);
    menu.SetTitle("Your configured Premium bonuses (%s)", g_ClassNames[RuleClass(client)]);
    if (!HasAnyFeature(client)) AddNumericBenefit(menu, "No feature access; Premium membership inactive");
    else
    {
        AddNumericBenefit(menu, "Premium membership: %s; individual grants are separate", HasPremium(client) ? "active" : "inactive");
        if (!GameplayAllowed(client)) AddNumericBenefit(menu, "Gameplay unavailable for this team/access; cosmetics may remain");
        if (GameplayFeatureAllowed(client, Feature_Ammo))
        {
            AddNumericBenefit(menu, "Reserve ammo perk: x%.2f (magazines stay native)", EffectiveAmmoMultiplier(client));
            AddNumericBenefit(menu, "Ammo held now: primary %d, secondary %d", GetEntProp(client, Prop_Send, "m_iAmmo", _, 1), GetEntProp(client, Prop_Send, "m_iAmmo", _, 2));
        }
        if (GameplayFeatureAllowed(client, Feature_Health)) AddNumericBenefit(menu, "Spawn health +%d", cvHealth.IntValue);
        if (GameplayFeatureAllowed(client, Feature_KillHealth)) AddNumericBenefit(menu, "Kill health reward +%d", cvKillHealth.IntValue);
        if (GameplayFeatureAllowed(client, Feature_AssistHealth)) AddNumericBenefit(menu, "Assist health reward +%d", cvAssistHealth.IntValue);
        if (GameplayFeatureAllowed(client, Feature_Speed) && cvSpeed.BoolValue) AddNumericBenefit(menu, "Speed x%.2f", EffectiveClassSpeed(client));
        if (GameplayFeatureAllowed(client, Feature_Gravity)) AddNumericBenefit(menu, "Gravity x%.2f", EffectiveGravity(client));
        if (BhopEnabled(client)) AddNumericBenefit(menu, "Held jump on; gain x%.2f; cap %.0f", BhopGain(client), BhopCap(client));
        if (TF2_GetPlayerClass(client) == TFClass_Engineer)
        {
            if (GameplayFeatureAllowed(client, Feature_Metal)) AddNumericBenefit(menu, "Metal: %d held; configured spawn reserve %d", GetEntProp(client, Prop_Send, "m_iAmmo", _, 3), cvMetal.IntValue);
            if (GameplayFeatureAllowed(client, Feature_Sentry)) AddNumericBenefit(menu, "Sentries: %d/%d; team extras %d/%d", CountOwnedBuildings(client), ExtraSentryAllowed(client) ? cvSentryLimit.IntValue : 1,
                CountTeamExtraSentries(GetClientTeam(client)), cvTeamSentries.IntValue);
            if (GameplayFeatureAllowed(client, Feature_Dispenser)) AddNumericBenefit(menu, "Dispensers: %d/%d", CountUtilities(client, 0), UtilityLimit(client, 0));
            if (GameplayFeatureAllowed(client, Feature_Teleporter)) AddNumericBenefit(menu, "Teleporter entrances: %d/%d; exits: %d/%d", CountUtilities(client, 1), UtilityLimit(client, 1), CountUtilities(client, 2), UtilityLimit(client, 2));
            if (GameplayAllowed(client)) AddNumericBenefit(menu, "All buildings: %d/6 slots; one slot per teleporter end", CountOwnedBuildings(client, false));
        }
        if (TF2_GetPlayerClass(client) == TFClass_Scout && GameplayFeatureAllowed(client, Feature_Balls)) AddNumericBenefit(menu, "Ball reserve: %d; native one-at-a-time recharge", cvBalls.IntValue);
        if (TF2_GetPlayerClass(client) == TFClass_Sniper && GameplayFeatureAllowed(client, Feature_Jarate)) AddNumericBenefit(menu, "Jarate reserve: %d; native one-at-a-time recharge", cvJarate.IntValue);
        if (TF2_GetPlayerClass(client) == TFClass_Spy)
        {
            if (GameplayFeatureAllowed(client, Feature_Cloak)) AddNumericBenefit(menu, "Cloak x%.2f", cvCloak.BoolValue ? cvCloakMultiplier.FloatValue : 1.0);
            if (GameplayFeatureAllowed(client, Feature_DeadRinger)) AddNumericBenefit(menu, "Dead Ringer x%.2f", cvCloak.BoolValue ? cvDeadRingerMultiplier.FloatValue : 1.0);
        }
        if (TF2_GetPlayerClass(client) == TFClass_Medic && GameplayFeatureAllowed(client, Feature_Uber)) AddNumericBenefit(menu, "Starting medigun charge: %.0f%%", cvUber.FloatValue);
        if (TF2_GetPlayerClass(client) == TFClass_Pyro)
        {
            if (GameplayFeatureAllowed(client, Feature_Flight)) AddNumericBenefit(menu, "Flight: %s; speed %.0f; climb %.0f", FlightEnabled(client) ? "on" : "off", FlightSpeed(client), FlightLift(client));
            if (GameplayFeatureAllowed(client, Feature_Airblast)) AddNumericBenefit(menu, "Airblast jump: %s; recoil %.0f; lift %.0f; cooldown %.2fs", AirblastEnabled(client) ? "on" : "off", cvAirblastForce.FloatValue, cvAirblastLift.FloatValue, cvAirblastCooldown.FloatValue);
        }
        if (GameplayFeatureAllowed(client, Feature_Teleport)) AddNumericBenefit(menu, "Teleport: %s; cooldown %.0f seconds", TeleportEnabled(client) ? "on" : "off", cvTeleportCooldown.FloatValue);
        if (HasFeature(client, Feature_Swap)) { char reason[192]; SwapAvailability(client, reason, sizeof(reason)); AddNumericBenefit(menu, "%s", reason); }
    }
    if (HasFeature(client, Feature_Glow) && cvGlow.BoolValue) AddBenefit(menu, client, "Benefit Glow");
    if (HasFeature(client, Feature_Color) && cvColor.IntValue > 0) AddBenefit(menu, client, "Benefit Color");
    if (HasFeature(client, Feature_Dark) && cvDark.BoolValue) AddBenefit(menu, client, "Benefit Dark");
    if (HasFeature(client, Feature_Fov) && cvFov.BoolValue) AddNumericBenefit(menu, "FOV: %d", g_Player[client].fov > 0 ? g_Player[client].fov : cvDefaultFov.IntValue);
    AddNumericBenefit(menu, "Map: %s; flight map gate %s; teleport map gate %s", g_RuleMap, g_MapFlight ? "open" : "closed", g_MapTeleport ? "open" : "closed");
    menu.ExitBackButton = true; menu.Display(client, 30);
}

public Action CommandBenefits(int client, int args)
{
    if (ValidClient(client)) DisplayConfiguredBenefits(client); else Tell(client, "%t", "In Game Only");
    return Plugin_Handled;
}

public Action PremiumCosmeticTransmit(int entity, int viewer)
{
    return ValidClient(viewer) && !g_Options[viewer].cosmetics ? Plugin_Handled : Plugin_Continue;
}
