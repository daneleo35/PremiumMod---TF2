#pragma semicolon 1
#pragma newdecls required
#include <sourcemod>
#include "../scripting/premium/core.inc"

int g_Checks;

void Expect(bool condition, const char[] label)
{
    g_Checks++;
    if (!condition) ThrowError("FAIL: %s", label);
}

void ExpectIdentity(const char[] input, const char[] expected)
{
    char normalized[32];
    Expect(PremiumNormalizeSteam(input, normalized, sizeof(normalized)), input);
    Expect(StrEqual(normalized, expected), input);
}

void Reject(const char[] input)
{
    char normalized[32];
    Expect(!PremiumNormalizeSteam(input, normalized, sizeof(normalized)), input);
}

// `main` is runnable in a SourcePawn VM; `OnPluginStart` also supports a test server.
public int main()
{
    ExpectIdentity("STEAM_0:1:4153990", "[U:1:8307981]");
    ExpectIdentity("STEAM_1:1:4153990", "[U:1:8307981]");
    ExpectIdentity("[U:1:8307981]", "[U:1:8307981]");
    ExpectIdentity("76561197968573709", "[U:1:8307981]");
    ExpectIdentity("STEAM_0:1:0", "[U:1:1]");
    ExpectIdentity("76561197960265729", "[U:1:1]");
    ExpectIdentity("[U:1:4294967295]", "[U:1:4294967295]");
    ExpectIdentity("STEAM_1:1:2147483647", "[U:1:4294967295]");
    ExpectIdentity("76561202255233023", "[U:1:4294967295]");
    ExpectIdentity("[U:1:2147483648]", "[U:1:2147483648]");
    Reject(""); Reject("BOT"); Reject("STEAM_ID_PENDING");
    Reject("STEAM_0:2:123"); Reject("STEAM_2:1:123");
    Reject("STEAM_0:1:1:ignored"); Reject("STEAM_0:1:-1");
    Reject("STEAM_0:1:2147483648"); Reject("STEAM_0:0:0");
    Reject("[U:1:0]"); Reject("[U:2:123]"); Reject("[U:1:4294967296]");
    Reject("[U:1:-1]"); Reject("[U:1:1]extra"); Reject("[U:1:1'); DROP TABLE premium_v3_members;--]");
    Reject("76561197960265728"); Reject("76561202255233024"); Reject("76561197968573709x");
    Expect(PremiumRemaining(1000, 900) == 100, "remaining duration");
    Expect(PremiumRemaining(1000, 1000) == 0, "exact expiry");
    Expect(PremiumRemaining(1000, 1001) == 0, "past expiry");
    Expect(PremiumRemaining(0, 1000) == 0, "no trial");
    for (int flags = 0; flags < 512; flags++)
        Expect(PremiumCanRespawn(flags, 2, false, false, true) == ((flags & 32) == 0), "death flag combinations");
    Expect(!PremiumCanRespawn(0, 1, false, false, true), "spectator exclusion");
    Expect(!PremiumCanRespawn(0, 2, true, false, true), "living player exclusion");
    Expect(!PremiumCanRespawn(0, 2, false, true, true), "arena exclusion");
    Expect(!PremiumCanRespawn(0, 2, false, false, false), "round-end exclusion");
    for (int state = view_as<int>(RoundState_Init); state <= view_as<int>(RoundState_BetweenRounds); state++)
    {
        RoundState phase = view_as<RoundState>(state);
        bool safe = phase == RoundState_Preround || phase == RoundState_RoundRunning;
        Expect(PremiumRespawnPhaseAllowed(phase, true) == safe, "respawn phase including sudden death");
        Expect(!PremiumRespawnPhaseAllowed(phase, false), "round-end event blocks every respawn phase");
    }
    Expect(!PremiumFlameFlightActive(0, 100), "idle flamethrower has no flight");
    Expect(PremiumFlameFlightActive(1, 100), "starting flames permit flight");
    Expect(PremiumFlameFlightActive(2, 100), "continuous flames permit flight");
    Expect(!PremiumFlameFlightActive(3, 100), "airblast has no flame flight");
    Expect(!PremiumFlameFlightActive(-1, 100), "missing state has no flight");
    Expect(!PremiumFlameFlightActive(4, 100), "unknown state has no flight");
    Expect(!PremiumFlameFlightActive(2, 0), "empty flamethrower has no flight");
    Expect(!PremiumFlameFlightActive(2, -1), "invalid ammo has no flight");
    Expect(PremiumThrowableAmmoSlot(false, -1, 8) == 4, "Sandman with no primary ammo still gets balls");
    Expect(PremiumThrowableAmmoSlot(false, 1, 8) == 4, "melee metadata cannot change bullet ammo");
    Expect(PremiumThrowableAmmoSlot(false, 99, 8) == 4, "balls use the game grenade slot");
    Expect(PremiumThrowableAmmoSlot(true, 4, 8) == 4, "Jarate retains its primary ammo pool");
    Expect(PremiumThrowableAmmoSlot(true, -1, 8) == -1, "invalid Jarate ammo rejected");
    Expect(PremiumThrowableAmmoSlot(true, 8, 8) == -1, "Jarate ammo upper bound rejected");
    Expect(PremiumThrowableAmmoSlot(false, -1, 4) == -1, "missing grenade slot rejected");
    Expect(PremiumThrowableAmmoSlot(true, 0, 0) == -1, "empty ammo table rejected");
    for (int rate = 66; rate <= 100; rate += 34)
    {
        float impulse;
        // A running stream receives all of one second's recoil, including ticks
        // between emitted flame packets; no per-shot cooldown enters this rule.
        for (int tick; tick < rate; tick++)
            if (PremiumFlameFlightActive(2, 100)) impulse += 990.0 / float(rate);
        Expect(FloatAbs(impulse - 990.0) < 0.01, "continuous recoil independent of tick rate");
    }
    Expect(PremiumReserveAmmoIndex(1, 8) == 0, "primary bullets, rockets and Pyro fuel use first reserve");
    Expect(PremiumReserveAmmoIndex(2, 8) == 1, "secondary shells and flares use second reserve");
    Expect(PremiumReserveAmmoIndex(3, 8) == -1, "metal is controlled separately");
    Expect(PremiumReserveAmmoIndex(4, 8) == -1, "throwable reserves are controlled separately");
    Expect(PremiumReserveAmmoIndex(-1, 8) == -1, "energy or no-ammo sentinel excluded");
    Expect(PremiumReserveAmmoIndex(0, 8) == -1, "dummy ammo slot excluded");
    Expect(PremiumReserveAmmoIndex(2, 2) == -1, "ammo index must fit the array");
    Expect(PremiumReserveAmmoIndex(1, 0) == -1, "empty ammo array excluded");
    Expect(PremiumCanAddSentry(0, 3), "first sentry allowed");
    Expect(PremiumCanAddSentry(1, 3), "second sentry allowed");
    Expect(PremiumCanAddSentry(2, 3), "third sentry allowed");
    Expect(!PremiumCanAddSentry(3, 3), "fourth sentry denied");
    Expect(!PremiumCanAddSentry(0, 1), "extra sentry feature disabled at one");
    Expect(!PremiumCanAddSentry(-1, 3), "invalid building count denied");
    Expect(FloatAbs(PremiumCloakAfterDrain(100.0, 99.0, 2.0) - 99.5) < 0.001, "twice cloak duration halves actual drain");
    Expect(PremiumCloakAfterDrain(50.0, 51.0, 2.0) == 51.0, "natural recharge and pickups unchanged");
    Expect(PremiumCloakAfterDrain(50.0, 49.0, 1.0) == 49.0, "one means native cloak");
    Expect(PremiumCloakAfterDrain(0.1, 0.0, 20.0) == 0.0, "terminal native decloak never reversed");
    for (int multiplier = 1; multiplier <= 20; multiplier++)
    {
        for (int start = 50; start <= 100; start += 50)
        {
            float meter = float(start), dt = 1.0 / 66.0;
            int ticks;
            while (meter > 0.0 && ticks < 20000)
            {
                float nativeAfter = meter - 10.0 * dt;
                if (nativeAfter < 0.0) nativeAfter = 0.0;
                meter = PremiumCloakAfterDrain(meter, nativeAfter, float(multiplier));
                ticks++;
            }
            Expect(meter == 0.0, "finite cloak always exhausts, including initial Dead Ringer half meter");
            Expect(FloatAbs(float(ticks) * dt - float(start * multiplier) / 10.0) < 0.35,
                "extended drain duration respects multiplier within native terminal tick rounding");
        }
    }
    for (int team; team <= 4; team++)
        for (int mask; mask <= 3; mask++)
            Expect(PremiumTeamAllowed(team, mask) == ((team == 2 && (mask & 1) != 0) || (team == 3 && (mask & 2) != 0)), "team gameplay mask");
    Expect(!PremiumSwapBalanced(5, 5, 1), "equal teams cannot become two apart");
    Expect(PremiumSwapBalanced(6, 4, 1), "swap can rebalance teams");
    Expect(!PremiumSwapBalanced(4, 6, 1), "swap cannot enlarge a stacked team");
    Expect(PremiumSwapBalanced(4, 6, 0), "owner can allow uneven swaps");
    Expect(PremiumFlightWeapon("tf_weapon_flamethrower"), "Backburner, Degreaser, Phlog and flame reskins share support");
    Expect(PremiumFlightWeapon("tf_weapon_rocketlauncher_fireball"), "Dragon Fury is a supported separate class");
    Expect(!PremiumFlightWeapon("tf_weapon_shotgun_pyro"), "Pyro shotgun cannot activate flight");
    for (int gravity = 400; gravity <= 1600; gravity += 400)
        for (int rate = 66; rate <= 100; rate += 34)
        {
            float velocity, dt = 1.0 / float(rate);
            for (int tick; tick < rate; tick++)
            {
                velocity -= float(gravity) * dt; // native movement before our hook
                velocity = PremiumFlightVertical(velocity, 1.0, float(gravity), 200.0, 600.0, dt);
            }
            Expect(FloatAbs(velocity - 200.0) < 0.02, "sustained climb after native movement across gravity/tick rates");
            for (int tick; tick < rate; tick++)
            {
                velocity -= float(gravity) * dt;
                velocity = PremiumFlightVertical(velocity, 0.0, float(gravity), 200.0, 600.0, dt);
            }
            Expect(FloatAbs(velocity) < 0.02, "aiming level hovers instead of sinking after initial flight");
        }
    Expect(FloatAbs(PremiumApproach(-100.0, 300.0, 20.0) + 80.0) < 0.001, "flight counters reverse drift toward input");
    Expect(FloatAbs(PremiumApproach(100.0, 0.0, 20.0) - 80.0) < 0.001, "flight slows when input released");
    Expect(FloatAbs(PremiumApproach(10.0, 15.0, 20.0) - 15.0) < 0.001, "flight does not overshoot positive target");
    Expect(FloatAbs(PremiumApproach(-10.0, -15.0, 20.0) + 15.0) < 0.001, "flight does not overshoot negative target");
    Expect(FloatAbs(PremiumClamp(0.0, 1.0, 2.0) - 1.0) < 0.0001, "clamp lower boundary");
    Expect(FloatAbs(PremiumClamp(3.0, 1.0, 2.0) - 2.0) < 0.0001, "clamp upper boundary");
    Expect(FloatAbs(PremiumClamp(1.5, 1.0, 2.0) - 1.5) < 0.0001, "clamp interior");
    Expect(PremiumGroundJumpLatch(2, 2, true) == 0, "held jump rearms on landing");
    Expect(PremiumGroundJumpLatch(3, 3, true) == 1, "rearm preserves attack input");
    Expect(PremiumGroundJumpLatch(0, 2, true) == 0, "fresh jump is unchanged");
    Expect(PremiumGroundJumpLatch(2, 0, true) == 2, "released input left to engine");
    Expect(PremiumGroundJumpLatch(2, 2, false) == 2, "held airborne jump preserves native latch");
    Expect(PremiumGroundJumpLatch(0, 2, false) == 0, "fresh airborne Scout or parachute input preserved");
    PrintToServer("PASS: %d policy assertions", g_Checks);
    return g_Checks;
}

public void OnPluginStart()
{
    main();
}
