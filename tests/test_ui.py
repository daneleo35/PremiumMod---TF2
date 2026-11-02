from pathlib import Path
import importlib.util
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))


class MenuCatalogChecks(unittest.TestCase):
    def test_generated_catalog_is_current_and_covers_every_nonstorage_setting(self):
        from tools.prepare_ui import render
        from tools.config_layout import SECTIONS
        text, count, editors = render()
        self.assertEqual(text, (ROOT/'scripting/premium/ui_settings_data.inc').read_text(encoding='utf-8'))
        self.assertEqual(count, sum(len(settings) for _, _, settings in SECTIONS[:-1]))
        self.assertEqual(editors, 105)
        self.assertNotIn('"premium_database"', text)
        self.assertNotIn('"premium_use_mysql"', text)

    def test_admin_actions_preserve_command_permissions_and_target_immunity(self):
        text = (ROOT/'scripting/premium/ui.inc').read_text()
        self.assertIn('CheckCommandAccess(client, "sm_premium_admin", ADMFLAG_ROOT)', text)
        self.assertIn('CanUserTarget(client, target)', text)
        self.assertIn('GetClientFromSerial(serial)', text)
        self.assertNotIn('ServerCommand(', text)
        self.assertNotIn('WriteMembership(', text)
        self.assertIn('FakeClientCommand(client, "sm_premadduser #%d"', text)
        self.assertIn('StringToInt(parts[1]) != g_UiSettingsRevision', text)

    def test_purchase_confirmation_uses_item_identity_not_visible_row_number(self):
        # The balance/availability row comes before the purchase button.
        text = (ROOT/'scripting/premium/economy.inc').read_text()
        handler = text.split('public int PointsPurchaseSelected(', 1)[1].split('public Action CommandPointsShop', 1)[0]
        self.assertNotIn('selection == 0', handler)
        self.assertIn('ShopMenuItem(menu, selection, index, true)', handler)
        self.assertIn('ShopCanBuy(client, index, reason, sizeof(reason))', handler)
        self.assertIn('QueueEconomyOperation(op)', handler)


if __name__ == '__main__': unittest.main()
