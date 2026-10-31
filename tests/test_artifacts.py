from pathlib import Path
import json
import re
import unittest
import tempfile
import importlib.util

ROOT = Path(__file__).resolve().parents[1]


def keyvalues(source):
    """Validate the conservative multiline SMC layout used by our text files.

    This is a format guard, not a replacement for SourceMod's full SMC parser.
    Newlines must not be discarded: a key/value followed by a closing section
    on the same line fails SourceMod even if generic KeyValues accepts it.
    """
    quoted = r'"(?:[^"\\]|\\.)*"'
    result = {}
    stack = [result]
    pending = None
    for number, raw in enumerate(source.splitlines(), 1):
        line = raw.strip()
        if not line or line.startswith('//'):
            continue
        if line == '{':
            if pending is None: raise ValueError(f'line {number}: section needs a header')
            child = {}
            stack[-1][pending] = child
            stack.append(child)
            pending = None
        elif line == '}':
            if pending is not None or len(stack) == 1:
                raise ValueError(f'line {number}: unmatched section ending')
            stack.pop()
        else:
            if pending is not None:
                raise ValueError(f'line {number}: missing section opening')
            if not re.fullmatch(rf'{quoted}(?:\s+{quoted})?', line):
                raise ValueError(f'line {number}: use one header, brace, or key/value per line')
            tokens = [json.loads(token) for token in re.findall(quoted, line)]
            key = tokens[0]
            if key in stack[-1]: raise ValueError(f'line {number}: duplicate key: {key}')
            if len(tokens) == 1:
                pending = key
            else:
                if len(stack) == 1: raise ValueError(f'line {number}: property outside section')
                stack[-1][key] = tokens[1]
    if pending is not None or len(stack) != 1:
        raise ValueError('unclosed section')
    return result


