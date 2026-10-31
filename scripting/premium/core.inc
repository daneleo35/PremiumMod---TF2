// Pure policy helpers shared with the executable SourcePawn test harness.
#if defined _premium_core_included
 #endinput
#endif
#define _premium_core_included
#include <sdktools_gamerules>

int PremiumRemaining(int expires, int now)
{
    return expires > now ? expires - now : 0;
}

bool PremiumCanRespawn(int flags, int team, bool alive, bool arena, bool roundActive)
{
    return !(flags & (1 << 5)) && team >= 2 && !alive && !arena && roundActive;
}

bool PremiumRespawnPhaseAllowed(RoundState phase, bool roundActive)
{
    // Stalemate is sudden death: forcing a respawn bypasses elimination rules.
    return roundActive && (phase == RoundState_RoundRunning || phase == RoundState_Preround);
}

bool PremiumFlameFlightActive(int weaponState, int ammo)
{
    // Valve's FlameThrowerState_t: 1 STARTFIRING, 2 FIRING; 0 idle, 3 secondary.
    // Shot cooldown is independent of an ongoing stream of flames.
    return ammo > 0 && (weaponState == 1 || weaponState == 2);
}

int PremiumThrowableAmmoSlot(bool jarate, int primaryAmmo, int capacity)
{
    // CTFBat_Wood and CTFBat_Giftwrap explicitly use TF_AMMO_GRENADES1 (4),
    // regardless of the melee weapon's primary ammo metadata. Jars use primary.
    int slot = jarate ? primaryAmmo : 4;
    return slot >= 0 && slot < capacity ? slot : -1;
}

float PremiumClamp(float value, float minimum, float maximum)
{
    return value < minimum ? minimum : (value > maximum ? maximum : value);
}

// Rearm the engine's jump latch only on the ground. Clearing airborne inputs
// instead would consume Scout air-dashes or interfere with parachute toggles.
int PremiumGroundJumpLatch(int previous, int current, bool grounded)
{
    int jump = 1 << 1; // IN_JUMP from Source's in_buttons.h.
    return grounded && (current & jump) ? previous & ~jump : previous;
}

bool PremiumDigits(const char[] value)
{
    if (!value[0]) return false;
    for (int i; value[i]; i++)
        if (value[i] < '0' || value[i] > '9') return false;
    return true;
}

// Compare unsigned decimal strings without overflowing SourcePawn's 32-bit cell.
bool PremiumUInt32(const char[] value, int &account)
{
    if (!PremiumDigits(value)) return false;
    int start;
    while (value[start] == '0' && value[start + 1]) start++;
    int size = strlen(value[start]);
    if (size > 10 || (size == 10 && strcmp(value[start], "4294967295") > 0)) return false;
    account = 0;
    for (int i = start; value[i]; i++) account = account * 10 + value[i] - '0';
    return account != 0;
}

// Public-universe individual accounts only. Canonical key is Steam3, including
// unsigned account IDs above INT_MAX. Steam64 subtraction is decimal, not float.
bool PremiumNormalizeSteam(const char[] input, char[] output, int capacity)
{
    output[0] = '\0';
    int account;
    if (StrContains(input, "STEAM_", false) == 0)
    {
        int separators;
        for (int i; input[i]; i++) if (input[i] == ':') separators++;
        if (separators != 2) return false;
        char pieces[3][32];
        if (ExplodeString(input, ":", pieces, sizeof(pieces), sizeof(pieces[])) != 3) return false;
        if (!StrEqual(pieces[0], "STEAM_0", false) && !StrEqual(pieces[0], "STEAM_1", false)) return false;
        if (!StrEqual(pieces[1], "0") && !StrEqual(pieces[1], "1")) return false;
        if (!PremiumDigits(pieces[2]) || strlen(pieces[2]) > 10) return false;
        int half;
        // Zero is valid here when parity is one (account ID 1).
        if (StrEqual(pieces[2], "0")) half = 0;
        else if (!PremiumUInt32(pieces[2], half) || half < 0) return false;
        account = half * 2 + StringToInt(pieces[1]);
        if (!account) return false;
    }
    else if (StrContains(input, "[U:1:", false) == 0)
    {
        int length = strlen(input);
        if (length < 7 || length > 16 || input[length - 1] != ']') return false;
        char digits[16];
        strcopy(digits, sizeof(digits), input[5]);
        digits[strlen(digits) - 1] = '\0';
        if (!PremiumUInt32(digits, account)) return false;
    }
    else
    {
        if (strlen(input) != 17 || !PremiumDigits(input)) return false;
        if (strcmp(input, "76561197960265728") <= 0 || strcmp(input, "76561202255233023") > 0) return false;
        char base[] = "76561197960265728";
        char difference[18];
        difference[17] = '\0';
        int borrow;
        for (int i = 16; i >= 0; i--)
        {
            int digit = input[i] - base[i] - borrow;
            borrow = digit < 0 ? 1 : 0;
            difference[i] = (digit < 0 ? digit + 10 : digit) + '0';
        }
        if (!PremiumUInt32(difference, account)) return false;
    }
    FormatEx(output, capacity, "[U:1:%u]", account);
    return strlen(output) < capacity - 1;
}

int PremiumReserveAmmoIndex(int ammo, int capacity)
{
    // TF_AMMO_PRIMARY=1, TF_AMMO_SECONDARY=2. Never multiply metal (3),
    // grenade reserves (4+) or invalid/clipless no-ammo sentinels.
    return (ammo == 1 || ammo == 2) && ammo < capacity ? ammo - 1 : -1;
}

float PremiumApproach(float current, float target, float step)
{
    return current < target ? (current + step < target ? current + step : target)
        : (current - step > target ? current - step : target);
}

bool PremiumCanAddSentry(int owned, int limit)
{
    return limit > 1 && limit <= 3 && owned >= 0 && owned < limit;
}

float PremiumCloakAfterDrain(float before, float after, float multiplier)
{
    // Only scale a measured drain. Pickups, recharge and activation costs are
    // handled by the engine outside this before/after cloaked-think pair.
    if (multiplier <= 1.0 || before <= after || after <= 0.0) return after;
    return PremiumClamp(before - (before - after) / multiplier, 0.0, 100.0);
}

bool PremiumTeamAllowed(int team, int mask)
{
    return (team == 2 && (mask & 1) != 0) || (team == 3 && (mask & 2) != 0);
}

bool PremiumSwapBalanced(int sourceCount, int destinationCount, int maximumDifference)
{
    return maximumDifference == 0 || destinationCount + 1 - (sourceCount - 1) <= maximumDifference;
}

bool PremiumFlightWeapon(const char[] classname)
{
    // Backburner, Degreaser, Phlog and reskins share the first class.
    // Dragon's Fury is a distinct, pulse-firing weapon class.
    return StrEqual(classname, "tf_weapon_flamethrower")
        || StrEqual(classname, "tf_weapon_rocketlauncher_fireball");
}

float PremiumFlightVertical(float current, float pitchSine, float gravity, float lift, float acceleration, float dt)
{
    // Applied AFTER movement. Compensate this tick's actual gravity first,
    // then steer towards the configured climb/descent speed. No fixed 800-gravity assumption.
    return PremiumApproach(current + gravity * dt, lift * pitchSine, acceleration * dt);
}
