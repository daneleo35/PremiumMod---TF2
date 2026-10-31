#include "airblast_policy.inc"
bool g_AirblastSample[MAXPLAYERS + 1];
int g_AirblastWeapon[MAXPLAYERS + 1], g_AirblastAmmoSlot[MAXPLAYERS + 1], g_AirblastAmmo[MAXPLAYERS + 1];
float g_AirblastPrevious[MAXPLAYERS + 1], g_NextAirblast[MAXPLAYERS + 1];

bool AirblastClientAllowed(int client)
{
    return GameplayFeatureAllowed(client, Feature_Airblast) && AirblastEnabled(client) && IsPlayerAlive(client)
        && TF2_GetPlayerClass(client) == TFClass_Pyro && !MovementBlocked(client);
}

public void CaptureAirblast(int client)
{
    g_AirblastSample[client] = false;
    if (!AirblastClientAllowed(client) || !(GetClientButtons(client) & IN_ATTACK2)) return;
    float angles[3]; GetClientEyeAngles(client, angles);
    if (!PremiumAirblastLookingDown(angles[0])) return;
    int weapon = GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon");
    if (!ValidWeapon(weapon) || !HasEntProp(weapon, Prop_Send, "m_flNextSecondaryAttack")) return;
    char classname[64]; GetEntityClassname(weapon, classname, sizeof(classname));
    if (!PremiumFlightWeapon(classname)) return;
    int ammo = GetEntProp(weapon, Prop_Send, "m_iPrimaryAmmoType");
    if (ammo < 0 || ammo >= GetEntPropArraySize(client, Prop_Send, "m_iAmmo")) return;
    g_AirblastWeapon[client] = EntIndexToEntRef(weapon);
    g_AirblastAmmoSlot[client] = ammo;
    g_AirblastAmmo[client] = GetEntProp(client, Prop_Send, "m_iAmmo", _, ammo);
    g_AirblastPrevious[client] = GetEntPropFloat(weapon, Prop_Send, "m_flNextSecondaryAttack");
    g_AirblastSample[client] = true;
}

public void ApplyAirblastBoost(int client)
{
    bool sampled = g_AirblastSample[client];
    g_AirblastSample[client] = false; // Consume before any engine operation; at most one boost per sample.
    if (!sampled || !AirblastClientAllowed(client) || GetGameTime() < g_NextAirblast[client]) return;
    int weapon = GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon");
    if (!ValidWeapon(weapon) || EntIndexToEntRef(weapon) != g_AirblastWeapon[client]) return;
    // A state or held button alone is insufficient: FireAirBlast advances the
    // native secondary cooldown AND spends fuel. Phlog rage/charge/failed input don't qualify.
    if (HasEntProp(weapon, Prop_Send, "m_iWeaponState") && GetEntProp(weapon, Prop_Send, "m_iWeaponState") != 3) return;
    int ammo = g_AirblastAmmoSlot[client];
    if (ammo < 0 || ammo >= GetEntPropArraySize(client, Prop_Send, "m_iAmmo")) return;
    if (!PremiumAirblastFired(g_AirblastPrevious[client], GetEntPropFloat(weapon, Prop_Send, "m_flNextSecondaryAttack"),
        GetGameTime(), g_AirblastAmmo[client], GetEntProp(client, Prop_Send, "m_iAmmo", _, ammo))) return;
    if (cvAirblastForce.FloatValue <= 0.0 && cvAirblastLift.FloatValue <= 0.0) return;
    float angles[3], direction[3], before[3], velocity[3];
    GetClientEyeAngles(client, angles);
    if (!PremiumAirblastLookingDown(angles[0])) return;
    GetAngleVectors(angles, direction, NULL_VECTOR, NULL_VECTOR);
    GetEntPropVector(client, Prop_Data, "m_vecVelocity", before);
    PremiumAirblastImpulse(direction, before, cvAirblastForce.FloatValue, cvAirblastLift.FloatValue, cvAirblastCap.FloatValue, velocity);
    g_NextAirblast[client] = GetGameTime() + cvAirblastCooldown.FloatValue;
    if (velocity[2] > 0.0 && (GetEntityFlags(client) & FL_ONGROUND))
    {
        SetEntPropEnt(client, Prop_Send, "m_hGroundEntity", -1);
        SetEntityFlags(client, GetEntityFlags(client) & ~FL_ONGROUND);
    }
    TeleportEntity(client, NULL_VECTOR, NULL_VECTOR, velocity);
}