class ArtifactChecks(unittest.TestCase):
    def test_airblast_boost_uses_completed_native_fire_and_consumes_observation(self):
        source = (ROOT/'scripting/premium/airblast.inc').read_text()
        self.assertIn('!(GetClientButtons(client) & IN_ATTACK2)', source)
        self.assertIn('PremiumAirblastFired(', source)
        self.assertIn('g_AirblastSample[client] = false; // Consume', source)
        self.assertNotIn('CreateTimer(', source)
        self.assertNotIn('"m_iAmmo",', '\n'.join(line for line in source.splitlines() if 'SetEntProp(' in line))
        state = (ROOT/'scripting/premium/state.inc').read_text()
        self.assertIn('SDKHook_PreThinkPost, CaptureAirblast', state)
        self.assertIn('SDKHook_PostThinkPost, ApplyAirblastBoost', state)
        rules = (ROOT/'scripting/premium/rules.inc').read_text()
        self.assertIn('GameplayFeatureAllowed(client, Feature_Airblast) && ConfiguredAirblast(client)', rules)
        self.assertIn('return g_MapFlight && PremiumToggle(cvAirblast.BoolValue', rules)

    def test_rules_file_has_all_classes_and_safe_inheritance_defaults(self):
        config = keyvalues((ROOT/'configs/premium_rules.cfg').read_text())['PremiumRules']
        self.assertEqual(set(config['Classes']), {'scout', 'sniper', 'soldier', 'demoman', 'medic', 'heavy', 'pyro', 'spy', 'engineer'})
        for values in config['Classes'].values():
            self.assertEqual(values['ammo_multiplier'], '0')
            self.assertEqual(values['flight'], '-1')
            self.assertEqual(values['teleport'], '-1')
        self.assertEqual(set(config['Maps']), {'example_map_name'})
        source = (ROOT/'scripting/premium/rules.inc').read_text()
        self.assertIn('g_MapFlight = false; g_MapTeleport = false; return false;', source)
        self.assertIn('PremiumMapMatches(pattern, g_RuleMap[slash + 1])', source)

    def test_preferences_wait_for_cookies_without_overwriting_early_choices(self):
        source = (ROOT/'scripting/premium/options.inc').read_text()
        self.assertIn('CookieAccess_Private', source)
        self.assertIn('if (!AreClientCookiesCached(client)) return;', source)
        self.assertIn('if (g_OptionsDirty[client]) { SaveOptions(client); return; }', source)
        self.assertIn('SaveOptions(client)', source)
        chat = (ROOT/'scripting/premium/chat.inc').read_text()
        self.assertIn('g_Options[client].hideTag', chat)
        self.assertIn('cvTagStyles.IntValue &', chat)

    def test_profiles_default_to_round_start_and_swap_checks_share_one_gate(self):
        commands = (ROOT/'scripting/premium/commands.inc').read_text()
        effects = (ROOT/'scripting/premium/effects.inc').read_text()
        profile = commands.split('public Action CommandProfile(', 1)[1].split('public Action CommandFlight(', 1)[0]
        self.assertIn('strcopy(when, sizeof(when), "next")', profile)
        self.assertIn('StrEqual(when, "now")', profile)
        self.assertIn('ApplyPendingProfile()', effects.split('public void EventRoundStart(', 1)[1].split('public void EventRoundEnd(', 1)[0])
        swap = commands.split('public Action CommandSwap(', 1)[1].split('bool TeleportAllowed(', 1)[0]
        self.assertIn('SwapAvailability(client', swap)
        presentation = (ROOT/'scripting/premium/presentation.inc').read_text()
        self.assertIn('SwapAvailability(client', presentation.split('public Action CommandSwapStatus(', 1)[1])

    def test_cosmetic_filter_is_owned_and_quiet_messages_are_per_recipient(self):
        host = (ROOT/'scripting/premium/glow_host.inc').read_text()
        self.assertIn('SDKHook(entity, SDKHook_SetTransmit, PremiumCosmeticTransmit)', host)
        presentation = (ROOT/'scripting/premium/presentation.inc').read_text()
        self.assertIn('!g_Options[viewer].cosmetics ? Plugin_Handled', presentation)
        main = (ROOT/'scripting/premium.sp').read_text()
        ads = main.split('if (clock >= g_NextAdvert)', 1)[1]
        self.assertNotIn('CPrintToChatAll(', ads)
        self.assertIn('!g_Options[recipient].quiet', ads)
        effects = (ROOT/'scripting/premium/effects.inc').read_text()
        self.assertIn('if (!g_Options[client].quiet) Tell(client, "%t", phrase, reward)', effects)
        benefits = presentation.split('void DisplayConfiguredBenefits(', 1)[1].split('public Action CommandBenefits(', 1)[0]
        self.assertNotIn('GivePlayerAmmo(', benefits)
        self.assertNotIn('SetEntityHealth(', benefits)

    def test_reserve_ammo_is_spawn_only_and_never_modifies_magazines(self):
        source = (ROOT/'scripting/premium/ammo.inc').read_text()
        self.assertIn('"maxammo primary increased"', source)
        self.assertIn('"maxammo secondary increased"', source)
        self.assertIn('ApplyAttribute(client,', source)
        self.assertNotIn('SetEntProp(', source)
        self.assertNotIn('m_iClip', source)
        self.assertNotIn('clip size bonus', source)
        maintenance = source.split('void MaintainAmmoCapacity(', 1)[1].split('void GiveSpawnAmmo(', 1)[0]
        self.assertNotIn('GivePlayerAmmo(', maintenance)
        effects = (ROOT/'scripting/premium/effects.inc').read_text()
        spawn = effects.split('void ApplyEffects(', 1)[1].split('void SetThrowableAmmo(', 1)[0]
        self.assertLess(spawn.index('if (!spawn) return;'), spawn.index('GiveSpawnAmmo(client)'))
        self.assertNotIn('clip size bonus', effects)

    def test_reserve_config_and_legacy_alias_are_unambiguous(self):
        spec = importlib.util.spec_from_file_location('premium_build', ROOT/'build.py')
        build = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(build)
        config = build.create_config()
        self.assertRegex(config, r'(?m)^premium_ammo_multiplier\s+"1\.25"(?:\s|$)')
        self.assertNotRegex(config, r'(?m)^premium_clip_multiplier\s')
        source = (ROOT/'scripting/premium/config.inc').read_text()
        self.assertIn('PremiumSyncAmmoAlias(cvAmmo, cvLegacyClip)', source)
        self.assertIn('PremiumSyncAmmoAlias(cvLegacyClip, cvAmmo)', source)
        self.assertIn('if (g_AmmoAliasSync) return;', source)

    def test_throwables_change_native_capacity_without_heartbeat_ammo_refill(self):
        source = (ROOT/'scripting/premium/effects.inc').read_text()
        capacity = source.split('void MaintainThrowableCapacity(int client)', 1)[1]
        self.assertIn('"maxammo grenades1 increased", float(target)', capacity)
        self.assertNotIn('SetEntProp(', capacity)
        self.assertNotIn('CreateTimer(', capacity)

    def test_chat_tags_preserve_engine_recipients_and_message_text(self):
        source = (ROOT/'scripting/premium/chat.inc').read_text()
        self.assertIn('!PremiumPlayerChatFormat(format)', source)
        policy = (ROOT/'scripting/premium/chat_policy.inc').read_text()
        self.assertIn('format[0] ==', policy)
        self.assertIn('"TF_Chat_"', policy)
        self.assertIn('GetClientSerial(players[i])', source)
        self.assertIn('GetClientFromSerial(serial)', source)
        self.assertIn('StartMessageEx(g_ChatMessage, recipients, valid', source)
        self.assertIn('output.WriteString(parameters[i])', source)
        self.assertNotIn('CPrintToChatAll(', source)
        self.assertNotIn('USERMSG_BLOCKHOOKS', source)
        self.assertIn('PrefixPremiumName(author, name, 64)', source)
        self.assertIn('255 - overhead', source)

    def test_turret_tooltip_uses_native_owner_only_events_without_big_overlay(self):
        source = (ROOT/'scripting/premium/presentation.inc').read_text()
        main = (ROOT/'scripting/premium.sp').read_text()
        self.assertNotIn('ShowSyncHudText(', source)
        self.assertNotIn('SetHudTextParams(', source)
        self.assertNotIn('CreateHudSynchronizer(', main)
        self.assertIn('CreateEvent("show_annotation", true)', source)
        self.assertIn('CreateEvent("hide_annotation", true)', source)
        self.assertIn('event.SetInt("follow_entindex", target)', source)
        self.assertEqual(source.count('event.FireToClient(client); event.Cancel();'), 2)
        self.assertIn('event.SetInt("visibilityBitfield", 0)', source)
        self.assertIn('event.SetFloat("lifetime", 2.0)', source)
        self.assertIn('GetClientAimTarget(client, false)', source)
        self.assertIn('PlacedSentry(client, target)', source)
        self.assertNotIn('CreateEntityByName(', source)
        self.assertNotIn('1 << client', source)

    def test_extra_sentries_use_native_owner_registration_and_cleanup(self):
        source = (ROOT/'scripting/premium/buildings.inc').read_text()
        # Blueprints, costs, placement checks and registration stay in native
        # construction. The VM harness exercises our mode/role ownership logic.
        self.assertNotIn('CreateEntityByName(', source)
        self.assertNotIn('CommandSentry', source)
        self.assertIn('TrackPlacedSentry(client, event.GetInt("index", -1))', source)
        self.assertIn('CountOwnedBuildings(client, false) >= 6', source)
        self.assertNotIn('"m_iObjectType",', source)
        self.assertIn('"m_bWasMapPlaced", 1', source)
        self.assertIn('RestoreSentryModes(client)', source)
        self.assertIn('LockBonusSentries(client)', source)
        for file, count in [('premium.sp', 3), ('premium/effects.inc', 2)]:
            code = (ROOT/'scripting'/file).read_text()
            self.assertGreaterEqual(code.count('RemoveExtraBuildings(client)'), count)

    def test_extra_utilities_share_native_slots_and_restore_pda_flags(self):
        source = (ROOT/'scripting/premium/utility_buildings.inc').read_text()
        self.assertNotIn('CreateEntityByName(', source)
        self.assertNotIn('SDKCall(', source)
        self.assertNotIn('LoadFromAddress(', source)
        self.assertNotIn('"m_iObjectType",', source)
        self.assertNotIn('"m_iObjectMode", 2', source)
        self.assertIn('CountOwnedBuildings(client, false) >= 6', source)
        self.assertIn('CountOwnedBuildings(client, false) > 6', source)
        self.assertIn('RestoreUtilityMasks(client)', source)
        config = (ROOT/'scripting/premium/config.inc').read_text()
        for setting in ('premium_dispenser_limit', 'premium_teleporter_entrance_limit', 'premium_teleporter_exit_limit'):
            self.assertIn(f'Setting("{setting}", "3",', config)
        effects = (ROOT/'scripting/premium/effects.inc').read_text()
        callback = effects.split('public Action TF2_CalcIsAttackCritical(', 1)[1]
        self.assertLess(callback.index('ProtectUtilityWrench(client, classname)'), callback.index('if (!GameplayAllowed(client)'))
        main = (ROOT/'scripting/premium.sp').read_text()
        self.assertIn('AddCommandListener(LimitNativeBuildings, "build")', main)
        self.assertIn('BuildingPlacementBlocked(client)', effects)
        presentation = (ROOT/'scripting/premium/presentation.inc').read_text()
        self.assertIn('All buildings: %d/6 slots', presentation)
        self.assertIn('Buildings: %d/6', presentation)

    def test_independent_grants_do_not_change_membership_or_chat_tags(self):
        runtime = (ROOT/'scripting/premium/feature_runtime.inc').read_text()
        state = (ROOT/'scripting/premium/state.inc').read_text()
        membership = state.split('bool HasPremium(', 1)[1].split('bool GameplayAllowed(', 1)[0]
        self.assertIn('PremiumMembership(', membership)
        self.assertIn('cvTrial.BoolValue', membership)
        for forbidden in ('g_ClientFeatures', 'g_PublicFeatures', 'g_UserFeatures', 'HasFeature('):
            self.assertNotIn(forbidden, membership)
        self.assertIn('GetCommandOverride(overrideName, Override_Command, flags)', runtime)
        self.assertIn('PremiumFeatureSources(HasPremium(client)', runtime)
        chat = (ROOT/'scripting/premium/chat.inc').read_text()
        self.assertIn('!HasPremium(client)', chat)
        self.assertNotIn('HasAnyFeature(', chat)
        self.assertNotIn('HasFeature(', chat)
        access = (ROOT/'scripting/premium/feature_access.inc').read_text()
        for forbidden in ('MEMBER_TABLE', 'TRIAL_TABLE', 'WriteMembership(', 'SQL_', 'g_Player[client].member =', 'SetUserFlagBits('):
            self.assertNotIn(forbidden, access)
        commands = (ROOT/'scripting/premium/commands.inc').read_text()
        for name in ('premium_feature', 'premium_feature_public', 'premium_features_reload'):
            self.assertIn(f'AdminCommand("{name}",', commands)
        self.assertIn('RenameFile(backup, path)', access)
        self.assertIn('RenameFile(path, temporary)', access)
        self.assertIn('RenameFile(path, backup)', access)

    def test_feature_file_defaults_closed_and_has_every_supported_key(self):
        config = keyvalues((ROOT/'configs/premium_features.cfg').read_text())['PremiumFeatures']
        policy = (ROOT/'scripting/premium/feature_policy.inc').read_text()
        keys = set(re.findall(r'"([a-z_]+)"', policy.split('char g_FeatureKeys[][] =', 1)[1].split('};', 1)[0]))
        self.assertEqual(set(config), {'Public', 'Users'})
        self.assertEqual(set(config['Public']), keys)
        self.assertEqual(len(keys), 29)
        self.assertEqual(set(config['Public'].values()), {'0'})
        self.assertEqual(config['Users'], {})
        build = (ROOT/'build.py').read_text()
        self.assertIn("'addons/sourcemod/configs/premium_features.cfg': ROOT/'configs/premium_features.cfg'", build)

    def test_each_automatic_feature_uses_its_own_access_gate(self):
        effects = (ROOT/'scripting/premium/effects.inc').read_text()
        state = (ROOT/'scripting/premium/state.inc').read_text()
        for feature in ('Health', 'Speed', 'Gravity', 'Uber', 'Metal', 'Jarate', 'Balls', 'Respawn', 'FireArrows'):
            self.assertIn(f'GameplayFeatureAllowed(client, Feature_{feature})', effects)
        for feature in ('Dark', 'Color', 'Fov'):
            self.assertIn(f'HasFeature(client, Feature_{feature})', effects)
        self.assertIn('"Kill Reward", Feature_KillHealth)', effects)
        self.assertIn('"Assist Reward", Feature_AssistHealth)', effects)
        self.assertIn('HasFeature(attacker, Feature_FakeKill)', effects)
        for feature in ('Health', 'Respawn', 'Teleport'):
            self.assertIn(f'GameplayFeatureAllowed(client, Feature_{feature})', state)
        for module, feature in [('ammo', 'Ammo'), ('flight', 'Flight'), ('airblast', 'Airblast'), ('buildings', 'Sentry')]:
            text = (ROOT/f'scripting/premium/{module}.inc').read_text()
            self.assertIn(f'GameplayFeatureAllowed(client, Feature_{feature})', text)
            self.assertNotIn('HasPremium(', text)
        self.assertIn('HasFeature(client, Feature_Glow)', (ROOT/'scripting/premium/glow_host.inc').read_text())
        self.assertIn('HasFeature(client, Feature_Celebration)', (ROOT/'scripting/premium/celebrations.inc').read_text())
        self.assertIn('Feature_Dispenser : Feature_Teleporter', (ROOT/'scripting/premium/utility_buildings.inc').read_text())
        self.assertIn('Feature_DeadRinger : Feature_Cloak', (ROOT/'scripting/premium/cloak.inc').read_text())
        # Unrelated grant reconciliation retains bonus ownership until health
        # permission is removed; otherwise later revocation could leak health.
        restore = effects.split('void RestoreGameplayEffects(', 1)[1].split('void RestoreEffects(', 1)[0]
        self.assertIn('if (!IsPlayerAlive(client) || !GameplayFeatureAllowed(client, Feature_Health)) g_Player[client].bonusLifeHealth = 0;', restore)

    def test_points_system_defaults_disabled_with_separate_optional_controls(self):
        config = (ROOT/'scripting/premium/config.inc').read_text()
        self.assertIn('Setting("premium_points_enable", "0"', config)
        self.assertIn('Setting("premium_points_trials", "0"', config)
        for setting in ('earning', 'shop', 'rentals', 'earn_notice', 'purchase_notice', 'expiry_notice', 'renewals'):
            self.assertIn(f'Setting("premium_points_{setting}"', config)
        shop = keyvalues((ROOT/'configs/premium_shop.cfg').read_text())['PremiumShop']['Items']
        self.assertEqual(len(shop), 4)
        for offer in shop.values():
            self.assertIn(offer['unit'], ('minutes', 'hours', 'days'))
            self.assertGreater(int(offer['price']), 0)
        self.assertTrue(any(item['feature'] == 'trial' for item in shop.values()))
        build = (ROOT/'build.py').read_text()
        self.assertIn("'addons/sourcemod/configs/premium_shop.cfg': ROOT/'configs/premium_shop.cfg'", build)

    def test_economy_uses_durable_tokens_and_cancels_stale_quotes(self):
        journal = (ROOT/'scripting/premium/economy_outbox.inc').read_text()
        queries = (ROOT/'scripting/premium/economy_queries.inc').read_text()
        sql = (ROOT/'scripting/premium/economy_sql.inc').read_text()
        controller = (ROOT/'scripting/premium/economy.inc').read_text()
        self.assertIn('SaveEconomyOutbox()', journal)
        self.assertIn('g_EconomyOutboxCorrupt = true', journal)
        self.assertIn('if (!current || !g_EconomyOutbox.Length) return;', sql)
        self.assertIn('if (!StrEqual(token, op.token)) return;', sql)
        self.assertIn('NOT EXISTS (SELECT 1 FROM premium_v3_receipts', queries)
        self.assertIn('point_change, expires, accepted)', queries)
        self.assertIn('trial_purchases = trial_purchases + 1', queries)
        self.assertIn('StringToInt(parts[0]) != g_ShopRevision', controller)
        self.assertIn('Menu confirm = new Menu(PointsPurchaseSelected)', controller)
        self.assertIn('ResolveBaseFeatureMask(client)', controller)
        self.assertIn('EconomyEarnPending(g_Economy[client].steam)', controller)
        self.assertIn('CPrintToChat(client, "%s %s", tag, message)', controller)
        self.assertNotIn('CPrintToChatAll(', controller + sql)
        self.assertIn('!g_Options[client].quiet', sql)
        self.assertIn('g_Database.Execute(transaction, EconomyCommitted, EconomyCommitFailed', sql)

    def test_bought_trial_is_separate_from_free_trial_and_feature_rentals(self):
        state = (ROOT/'scripting/premium/economy_state.inc').read_text()
        membership = (ROOT/'scripting/premium/state.inc').read_text().split('bool HasPremium(', 1)[1].split('bool GameplayAllowed(', 1)[0]
        runtime = (ROOT/'scripting/premium/feature_runtime.inc').read_text()
        self.assertIn('EconomyTrialActive(client)', membership)
        self.assertIn('if (rentals) user |= EconomyRentalMask(client)', runtime)
        self.assertNotIn('EconomyRentalMask(', membership)
        self.assertIn('cvPointsTrials.BoolValue', state)
        self.assertIn('cvPointsRentals.BoolValue', state)
        sql = (ROOT/'scripting/premium/economy_sql.inc').read_text() + (ROOT/'scripting/premium/economy_queries.inc').read_text()
        self.assertNotIn('UPDATE premium_v3_trials', sql)
        self.assertNotIn('premium_v3_members SET', sql)
        commands = (ROOT/'scripting/premium/commands.inc').read_text()
        self.assertIn('AdminCommand("premium_points_adjust", CommandPointsAdjust)', commands)
        self.assertIn('AdminCommand("premium_shop_reload", CommandShopReload)', commands)

    def test_smc_layout_rejects_the_reported_translation_failure(self):
        bad = '"Phrases"\n{\n    "In Game Only" { "en" "Use this command while connected to the game." }\n}\n'
        with self.assertRaisesRegex(ValueError, 'line 3:'):
            keyvalues(bad)
        # A key/value must also end before its closing brace or next property.
        for invalid in ('"en" "text" }', '"#format" "{1:d}" "en" "{1}"'):
            with self.subTest(invalid=invalid), self.assertRaises(ValueError):
                keyvalues('"Phrases"\n{\n"Example"\n{\n' + invalid + '\n}\n}\n')
        valid = '"Phrases"\n{\n"Example"\n{\n"#format" "{1:d}"\n"en" "Value: {1}"\n}\n}\n'
        self.assertEqual(keyvalues(valid)['Phrases']['Example']['en'], 'Value: {1}')

    def test_original_command_contracts_including_glow_are_available(self):
        source = (ROOT/'scripting/premium/commands.inc').read_text()
        public = ('premium', 'trial', 'save_loc', 'load_loc', 'premium_fov', 'dark', 'glow', 'swapteam', 'colorme')
        admin = ('premadduser', 'premremuser', 'prem_reload', 'createtrial', 'resettrial', 'premium_database_create')
        for name in public:
            self.assertRegex(source, rf'PublicCommand\("{name}",\s*\w+\)')
        for name in admin:
            self.assertRegex(source, rf'AdminCommand\("{name}",\s*\w+\)')
        self.assertIn('menu.AddItem("glow"', (ROOT/'scripting/premium/ui.inc').read_text())

    def test_glow_is_wired_to_each_ownership_cleanup_boundary(self):
        main = (ROOT/'scripting/premium.sp').read_text()
        effects = (ROOT/'scripting/premium/effects.inc').read_text()
        state = (ROOT/'scripting/premium/state.inc').read_text()
        host = (ROOT/'scripting/premium/glow_host.inc').read_text()
        def body(source, name):
            tail = re.split(r'\b' + name + r'\([^)]*\)\s*\{', source, maxsplit=1)[1]
            depth = 1
            for index, character in enumerate(tail):
                if character == '{': depth += 1
                elif character == '}': depth -= 1
                if depth == 0: return tail[:index]
            self.fail(f'unclosed function {name}')
        for name in ('OnMapEnd', 'OnPluginEnd'):
            self.assertIn('StopGlow(client)', body(main, name))
        self.assertIn('ResetGlow(client)', body(main, 'OnClientDisconnect'))
        self.assertIn('StopGlow(client)', body(effects, 'RestoreEffects'))
        self.assertIn('StopGlow(client)', body(effects, 'EventRoundEnd'))
        self.assertIn('ResetGlow(client)', body(state, 'RefreshClient'))
        self.assertIn('StopGlow(client)', body(host, 'TF2_OnConditionAdded'))
        self.assertIn('RemoveEntity(entity)', body(host, 'GlowHostDestroy'))
        self.assertNotIn('CreateTimer(', host + (ROOT/'scripting/premium/glow.inc').read_text())

    def test_missing_build_dependencies_produce_actionable_warning(self):
        spec = importlib.util.spec_from_file_location('premium_build', ROOT/'build.py')
        build = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(build)
        scratch = ROOT/'tests/tmp'
        scratch.mkdir(exist_ok=True)
        with tempfile.TemporaryDirectory(dir=scratch) as location:
            isolated = Path(location)
            self.assertTrue(isolated.resolve().is_relative_to(scratch.resolve()))
            with self.assertRaises(SystemExit) as failure:
                build.check_build_dependencies(isolated, isolated/'compiler')
            message = str(failure.exception)
            for expected in ('morecolors.inc', 'tf2attributes.inc', 'https://github.com', 'scripting/include', 'not a runtime plugin'):
                self.assertIn(expected, message)

    def test_releases_exclude_separate_third_party_dependency_files(self):
        spec = importlib.util.spec_from_file_location('premium_build', ROOT/'build.py')
        build = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(build)
        self.assertEqual({'morecolors.inc', 'tf2attributes.inc'}, build.EXTERNAL_HEADERS)
        source = (ROOT/'build.py').read_text()
        for forbidden in ("'addons/sourcemod/plugins/tf2attributes.smx':", "'addons/sourcemod/gamedata/tf2.attributes.txt':", "members['vendor/tf2attributes.sp']"):
            self.assertNotIn(forbidden, source)

    def test_all_literal_translation_references_are_supplied(self):
        phrases = keyvalues((ROOT/'translations/premium_rebuilt.phrases.txt').read_text())['Phrases']
        source = '\n'.join(p.read_text(encoding='utf-8-sig') for p in (ROOT/'scripting').rglob('*') if p.suffix in ('.sp', '.inc') and p.parent.name != 'include')
        references = set(re.findall(r'"%[tT]",\s*"([^"\n]+)"', source))
        self.assertGreater(len(references), 30)
        self.assertFalse(references - phrases.keys(), references - phrases.keys())
        for name, phrase in phrases.items():
            self.assertIn('en', phrase, name)
            formatting = phrase.get('#format', '')
            variables = set(re.findall(r'\{(\d+)\}', phrase['en']))
            declared = set(re.findall(r'\{(\d+):[a-zA-Z]\}', formatting))
            self.assertEqual(variables, declared, name)

    def test_no_admin_flag_mutation_or_blocking_sql(self):
        source = '\n'.join(p.read_text() for p in (ROOT/'scripting/premium').glob('*.inc'))
        for forbidden in ('SetAdminFlag(', 'SetUserFlagBits(', 'AddUserFlags(', 'RemoveUserFlags(', 'SQL_Query(', 'SQL_Connect('):
            self.assertNotIn(forbidden, source)

    def test_movement_callback_does_not_create_timers(self):
        source = (ROOT/'scripting/premium/effects.inc').read_text()
        movement = source.split('public Action OnPlayerRunCmd', 1)[1]
        self.assertNotIn('CreateTimer(', movement)
        self.assertNotIn('m_flMaxspeed', movement)
        self.assertIn('MovementBlocked(client)', movement)
        flight = (ROOT/'scripting/premium/flight.inc').read_text()
        self.assertIn('FL_FROZEN', flight)
        self.assertIn('TFCond_FreezeInput', flight)
        self.assertIn('PremiumFlameFlightActive(', flight)
        self.assertNotIn('m_flNextPrimaryAttack', flight)
        state = (ROOT/'scripting/premium/state.inc').read_text()
        self.assertIn('SDKHook_PostThinkPost, ApplyFlameFlight', state)
        self.assertNotIn('cvFlight.BoolValue', movement)

    def test_metal_is_spawn_only_and_cloak_has_no_full_meter_heartbeat(self):
        effects = (ROOT/'scripting/premium/effects.inc').read_text()
        spawn = effects.split('void ApplyEffects(', 1)[1].split('void SetThrowableAmmo(', 1)[0]
        self.assertLess(spawn.index('if (!spawn) return;'), spawn.index('"m_iAmmo", cvMetal.IntValue'))
        all_source = '\n'.join(p.read_text() for p in (ROOT/'scripting/premium').glob('*.inc'))
        self.assertNotIn('CommandMetal(', all_source)
        self.assertNotIn('premium_deadringer_infinite', all_source)
        main = (ROOT/'scripting/premium.sp').read_text()
        self.assertNotIn('"m_flCloakMeter", 100.0', main + all_source)
        state = (ROOT/'scripting/premium/state.inc').read_text()
        self.assertIn('SDKHook_PreThink, CaptureCloakDrain', state)
        self.assertIn('SDKHook_PreThinkPost, ScaleCloakDrain', state)

    def test_profiles_use_known_settings_and_share_complete_gameplay_controls(self):
        spec = importlib.util.spec_from_file_location('premium_build', ROOT/'build.py')
        build = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(build)
        defaults = dict(re.findall(r'^(premium_\w+)\s+"([^"]*)"(?:\s*//[^\n]*)?$', build.create_config(), re.M))
        controls = None
        for name in ('cosmetic', 'balanced', 'fun'):
            source = (ROOT/f'configs/premium_profiles/{name}.cfg').read_text()
            values = dict(re.findall(r'^(premium_\w+)\s+"([^"]*)"(?:\s*//[^\n]*)?$', source, re.M))
            self.assertFalse(values.keys() - defaults.keys(), name)
            self.assertNotIn('premium_flags', values)
            self.assertNotIn('premium_database', values)
            self.assertNotIn('premium_trial_timelimit', values)
            if controls is None: controls = values.keys()
            self.assertEqual(controls, values.keys())
            self.assertIn('premium_cloak_multiplier', values)
            self.assertIn('premium_deadringer_multiplier', values)
            if name == 'cosmetic': self.assertEqual(values['premium_gameplay_teams'], '0')

    def test_delayed_respawns_are_serial_identified_and_map_scoped(self):
        source = (ROOT/'scripting/premium/effects.inc').read_text()
        creation = next(line for line in source.splitlines() if 'CreateTimer(' in line)
        self.assertIn('GetClientSerial(victim)', creation)
        self.assertIn('TIMER_FLAG_NO_MAPCHANGE', creation)
        self.assertIn('g_Player[client].respawn != timer', source)
        self.assertIn('PremiumRespawnPhaseAllowed(state, g_RoundActive)', source)
        self.assertIn('PremiumThrowableAmmoSlot(jarate, primary, GetEntPropArraySize(client, Prop_Send, "m_iAmmo"))', source)


if __name__ == '__main__':
    unittest.main(verbosity=2)
