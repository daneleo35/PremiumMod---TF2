// Apply flight after native movement and weapon ItemPostFrame. RunCmd only
// captures the current inputs; no pre-movement velocity write gets overwritten.
float g_FireballShot[MAXPLAYERS + 1];
int g_FireballWeapon[MAXPLAYERS + 1];

bool MovementBlocked(int client)
{
    return GetEntityMoveType(client) != MOVETYPE_WALK || (GetEntityFlags(client) & FL_FROZEN) != 0
        || GetEntProp(client, Prop_Send, "m_nWaterLevel") >= 2
        || TF2_IsPlayerInCondition(client, TFCond_FreezeInput)
        || TF2_IsPlayerInCondition(client, TFCond_Taunting)
        || TF2_IsPlayerInCondition(client, TFCond_Dazed)
        || TF2_IsPlayerInCondition(client, TFCond_Charging);
}

bool FlightStreamActive(int client, int weapon)
{
    if (!ValidWeapon(weapon) || MovementBlocked(client)) return false;
    char classname[64];
    GetEntityClassname(weapon, classname, sizeof(classname));
    if (!PremiumFlightWeapon(classname)) return false;
    int ammo = GetEntProp(weapon, Prop_Send, "m_iPrimaryAmmoType");
    if (ammo < 0 || ammo >= GetEntPropArraySize(client, Prop_Send, "m_iAmmo")
        || GetEntProp(client, Prop_Send, "m_iAmmo", _, ammo) <= 0) return false;
    if (GetEntPropFloat(client, Prop_Send, "m_flNextAttack") > GetGameTime()) return false;
    if (StrEqual(classname, "tf_weapon_flamethrower"))
        return HasEntProp(weapon, Prop_Send, "m_iWeaponState")
            && PremiumFlameFlightActive(GetEntProp(weapon, Prop_Send, "m_iWeaponState"),
                GetEntProp(client, Prop_Send, "m_iAmmo", _, ammo));
    // Dragon's Fury has no flame-state property. A real primary shot arms its
    // native refire interval, so holding an unfired/holstered weapon cannot fly.
    return g_FireballWeapon[client] == EntIndexToEntRef(weapon)
        && GetGameTime() - g_FireballShot[client] < 1.0;
}

public void ApplyFlameFlight(int client)
{
    if (!GameplayFeatureAllowed(client, Feature_Flight) || !FlightEnabled(client) || !IsPlayerAlive(client)
        || TF2_GetPlayerClass(client) != TFClass_Pyro || g_Player[client].flightTick != GetGameTickCount()
        || !(g_Player[client].flightButtons & IN_ATTACK) || (g_Player[client].flightButtons & IN_ATTACK2)
        || (GetEntityFlags(client) & FL_ONGROUND)) return;
    int active = GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon");
    if (!FlightStreamActive(client, active)) return;
    float velocity[3];
    GetEntPropVector(client, Prop_Data, "m_vecVelocity", velocity);
    float dt = GetTickInterval();
    float pitch = DegToRad(g_Player[client].flightAngles[0]), yaw = DegToRad(g_Player[client].flightAngles[1]);
    float gravityScale = GetEntityGravity(client);
    if (gravityScale == 0.0) gravityScale = 1.0;
    ConVar gravity = FindConVar("sv_gravity");
    float acceleration = FlightAcceleration(client);
    velocity[2] = PremiumFlightVertical(velocity[2], Sine(pitch), gravity.FloatValue * gravityScale,
        FlightLift(client), acceleration, dt);
    float forwardInput = g_Player[client].flightInput[0], side = g_Player[client].flightInput[1];
    float inputX = Cosine(yaw) * forwardInput + Sine(yaw) * side;
    float inputY = Sine(yaw) * forwardInput - Cosine(yaw) * side;
    float length = SquareRoot(inputX * inputX + inputY * inputY);
    // Movement input is usually capped to class speed; normalize to the chosen
    // flight speed, so a configured 400 does not silently mean Pyro's 300.
    float scale = length > 0.01 ? FlightSpeed(client) / length : 0.0;
    velocity[0] = PremiumApproach(velocity[0], inputX * scale, acceleration * dt);
    velocity[1] = PremiumApproach(velocity[1], inputY * scale, acceleration * dt);
    TeleportEntity(client, NULL_VECTOR, NULL_VECTOR, velocity);
}
