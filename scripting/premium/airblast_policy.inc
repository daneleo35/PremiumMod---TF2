bool PremiumAirblastLookingDown(float pitch)
{
    // Source eye pitch is positive below the horizon; level/upward aim cannot boost.
    return pitch > 0.0 && pitch <= 90.0;
}

bool PremiumAirblastFired(float previousCooldown, float currentCooldown, float clock, int beforeAmmo, int afterAmmo)
{
    return previousCooldown >= 0.0 && currentCooldown > previousCooldown && currentCooldown > clock
        && beforeAmmo > 0 && afterAmmo >= 0 && afterAmmo < beforeAmmo;
}

void PremiumAirblastImpulse(const float direction[3], const float before[3], float force, float lift, float cap, float result[3])
{
    result[0] = before[0] - direction[0] * force;
    result[1] = before[1] - direction[1] * force;
    float upward = -direction[2] * force;
    if (upward < lift) upward = lift;
    result[2] = before[2] + upward;
    float oldHorizontal = SquareRoot(before[0] * before[0] + before[1] * before[1]);
    float horizontal = SquareRoot(result[0] * result[0] + result[1] * result[1]);
    float maximum = oldHorizontal > cap ? oldHorizontal : cap;
    if (horizontal > maximum && horizontal > 0.0)
    { result[0] *= maximum / horizontal; result[1] *= maximum / horizontal; }
    maximum = before[2] > cap ? before[2] : cap;
    if (result[2] > maximum) result[2] = maximum;
}
