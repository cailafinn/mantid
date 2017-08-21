from __future__ import (absolute_import, division, print_function)

import mantid
import tempfile
import unittest
import mock
from mantid.kernel import config
import os
import json
from sans.gui_logic.presenter.settings_diagnostic_presenter import SettingsDiagnosticPresenter
from sans.test_helper.mock_objects import (create_run_tab_presenter_mock, FakeState, create_mock_settings_diagnostic_tab)


class SettingsDiagnosticPresenterTest(unittest.TestCase):
    def test_that_can_get_state_for_index(self):
        parent_presenter = create_run_tab_presenter_mock()
        presenter = SettingsDiagnosticPresenter(parent_presenter)
        state = presenter.get_state(3)
        self.assertTrue(isinstance(state, FakeState))

    def test_that_updates_tree_view_when_row_selection_changes(self):
        parent_presenter = create_run_tab_presenter_mock()
        view = create_mock_settings_diagnostic_tab()
        presenter = SettingsDiagnosticPresenter(parent_presenter)
        presenter.set_view(view)
        self.assertTrue(view.set_tree.call_count == 1)
        presenter.on_row_changed()
        self.assertTrue(view.set_tree.call_count == 2)

    def test_that_updates_rows_when_triggered(self):
        parent_presenter = create_run_tab_presenter_mock()
        view = create_mock_settings_diagnostic_tab()
        presenter = SettingsDiagnosticPresenter(parent_presenter)
        presenter.set_view(view)
        self.assertTrue(view.update_rows.call_count == 1)
        presenter.on_update_rows()
        self.assertTrue(view.update_rows.call_count == 2)

    def test_that_can_save_out_state(self):
        # Arrange
        parent_presenter = create_run_tab_presenter_mock()
        view = create_mock_settings_diagnostic_tab()
        dummy_file_path = os.path.join(tempfile.gettempdir(), "sans_settings_diag_test.json")
        print(dummy_file_path)
        view.get_save_location = mock.MagicMock(return_value=dummy_file_path)
        presenter = SettingsDiagnosticPresenter(parent_presenter)
        presenter.set_view(view)

        # Act
        presenter.on_save_state()

        # Assert
        self.assertTrue(os.path.exists(dummy_file_path))

        with open(dummy_file_path) as f:
            data = json.load(f)
        self.assertTrue(data == "dummy_state")

        if os.path.exists(dummy_file_path):
            os.remove(dummy_file_path)


if __name__ == '__main__':
    unittest.main()
