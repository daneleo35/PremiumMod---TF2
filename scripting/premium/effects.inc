void RestoreGravity(int client)
{
    if (!g_Player[client].gravityOwned) return;
    if (FloatAbs(GetEntityGravity(client) - g_Player[client].appliedGravity) < 0.0001)
        SetEntityGravity(client, g_Player[client].oldGravity);
    g_Player[client].gravityOwned = false;
}

void RestoreFov(int client)
{
    if (!g_Player[client].fovOwned) return;
    if (GetEntProp(client, Prop_Send, "m_iDefaultFOV") == g_Player[client].appliedFov)
        SetEntProp(client, Prop_Send, "m_iDefaultFOV", g_Player[client].oldFov);
    g_Player[client].fovOwned = false;
}

void RestoreColor(int client)
{
    if (!g_Player[client].colorOwned) return;
    int color[4];
    GetEntityRenderColor(client, color[0], color[1], color[2], color[3]);
    bool ours = true;
    for (int i; i < 4; i++) if (color[i] != g_Player[client].appliedColor[i]) ours = false;
    if (ours)
        SetEntityRenderColor(client, g_Player[client].oldColor[0], g_Player[client].oldColor[1],
            g_Player[client].oldColor[2], g_Player[client].oldColor[3]);
    g_Player[client].colorOwned = false;
}

void RestoreGameplayEffects(int client)
{
    RestoreAttribute(g_Speed[client], "move speed bonus");
    RestoreAmmoCapacity(client);
    RestoreAttribute(g_MetalCapacity[client], "maxammo metal increased");
    RestoreAttribute(g_ThrowableCapacity[client], "maxammo grenades1 increased");
    RestoreGravity(client);
    if (IsPlayerAlive(client) && g_Player[client].bonusLifeHealth > 0 && !GameplayFeatureAllowed(client, Feature_Health))
    {
        int maximum = PlayerMaximum(client);
        if (GetClientHealth(client) > maximum) SetEntityHealth(client, maximum);
    }
    // Reconciliation of an unrelated grant must not forget the health bonus
    // we may still need to remove when health access is later revoked.
    if (!IsPlayerAlive(client) || !GameplayFeatureAllowed(client, Feature_Health)) g_Player[client].bonusLifeHealth = 0;
    g_CloakSample[client] = false;
    ResetMovement(client);
}

void RestoreEffects(int client)
{
    ResetPresentation(client);
    StopGlow(client);
    RestoreGameplayEffects(client);
    RestoreFov(client);
    RestoreColor(client);
}

void ApplyRenderState(int client)
{
    int red = 255, green = 255, blue = 255;
    bool enabled = false;
    if (g_Player[client].dark && cvDark.BoolValue && HasFeature(client, Feature_Dark))
    {
        red = 0; green = 0; blue = 0;
        enabled = true;
    }
    else if (cvColor.IntValue > 0 && HasFeature(client, Feature_Color))
    {
        if (g_Player[client].customColor)
        {
            red = g_Player[client].rgb[0];
            green = g_Player[client].rgb[1];
            blue = g_Player[client].rgb[2];
            enabled = true;
        }
        else
        {
            switch (cvColor.IntValue)
            {
                case 2: { red = 255; green = 100; blue = 100; enabled = true; }
                case 3: { red = 100; green = 100; blue = 255; enabled = true; }
                case 4: { red = 255; green = 255; blue = 100; enabled = true; }
                case 5: { red = 100; green = 255; blue = 255; enabled = true; }
                case 6: { red = 255; green = 100; blue = 255; enabled = true; }
            }
        }
    }
    if (!enabled)
    {
        RestoreColor(client);
        return;
    }
    if (!g_Player[client].colorOwned)
    {
        GetEntityRenderColor(client, g_Player[client].oldColor[0], g_Player[client].oldColor[1],
            g_Player[client].oldColor[2], g_Player[client].oldColor[3]);
        g_Player[client].colorOwned = true;
    }
    g_Player[client].appliedColor[0] = red;
    g_Player[client].appliedColor[1] = green;
    g_Player[client].appliedColor[2] = blue;
    g_Player[client].appliedColor[3] = 255;
    SetEntityRenderColor(client, red, green, blue, 255);
}

