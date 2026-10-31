bool g_SettingsQueued;
bool g_BackendChanged;
bool g_AmmoAliasSync;

ConVar Setting(const char[] name, const char[] value, const char[] description, float minimum, float maximum)
{
    ConVar setting = CreateConVar(name, value, description, FCVAR_NONE, true, minimum, true, maximum);
    // ConVars survive plugin reloads; CreateConVar alone keeps rc4's old bounds.
    setting.SetBounds(ConVarBound_Lower, true, minimum);
    setting.SetBounds(ConVarBound_Upper, true, maximum);
    setting.AddChangeHook(ConfigurationChanged);
    return setting;
}

void CreateConfiguration()
{
    ConVar version = CreateConVar("premium_version", VERSION, "Premium Mod rewrite version", FCVAR_NOTIFY | FCVAR_DONTRECORD);
    version.SetString(VERSION); // An earlier plugin's cvar can survive a hot reload.
    cvEnable = Setting("premium_enable", "1", "Enable premium perks; disabling restores owned effects", 0.0, 1.0);
    cvGameplayTeams = Setting("premium_gameplay_teams", "3", "Gameplay perks: 0 neither, 1 RED, 2 BLU, 3 both; cosmetic feature grants and membership chat are independent", 0.0, 3.0);
    cvFlags = CreateConVar("premium_flags", "o", "Default premium flags; premium_access override takes precedence");
    cvFlags.AddChangeHook(ConfigurationChanged);
    cvDatabase = CreateConVar("premium_database", "premium", "databases.cfg entry used when premium_use_mysql is enabled");
    cvDatabase.AddChangeHook(ConfigurationChanged);
    cvMySQL = Setting("premium_use_mysql", "0", "0: storage-local SQLite; 1: named MySQL/SQLite config", 0.0, 1.0);
    cvTrial = Setting("premium_trial", "1", "Enable trials; 0 stops new and existing trial access without resetting saved deadlines", 0.0, 1.0);
    cvTrialSeconds = Setting("premium_trial_timelimit", "900", "Trial duration in whole wall-clock seconds, including offline time", 1.0, 86400.0);
    cvAds = Setting("premium_advertisement_enable", "1", "Advertise premium availability", 0.0, 1.0);
    cvAdInterval = Setting("premium_advertisement_interval", "300", "Seconds between advertisements", 30.0, 86400.0);
    cvWelcome = Setting("premium_welcome_message", "1", "Private premium welcome", 0.0, 1.0);
    cvJoin = Setting("premium_premiumjoin_announce", "0", "Public join announcement (off by default for admin privacy)", 0.0, 1.0);
    cvTag = CreateConVar("premium_tag", "{green}[{lightgreen}Premium{green}]{default}", "More Colors chat prefix, up to 63 characters");
    cvFeatures = Setting("premium_displaypremfeatures", "1", "Show feature menu on first successful trial activation", 0.0, 1.0);
    cvSpeed = Setting("premium_speed", "1", "Enable attribute-based class speed multipliers", 0.0, 1.0);
    char classes[][] = {"unknown", "scout", "sniper", "soldier", "demoman", "medic", "heavy", "pyro", "spy", "engineer"};
    for (int i = 1; i <= 9; i++)
    {
        char name[64];
        FormatEx(name, sizeof(name), "premium_speedmult_%s", classes[i]);
        cvClassSpeed[i] = Setting(name, "1.0", "Class speed multiplier; game handles slows and weapon state", 1.0, 2.0);
    }
    cvGravity = Setting("premium_gravity_mult", "0.75", "Player gravity multiplier; 1 disables", 0.1, 2.0);
    cvHealth = Setting("premium_health_boost", "15", "Extra max health and health on spawn", 0.0, 500.0);
    bool ammoExisted = FindConVar("premium_ammo_multiplier") != null;
    cvAmmo = Setting("premium_ammo_multiplier", "1.25", "Primary/secondary reserve ammo multiplier, including Pyro fuel; normal magazines; extra ammo on spawn", 1.0, 10.0);
    cvLegacyClip = Setting("premium_clip_multiplier", "1.25", "Legacy alias of premium_ammo_multiplier; increases reserve ammo, not magazines", 1.0, 10.0);
    PremiumSyncAmmoAlias(ammoExisted ? cvAmmo : cvLegacyClip, ammoExisted ? cvLegacyClip : cvAmmo);
    cvKillHealth = Setting("premium_healthbonus_kill", "25", "Health reward for a real kill", 0.0, 500.0);
    cvAssistHealth = Setting("premium_healthbonus_assist", "25", "Health reward for a real assist", 0.0, 500.0);
    cvOverheal = Setting("premium_reward_health_cap", "1.0", "Kill/assist reward cap as multiple of max health", 1.0, 2.0);
    cvColor = Setting("premium_color", "1", "0 disables coloring; 1 allows custom RGB; 2-6 set a default color", 0.0, 6.0);
    cvDark = Setting("premium_dark", "1", "Allow black render tint (does not make players invisible)", 0.0, 1.0);
    cvGlow = Setting("premium_glow", "1", "Allow the toggleable particle aura via glow; hidden while cloaked/disguised", 0.0, 1.0);
    cvGlowStyle = Setting("premium_glow_style", "0", "0 classic four effects; 1 team two effects; 2 subtle team one effect", 0.0, 2.0);
    cvFov = Setting("premium_fov_enable", "1", "Allow and apply premium FOV", 0.0, 1.0);
    cvDefaultFov = Setting("premium_fov_default", "100", "Default premium FOV", 75.0, 120.0);
    cvBalls = Setting("premium_stunballs", "1", "Spawn reserve balls for supported Sandman/Wrap Assassin", 0.0, 20.0);
    cvJarate = Setting("premium_jarates", "1", "Spawn reserve Jarate for supported Jarate weapons", 0.0, 20.0);
    cvUber = Setting("premium_ubercharge", "1", "Minimum spawn medigun charge percentage", 0.0, 100.0);
    cvRespawn = Setting("premium_instant_respawn", "1", "Respawn after one second except Arena/MvM, sudden death or round end", 0.0, 1.0);
    cvSwap = Setting("premium_swapteams", "1", "Allow premium team switching", 0.0, 1.0);
    cvSwapCooldown = Setting("premium_swapteam_cooldown", "60", "Seconds between successful premium team swaps", 0.0, 600.0);
    cvSwapDifference = Setting("premium_swapteam_max_difference", "1", "Maximum team-size difference created by a swap; 0 allows any difference", 0.0, 32.0);
    cvCloak = Setting("premium_cloak", "1", "Enable finite cloak-duration multipliers; no forced full meter or extra recharge", 0.0, 1.0);
    cvCloakMultiplier = Setting("premium_cloak_multiplier", "2", "Cloak duration multiplier for ordinary and motion watches; 1 is native duration", 1.0, 20.0);
    cvDeadRingerMultiplier = Setting("premium_deadringer_multiplier", "2", "Dead Ringer cloak-duration multiplier; activation cost, recharge and resistance stay native", 1.0, 20.0);
    cvMetal = Setting("premium_metal", "200", "Engineer metal capacity and initial spawn reserve; no command or automatic refill; 0 disables", 0.0, 2000.0);
    cvSentryLimit = Setting("premium_sentry_limit", "3", "Maximum owned sentries including ordinary PDA sentry; 1 disables extra sentries", 1.0, 3.0);
    cvDispenserLimit = Setting("premium_dispenser_limit", "3", "Maximum dispensers per eligible Engineer (1-3); all building types share six total slots", 1.0, 3.0);
    cvEntranceLimit = Setting("premium_teleporter_entrance_limit", "3", "Maximum teleporter entrances (1-3); entrances and exits each consume one of six total building slots", 1.0, 3.0);
    cvExitLimit = Setting("premium_teleporter_exit_limit", "3", "Maximum teleporter exits (1-3); three complete pairs require all six building slots", 1.0, 3.0);
    cvTeamSentries = Setting("premium_team_extra_sentries", "4", "Extra sentries per team; primary guns do not count; 0 disables extras", 0.0, 32.0);
    cvEngineerHud = Setting("premium_engineer_hud", "1", "Owner-only native turret tooltip while aiming; no large HUD overlay", 0.0, 1.0);
    cvSentryLabels = Setting("premium_sentry_labels", "1", "Include primary/extra role in the native turret tooltip", 0.0, 1.0);
    cvReminders = Setting("premium_trial_reminders", "1", "Optional trial reminders at 5 minutes, 1 minute and 10 seconds", 0.0, 1.0);
    cvTagStyles = Setting("premium_tag_styles", "7", "Allowed personal tag styles bitmask: 1 team colour, 2 gold, 4 plain; default tag always allowed", 0.0, 7.0);
    cvPersonalGlow = Setting("premium_personal_glow", "1", "Allow personal glow choice: default, classic, team or subtle", 0.0, 1.0);
    cvCelebration = Setting("premium_round_celebration", "1", "Short winning-team Premium particle effect after round win; no sound", 0.0, 1.0);
    cvCelebrationSeconds = Setting("premium_round_celebration_seconds", "2", "Round-win effect duration", 0.5, 5.0);
    cvCelebrationBudget = Setting("premium_round_celebration_budget", "8", "Maximum simultaneous round-win emitters", 0.0, 16.0);
    cvChatTags = Setting("premium_chat_tags", "1", "Prefix ordinary player chat with membership or trial tag", 0.0, 1.0);
    cvMemberTag = CreateConVar("premium_chat_tag", "{green}[Premium]{default}", "Paid/flag member player-chat tag, More Colors markup");
    cvTrialTag = CreateConVar("premium_trial_chat_tag", "{lightgreen}[Trial]{default}", "Trial player-chat tag, More Colors markup");
    cvFakeKill = Setting("premium_deadringer", "1", "Notify a premium attacker about an actual feigned death", 0.0, 1.0);
    cvArrows = Setting("premium_firearrows", "1", "Ignite supported bow arrows", 0.0, 1.0);
    cvBhop = Setting("premium_bhop_enabled", "1", "Allow held-jump bunnyhop/trimp", 0.0, 1.0);
    cvBhopGain = Setting("premium_bhop_increment", "1.15", "Bunnyhop horizontal velocity multiplier", 1.0, 1.5);
    cvBhopCap = Setting("premium_bhop_speed_cap", "640", "Maximum boosted horizontal velocity", 300.0, 1000.0);
    cvDoubleJump = Setting("premium_doublejump", "0", "Allow one follow-up jump after a trimp; excludes normal class double-jumps", 0.0, 1.0);
    cvFlight = Setting("premium_pyrofly_enabled", "1", "Airborne flame flight for all flamethrowers including Dragon's Fury; jump and fire, WASD to steer", 0.0, 1.0);
    cvFlightSpeed = Setting("premium_pyrofly_speed", "400", "Maximum horizontal WASD flight speed; aim down for lift", 100.0, 800.0);
    cvFlightLift = Setting("premium_pyrofly_lift", "200", "Maximum flight climb/descent speed controlled by aim pitch", 50.0, 800.0);
    cvFlightAcceleration = Setting("premium_pyrofly_acceleration", "600", "Flight steering acceleration per second", 100.0, 3000.0);
    cvAirblast = Setting("premium_airblast_enabled", "1", "Pyro recoil jump only while looking down on a real fuel-consuming airblast; map flight bans also block this", 0.0, 1.0);
    cvAirblastForce = Setting("premium_airblast_force", "400", "Airblast boost recoil away from aim direction", 0.0, 1500.0);
    cvAirblastLift = Setting("premium_airblast_lift", "350", "Minimum upward impulse per airblast boost, added to current velocity", 0.0, 1000.0);
    cvAirblastCap = Setting("premium_airblast_speed_cap", "900", "Boost horizontal/upward speed cap; preserves pre-existing faster momentum", 100.0, 2000.0);
    cvAirblastCooldown = Setting("premium_airblast_cooldown", "0.75", "Minimum seconds between successful boosts; native airblast cooldown still applies", 0.1, 10.0);
    cvTeleport = Setting("premium_teleport_enable", "1", "Allow session/life/team-bound save_loc and load_loc", 0.0, 1.0);
    cvTeleportCooldown = Setting("premium_teleport_cooldown", "5", "Minimum seconds between location loads", 1.0, 60.0);
    cvPoints = Setting("premium_points_enable", "0", "Enable optional points economy; 0 also suspends purchased access without changing saved expiry", 0.0, 1.0);
    cvPointsEarn = Setting("premium_points_earning", "1", "Award play-time points when the economy is enabled", 0.0, 1.0);
    cvPointsShop = Setting("premium_points_shop", "1", "Allow purchases; disable separately to keep existing rentals active", 0.0, 1.0);
    cvPointsRentals = Setting("premium_points_rentals", "1", "Honor purchased individual features; expiry still counts while disabled", 0.0, 1.0);
    cvPointsTrials = Setting("premium_points_trials", "0", "Allow/honor points-bought Premium trials independently of free premium_trial", 0.0, 1.0);
    cvPointsTrialLimit = Setting("premium_points_trial_limit", "1", "Lifetime purchased trials per Steam ID; 0 unlimited; never resets free-trial eligibility", 0.0, 10000.0);
    cvPointsMinutes = Setting("premium_points_interval_minutes", "10", "Eligible play minutes per points award", 1.0, 1440.0);
    cvPointsReward = Setting("premium_points_per_interval", "5", "Points awarded per completed play interval", 1.0, 1000000.0);
    cvPointsCap = Setting("premium_points_cap", "1000000", "Maximum newly earned/credited balance; lowering does not confiscate existing points", 1.0, 1000000000.0);
    cvPointsMinPlayers = Setting("premium_points_min_players", "1", "Minimum connected humans on counted teams before earning", 1.0, 100.0);
    cvPointsSpectators = Setting("premium_points_count_spectators", "0", "Count spectator/unassigned connected humans toward earning/population", 0.0, 1.0);
    cvPointsDead = Setting("premium_points_count_dead", "1", "Count time dead on a playing team toward earning", 0.0, 1.0);
    cvPointsAfk = Setting("premium_points_afk_seconds", "0", "Stop earning after this many seconds without movement/fire/jump input; 0 disables this filter", 0.0, 3600.0);
    cvPointsEarnNotice = Setting("premium_points_earn_notice", "1", "Private chat notice after committed points earnings; quiet mode suppresses it", 0.0, 1.0);
    cvPointsBuyNotice = Setting("premium_points_purchase_notice", "1", "Private chat receipt after a committed purchase", 0.0, 1.0);
    cvPointsExpiryNotice = Setting("premium_points_expiry_notice", "1", "Private rental/trial expiry notices; quiet mode suppresses them", 0.0, 1.0);
    cvPointsRenew = Setting("premium_points_renewals", "1", "Allow extending active individual rentals; active trials cannot be repurchased", 0.0, 1.0);
    cvPointsMaxDays = Setting("premium_points_max_rental_days", "365", "Maximum remaining duration of an individual rental including extensions", 1.0, 365.0);

}