void ApplyEffects(int client, bool spawn)
{
    if (!ValidClient(client) || !IsPlayerAlive(client)) return;
    if (!HasAnyFeature(client)) { RestoreEffects(client); return; }
    int class = view_as<int>(TF2_GetPlayerClass(client));
    bool gameplay = GameplayAllowed(client);
    int healthBonus = GameplayFeatureAllowed(client, Feature_Health) ? cvHealth.IntValue : 0;
    if (g_Player[client].bonusLifeHealth > healthBonus)
    {
        int maximum = PlayerMaximum(client);
        if (GetClientHealth(client) > maximum) SetEntityHealth(client, maximum);
        g_Player[client].bonusLifeHealth = healthBonus;
    }
    float speed = (GameplayFeatureAllowed(client, Feature_Speed) && cvSpeed.BoolValue && class >= 1 && class <= 9) ? EffectiveClassSpeed(client) : 1.0;
    ApplyAttribute(client, g_Speed[client], "move speed bonus", speed);
    MaintainWeaponPerks(client);

    if (!GameplayFeatureAllowed(client, Feature_Gravity) || EffectiveGravity(client) == 1.0) RestoreGravity(client);
    else
    {
        if (!g_Player[client].gravityOwned)
        {
            g_Player[client].oldGravity = GetEntityGravity(client);
            g_Player[client].gravityOwned = true;
        }
        // Zero is the engine's default-gravity sentinel, equivalent to 1.0 here.
        float base = g_Player[client].oldGravity == 0.0 ? 1.0 : g_Player[client].oldGravity;
        g_Player[client].appliedGravity = base * EffectiveGravity(client);
        SetEntityGravity(client, g_Player[client].appliedGravity);
    }
    ApplyRenderState(client);
    RefreshGlow(client);
    if (HasFeature(client, Feature_Fov) && cvFov.BoolValue)
    {
        if (!g_Player[client].fovOwned)
        {
            g_Player[client].oldFov = GetEntProp(client, Prop_Send, "m_iDefaultFOV");
            g_Player[client].fovOwned = true;
        }
        int fov = g_Player[client].fov > 0 ? g_Player[client].fov : cvDefaultFov.IntValue;
        g_Player[client].appliedFov = fov;
        SetEntProp(client, Prop_Send, "m_iDefaultFOV", fov);
    }
    else RestoreFov(client);

    if (!gameplay)
    {
        RestoreGameplayEffects(client);
        CancelRespawn(client);
        g_Player[client].saved = false;
        return;
    }
    if (!spawn) return; // Inventory reconciliation never stacks one-shot rewards.
    GiveSpawnAmmo(client);
    int bonus = healthBonus;
    if (bonus > 0 && g_Player[client].bonusLifeHealth == 0)
    {
        int health = GetClientHealth(client) + bonus;
        int maximum = PlayerMaximum(client);
        SetEntityHealth(client, health < maximum ? health : maximum);
        g_Player[client].bonusLifeHealth = bonus;
    }
    int secondary = GetPlayerWeaponSlot(client, 1);
    if (GameplayFeatureAllowed(client, Feature_Uber) && class == view_as<int>(TFClass_Medic) && ValidWeapon(secondary)
        && HasEntProp(secondary, Prop_Send, "m_flChargeLevel"))
    {
        float charge = cvUber.FloatValue * 0.01;
        if (GetEntPropFloat(secondary, Prop_Send, "m_flChargeLevel") < charge)
            SetEntPropFloat(secondary, Prop_Send, "m_flChargeLevel", charge);
    }
    if (GameplayFeatureAllowed(client, Feature_Metal) && class == view_as<int>(TFClass_Engineer) && cvMetal.IntValue > 0)
    {
        int metal = GetEntProp(client, Prop_Send, "m_iAmmo", _, 3);
        if (metal < cvMetal.IntValue) SetEntProp(client, Prop_Send, "m_iAmmo", cvMetal.IntValue, _, 3);
    }
    SetThrowableAmmo(client, secondary, GameplayFeatureAllowed(client, Feature_Jarate) ? cvJarate.IntValue : 0, true);
    SetThrowableAmmo(client, GetPlayerWeaponSlot(client, 2), GameplayFeatureAllowed(client, Feature_Balls) ? cvBalls.IntValue : 0, false);
}

void SetThrowableAmmo(int client, int weapon, int amount, bool jarate)
{
    if (amount <= 0 || !ValidWeapon(weapon)) return;
    int definition = GetEntProp(weapon, Prop_Send, "m_iItemDefinitionIndex");
    if (jarate && definition != 58 && definition != 1083 && definition != 1105) return;
    if (!jarate && definition != 44 && definition != 648) return;
    int primary = jarate ? GetEntProp(weapon, Prop_Send, "m_iPrimaryAmmoType") : -1;
    int index = PremiumThrowableAmmoSlot(jarate, primary, GetEntPropArraySize(client, Prop_Send, "m_iAmmo"));
    if (index < 0) return;
    if (GetEntProp(client, Prop_Send, "m_iAmmo", _, index) < amount)
        SetEntProp(client, Prop_Send, "m_iAmmo", amount, _, index);
}

public void EventSpawn(Event event, const char[] name, bool dontBroadcast)
{
    int client = GetClientOfUserId(event.GetInt("userid"));
    if (!ValidClient(client)) return;
    CancelRespawn(client);
    RestoreEffects(client);
    g_Player[client].bonusLifeHealth = 0;
    g_Player[client].saved = false;
    ResetMovement(client);
    QueueEffects(client, true);
}

public void EventInventory(Event event, const char[] name, bool dontBroadcast)
{
    int client = GetClientOfUserId(event.GetInt("userid"));
    if (ValidClient(client)) QueueEffects(client);
}

public void EventClass(Event event, const char[] name, bool dontBroadcast)
{
    int client = GetClientOfUserId(event.GetInt("userid"));
    if (!ValidClient(client)) return;
    StopGlow(client);
    StopCelebration(client);
    RemoveExtraBuildings(client);
    g_BaseMaxHealth[client] = 0;
    g_Player[client].saved = false;
    ResetMovement(client);
    QueueEffects(client);
}

public void EventTeam(Event event, const char[] name, bool dontBroadcast)
{
    int client = GetClientOfUserId(event.GetInt("userid"));
    if (!ValidClient(client)) return;
    CancelRespawn(client);
    RemoveExtraBuildings(client);
    RestoreEffects(client);
    g_Player[client].saved = false;
    ResetMovement(client);
    QueueEffects(client);
}

public void EventRoundStart(Event event, const char[] name, bool dontBroadcast)
{
    StopCelebrations();
    ApplyPendingProfile();
    g_RoundActive = true;
    for (int client = 1; client <= MaxClients; client++) g_Player[client].saved = false;
}

public void EventRoundEnd(Event event, const char[] name, bool dontBroadcast)
{
    if (StrEqual(name, "teamplay_round_win")) BeginCelebration(event.GetInt("team"));
    g_RoundActive = false;
    for (int client = 1; client <= MaxClients; client++)
    {
        StopGlow(client);
        CancelRespawn(client);
        RestoreBuildingPda(client);
        g_Player[client].saved = false;
    }
}

bool RespawnModeAllowed(int client, int deathFlags = 0)
{
    bool blockedMode = GameRules_GetProp("m_nGameType") == 4 || GameRules_GetProp("m_bPlayingMannVsMachine") != 0;
    RoundState state = GameRules_GetRoundState();
    bool playing = PremiumRespawnPhaseAllowed(state, g_RoundActive);
    return cvRespawn.BoolValue && GameplayFeatureAllowed(client, Feature_Respawn)
        && PremiumCanRespawn(deathFlags, GetClientTeam(client), IsPlayerAlive(client), blockedMode, playing);
}

void RewardHealth(int client, int amount, const char[] phrase, PremiumFeature feature)
{
    if (!ValidClient(client) || !IsPlayerAlive(client) || !GameplayFeatureAllowed(client, feature) || amount <= 0) return;
    int maximum = RoundToFloor(float(PlayerMaximum(client)) * cvOverheal.FloatValue);
    int health = GetClientHealth(client);
    int reward = maximum - health;
    if (reward > amount) reward = amount;
    if (reward <= 0) return;
    SetEntityHealth(client, health + reward);
    if (!g_Options[client].quiet) Tell(client, "%t", phrase, reward);
}