public void ConfigurationChanged(ConVar setting, const char[] oldValue, const char[] newValue)
{
    g_UiSettingsRevision++;
    if (setting == cvAmmo && cvLegacyClip != null) PremiumSyncAmmoAlias(cvAmmo, cvLegacyClip);
    else if (setting == cvLegacyClip && cvAmmo != null) PremiumSyncAmmoAlias(cvLegacyClip, cvAmmo);
    if (setting == cvMySQL || setting == cvDatabase) g_BackendChanged = true;
    if (!g_SettingsQueued)
    {
        g_SettingsQueued = true;
        RequestFrame(FrameConfiguration);
    }
}

void PremiumSyncAmmoAlias(ConVar source, ConVar destination)
{
    if (g_AmmoAliasSync) return;
    g_AmmoAliasSync = true;
    if (destination.FloatValue != source.FloatValue) destination.SetFloat(source.FloatValue);
    g_AmmoAliasSync = false;
}

public void FrameConfiguration(any data)
{
    g_SettingsQueued = false;
    if (g_Stopping) return;
    if (g_BackendChanged)
    {
        g_BackendChanged = false;
        RestartDatabase();
    }
    g_NextAdvert = GetEngineTime() + cvAdInterval.FloatValue;
    for (int client = 1; client <= MaxClients; client++)
    {
        if (!ValidClient(client)) continue;
        if (!cvRespawn.BoolValue || !cvEnable.BoolValue) CancelRespawn(client);
        if (!cvBhop.BoolValue) ResetMovement(client);
        RefreshClient(client);
        QueueEffects(client);
        RefreshGlow(client);
        MaintainExtraBuildings(client);
    }
    InitializeEconomyStorage();
    ProcessEconomyOutbox();
    EnforceTeamSentryCaps();
}