public void EventDeath(Event event, const char[] name, bool dontBroadcast)
{
    int victim = GetClientOfUserId(event.GetInt("userid"));
    if (!ValidClient(victim)) return;
    int attacker = GetClientOfUserId(event.GetInt("attacker"));
    int flags = event.GetInt("death_flags");
    if (flags & TF_DEATHFLAG_DEADRINGER)
    {
        StopGlow(victim);
        if (ValidClient(attacker) && attacker != victim && HasFeature(attacker, Feature_FakeKill) && cvFakeKill.BoolValue && !g_Options[attacker].quiet)
            Tell(attacker, "%t", "Fake Kill");
        return; // Feign events are not deaths, rewards, resets, or respawn requests.
    }
    CancelRespawn(victim);
    StopCelebration(victim);
    RestoreBuildingPda(victim);
    RestoreEffects(victim);
    g_Player[victim].saved = false;
    g_Player[victim].spawnPending = false;
    ResetMovement(victim);
    if (ValidClient(attacker) && attacker != victim && GetClientTeam(attacker) != GetClientTeam(victim))
    {
        RewardHealth(attacker, cvKillHealth.IntValue, "Kill Reward", Feature_KillHealth);
        int assister = GetClientOfUserId(event.GetInt("assister"));
        if (ValidClient(assister) && assister != attacker && assister != victim
            && GetClientTeam(assister) != GetClientTeam(victim))
            RewardHealth(assister, cvAssistHealth.IntValue, "Assist Reward", Feature_AssistHealth);
    }
    if (RespawnModeAllowed(victim, flags))
        g_Player[victim].respawn = CreateTimer(1.0, RespawnPlayer, GetClientSerial(victim), TIMER_FLAG_NO_MAPCHANGE);
}

public Action RespawnPlayer(Handle timer, any serial)
{
    int client = GetClientFromSerial(serial);
    if (!ValidClient(client) || g_Player[client].respawn != timer) return Plugin_Stop;
    g_Player[client].respawn = null;
    if (RespawnModeAllowed(client)) TF2_RespawnPlayer(client);
    return Plugin_Stop;
}

public Action TF2_CalcIsAttackCritical(int client, int weapon, char[] classname, bool &result)
{
    ProtectUtilityWrench(client, classname);
    if (!GameplayAllowed(client) || !ValidWeapon(weapon)) return Plugin_Continue;
    if (StrEqual(classname, "tf_weapon_rocketlauncher_fireball"))
    {
        g_FireballShot[client] = GetGameTime();
        g_FireballWeapon[client] = EntIndexToEntRef(weapon);
    }
    if (GameplayFeatureAllowed(client, Feature_FireArrows) && cvArrows.BoolValue && StrEqual(classname, "tf_weapon_compound_bow") && HasEntProp(weapon, Prop_Send, "m_bArrowAlight"))
        SetEntProp(weapon, Prop_Send, "m_bArrowAlight", 1);
    return Plugin_Continue;
}

public Action OnPlayerRunCmd(int client, int &buttons, int &impulse, float movement[3], float angles[3], int &weapon)
{
    EconomyInput(client, buttons, movement);
    if (!ValidClient(client) || !GameplayAllowed(client) || !IsPlayerAlive(client))
        return Plugin_Continue;
    g_Player[client].flightTick = GetGameTickCount();
    g_Player[client].flightButtons = buttons;
    for (int axis; axis < 2; axis++)
    {
        g_Player[client].flightInput[axis] = movement[axis];
        g_Player[client].flightAngles[axis] = angles[axis];
    }
    int originalButtons = buttons;
    if ((buttons & IN_ATTACK) && BuildingPlacementBlocked(client)) buttons &= ~IN_ATTACK;
    bool blocked = MovementBlocked(client);
    if (blocked)
    {
        ResetMovement(client);
        return Plugin_Continue;
    }
    bool grounded = (GetEntityFlags(client) & FL_ONGROUND) != 0;
    float velocity[3];
    GetEntPropVector(client, Prop_Data, "m_vecVelocity", velocity);
    float horizontal = SquareRoot(velocity[0] * velocity[0] + velocity[1] * velocity[1]);
    bool changed;
    if (BhopEnabled(client))
    {
        // TF2 rejects held jump via m_Local.m_nOldButtons in CheckJumpButton.
        // SetupMove copies this field into the movement command. Rearming only
        // grounded jumps preserves native airborne jump actions and jump height.
        if (grounded && (buttons & IN_JUMP) && HasEntProp(client, Prop_Data, "m_nOldButtons"))
        {
            int previous = GetEntProp(client, Prop_Data, "m_nOldButtons");
            int rearmed = PremiumGroundJumpLatch(previous, buttons, grounded);
            if (rearmed != previous) SetEntProp(client, Prop_Data, "m_nOldButtons", rearmed);
        }
        if (grounded && (buttons & IN_JUMP) && !g_Player[client].wasGrounded && horizontal > 0.01)
        {
            float target = PremiumClamp(horizontal * BhopGain(client), 0.0, BhopCap(client));
            velocity[0] *= target / horizontal;
            velocity[1] *= target / horizontal;
            if (target >= 640.0 && (buttons & (IN_FORWARD | IN_BACK)))
            {
                velocity[0] *= Cosine(DegToRad(70.0));
                velocity[1] *= Cosine(DegToRad(70.0));
                velocity[2] = target * Sine(DegToRad(70.0));
                buttons &= ~IN_JUMP;
                g_Player[client].trimpJump = true;
            }
            changed = true;
        }
        else if (!grounded && g_Player[client].trimpJump && DoubleJumpEnabled(client)
            && (buttons & IN_JUMP) && !(g_Player[client].previousButtons & IN_JUMP))
        {
            if (horizontal > 0.01)
            {
                float minimum = BhopCap(client) < 400.0 ? BhopCap(client) : 400.0;
                float target = PremiumClamp(horizontal * 1.2, minimum, BhopCap(client));
                velocity[0] *= target / horizontal;
                velocity[1] *= target / horizontal;
            }
            velocity[2] = 267.0;
            g_Player[client].trimpJump = false;
            buttons &= ~IN_JUMP;
            changed = true;
        }
        if (grounded && !(originalButtons & IN_JUMP)) g_Player[client].trimpJump = false;
    }
    if (changed) TeleportEntity(client, NULL_VECTOR, NULL_VECTOR, velocity);
    g_Player[client].previousButtons = originalButtons;
    g_Player[client].wasGrounded = grounded;
    return buttons != originalButtons ? Plugin_Changed : Plugin_Continue;
}

void MaintainWeaponPerks(int client)
{
    if (!IsPlayerAlive(client)) return;
    if (!GameplayAllowed(client))
    {
        RestoreGameplayEffects(client);
        return;
    }
    MaintainAmmoCapacity(client);
    bool engineer = TF2_GetPlayerClass(client) == TFClass_Engineer;
    float metalMultiplier = GameplayFeatureAllowed(client, Feature_Metal) && engineer && cvMetal.IntValue > 200 ? float(cvMetal.IntValue) / 200.0 : 1.0;
    ApplyAttribute(client, g_MetalCapacity[client], "maxammo metal increased", metalMultiplier);
    MaintainThrowableCapacity(client);
}

void MaintainThrowableCapacity(int client)
{
    bool jarate = TF2_GetPlayerClass(client) == TFClass_Sniper;
    bool scout = TF2_GetPlayerClass(client) == TFClass_Scout;
    int target = jarate && GameplayFeatureAllowed(client, Feature_Jarate) ? cvJarate.IntValue
        : (scout && GameplayFeatureAllowed(client, Feature_Balls) ? cvBalls.IntValue : 0);
    int weapon = target > 0 ? GetPlayerWeaponSlot(client, jarate ? 1 : 2) : -1;
    if (!ValidWeapon(weapon))
    {
        RestoreAttribute(g_ThrowableCapacity[client], "maxammo grenades1 increased");
        return;
    }
    int definition = GetEntProp(weapon, Prop_Send, "m_iItemDefinitionIndex");
    if ((jarate && definition != 58 && definition != 1083 && definition != 1105)
        || (!jarate && definition != 44 && definition != 648))
    {
        RestoreAttribute(g_ThrowableCapacity[client], "maxammo grenades1 increased");
        return;
    }
    // Native EffectBarRegenFinished gives ONE ammo and restarts the recharge
    // timer while below GetMaxAmmo. Raising the actual capacity preserves that
    // loop instead of refilling batches or adding ammo in our heartbeat.
    ApplyAttribute(client, g_ThrowableCapacity[client], "maxammo grenades1 increased", float(target));
}
