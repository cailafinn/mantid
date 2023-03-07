# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2020 ISIS Rutherford Appleton Laboratory UKRI,
#   NScD Oak Ridge National Laboratory, European Spallation Source,
#   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
# SPDX - License - Identifier: GPL - 3.0 +
import unittest
from unittest import mock
from unittest.mock import patch
from numpy import isnan

from mantidqtinterfaces.Engineering.gui.engineering_diffraction.tabs.common.data_handling.data_model import FittingDataModel
from testhelpers import assertRaisesNothing

data_model_path = "mantidqtinterfaces.Engineering.gui.engineering_diffraction.tabs.common.data_handling.data_model"


class TestFittingDataModel(unittest.TestCase):
    def setUp(self):
        self.model = FittingDataModel()
        # setup a mock workspace
        self.mock_inst = mock.MagicMock()
        self.mock_inst.getFullName.return_value = "instrument"
        mock_prop = mock.MagicMock()
        mock_prop.value = "bank 1"  # bank-id
        mock_log_data = [mock.MagicMock(), mock.MagicMock()]
        mock_log_data[0].name = "LogName"
        mock_log_data[1].name = "proton_charge"
        self.mock_run = mock.MagicMock()
        self.mock_run.getProtonCharge.return_value = 1.0
        self.mock_run.getProperty.return_value = mock_prop
        self.mock_run.getLogData.return_value = mock_log_data
        self.mock_ws = mock.MagicMock()
        self.mock_ws.getNumberHistograms.return_value = 1
        self.mock_ws.getRun.return_value = self.mock_run
        self.mock_ws.getInstrument.return_value = self.mock_inst
        self.mock_ws.getRunNumber.return_value = 1
        self.mock_ws.getTitle.return_value = "title"
        mock_axis = mock.MagicMock()
        mock_unit = mock.MagicMock()
        self.mock_ws.getAxis.return_value = mock_axis
        mock_axis.getUnit.return_value = mock_unit
        mock_unit.caption.return_value = "Time-of-flight"

    @patch(data_model_path + ".FittingDataModel.update_log_workspace_group")
    @patch(data_model_path + ".Load")
    def test_loading_single_file_stores_workspace(self, mock_load, mock_update_logws_group):
        mock_load.return_value = self.mock_ws

        self.model.load_files("/ar/a_filename.whatever")

        self.assertEqual(1, len(self.model._data_workspaces))
        self.assertEqual(self.mock_ws, self.model._data_workspaces["a_filename"].loaded_ws)
        mock_load.assert_called_with("/ar/a_filename.whatever", OutputWorkspace="a_filename")
        mock_update_logws_group.assert_called_once()

    @patch(data_model_path + ".FittingDataModel.update_log_workspace_group")
    @patch(data_model_path + ".ADS")
    @patch(data_model_path + ".Load")
    def test_loading_single_file_already_loaded_untracked(self, mock_load, mock_ads, mock_update_logws_group):
        mock_ads.doesExist.return_value = True
        mock_ads.retrieve.return_value = self.mock_ws

        self.model.load_files("/ar/a_filename.whatever")

        self.assertEqual(1, len(self.model._data_workspaces))
        mock_load.assert_not_called()
        mock_update_logws_group.assert_called_once()

    @patch(data_model_path + ".FittingDataModel.update_log_workspace_group")
    @patch(data_model_path + ".Load")
    def test_loading_single_file_already_loaded_tracked(self, mock_load, mock_update_logws_group):
        fpath = "/ar/a_filename.whatever"
        self.model._data_workspaces.add(self.model._generate_workspace_name(fpath), loaded_ws=self.mock_ws)

        self.model.load_files(fpath)

        self.assertEqual(1, len(self.model._data_workspaces))
        mock_load.assert_not_called()
        mock_update_logws_group.assert_called()

    @patch(data_model_path + ".get_setting")
    @patch(data_model_path + ".AverageLogData")
    @patch(data_model_path + ".Load")
    def test_loading_single_file_with_logs(self, mock_load, mock_avglogs, mock_getsetting):
        mock_load.return_value = self.mock_ws
        log_names = ["to", "test"]
        mock_getsetting.return_value = ",".join(log_names)
        mock_avglogs.return_value = (1.0, 1.0)  # avg, stdev

        self.model.load_files("/ar/a_filename.whatever")

        self.assertEqual(1, len(self.model._data_workspaces))
        self.assertEqual(self.mock_ws, self.model._data_workspaces["a_filename"].loaded_ws)
        mock_load.assert_called_with("/ar/a_filename.whatever", OutputWorkspace="a_filename")
        self.assertEqual(1 + len(log_names), len(self.model._log_workspaces))
        for ilog in range(0, len(log_names)):
            self.assertEqual(log_names[ilog], self.model._log_workspaces[ilog + 1].name())
            self.assertEqual(1, self.model._log_workspaces[ilog + 1].rowCount())

    @patch(data_model_path + ".logger")
    @patch(data_model_path + ".Load")
    def test_loading_single_file_invalid(self, mock_load, mock_logger):
        mock_load.side_effect = RuntimeError("Invalid Path")

        self.model.load_files("/ar/a_filename.whatever")
        self.assertEqual(0, len(self.model._data_workspaces))
        mock_load.assert_called_with("/ar/a_filename.whatever", OutputWorkspace="a_filename")
        self.assertEqual(1, mock_logger.error.call_count)

    @patch(data_model_path + ".FittingDataModel.update_log_workspace_group")
    @patch(data_model_path + ".Load")
    def test_loading_multiple_files(self, mock_load, mock_update_logws_group):
        mock_load.return_value = self.mock_ws

        self.model.load_files("/dir/file1.txt, /dir/file2.nxs")

        self.assertEqual(2, len(self.model._data_workspaces))
        self.assertEqual(self.mock_ws, self.model._data_workspaces["file1"].loaded_ws)
        self.assertEqual(self.mock_ws, self.model._data_workspaces["file2"].loaded_ws)
        mock_load.assert_any_call("/dir/file1.txt", OutputWorkspace="file1")
        mock_load.assert_any_call("/dir/file2.nxs", OutputWorkspace="file2")
        mock_update_logws_group.assert_called_once()

    @patch(data_model_path + ".logger")
    @patch(data_model_path + ".Load")
    def test_loading_multiple_files_too_many_spectra(self, mock_load, mock_logger):
        self.mock_ws.getNumberHistograms.return_value = 2
        mock_load.return_value = self.mock_ws

        self.model.load_files("/dir/file1.txt, /dir/file2.nxs")

        self.assertEqual(0, len(self.model._data_workspaces))
        mock_load.assert_any_call("/dir/file1.txt", OutputWorkspace="file1")
        mock_load.assert_any_call("/dir/file2.nxs", OutputWorkspace="file2")
        self.assertEqual(2, mock_logger.warning.call_count)

    @patch(data_model_path + ".logger")
    @patch(data_model_path + ".Load")
    def test_loading_multiple_files_invalid(self, mock_load, mock_logger):
        mock_load.side_effect = RuntimeError("Invalid Path")

        self.model.load_files("/dir/file1.txt, /dir/file2.nxs")

        self.assertEqual(0, len(self.model._data_workspaces))
        mock_load.assert_any_call("/dir/file1.txt", OutputWorkspace="file1")
        mock_load.assert_any_call("/dir/file2.nxs", OutputWorkspace="file2")
        self.assertEqual(2, mock_logger.error.call_count)

    @patch(data_model_path + ".DeleteWorkspace")
    @patch(data_model_path + ".EnggEstimateFocussedBackground")
    @patch(data_model_path + ".Minus")
    def test_do_background_subtraction_first_time(self, mock_minus, mock_estimate_bg, mock_delete_ws):
        self.model._data_workspaces.add("name1", loaded_ws=self.mock_ws)
        mock_estimate_bg.return_value = self.mock_ws

        bg_params = [True, 40, 800, False]
        self.model.create_or_update_bgsub_ws("name1", bg_params)

        self.assertEqual(self.model._data_workspaces["name1"].bg_params, bg_params)
        mock_minus.assert_called_once()
        mock_estimate_bg.assert_called_once()
        mock_delete_ws.assert_called_once()

    @patch(data_model_path + ".DeleteWorkspace")
    @patch(data_model_path + ".EnggEstimateFocussedBackground")
    @patch(data_model_path + ".Minus")
    def test_do_background_subtraction_bgparams_changed(self, mock_minus, mock_estimate_bg, mock_delete_ws):
        self.model._data_workspaces.add("name1", loaded_ws=self.mock_ws, bgsub_ws=self.mock_ws, bg_params=[True, 80, 1000, False])
        mock_estimate_bg.return_value = self.mock_ws

        bg_params = [True, 40, 800, False]
        self.model.create_or_update_bgsub_ws("name1", bg_params)

        self.assertEqual(self.model._data_workspaces["name1"].bg_params, bg_params)
        mock_minus.assert_called_once()
        mock_estimate_bg.assert_called_once()
        mock_delete_ws.assert_called_once()

    @patch(data_model_path + ".EnggEstimateFocussedBackground")
    @patch(data_model_path + ".Minus")
    def test_do_background_subtraction_no_change(self, mock_minus, mock_estimate_bg):
        bg_params = [True, 80, 1000, False]
        self.model._data_workspaces.add("name1", loaded_ws=self.mock_ws, bgsub_ws=self.mock_ws, bg_params=bg_params)
        mock_estimate_bg.return_value = self.mock_ws

        self.model.create_or_update_bgsub_ws("name1", bg_params)

        self.assertEqual(self.model._data_workspaces["name1"].bg_params, bg_params)
        mock_minus.assert_not_called()
        mock_estimate_bg.assert_not_called()

    @patch(data_model_path + ".SetUncertainties")
    @patch(data_model_path + ".DeleteWorkspace")
    @patch(data_model_path + ".EnggEstimateFocussedBackground")
    @patch(data_model_path + ".Minus")
    def test_invalid_bg_inputs_dont_throw(self, mock_minus, mock_estimate_bg, mock_delete_ws, mock_set_uncertainties):
        self.model._data_workspaces.add("name1", loaded_ws=self.mock_ws)
        mock_estimate_bg.side_effect = ValueError("Some problem")

        bg_params = [True, -1, 800, False]
        assertRaisesNothing(self, self.model.create_or_update_bgsub_ws, "name1", bg_params)

    @patch(data_model_path + ".RenameWorkspace")
    @patch(data_model_path + ".ADS")
    @patch(data_model_path + ".DeleteTableRows")
    def test_remove_run_from_log_table(self, mock_delrows, mock_ads, mock_rename):
        mock_table = mock.MagicMock()
        mock_table.column.return_value = [1, 2]
        mock_table.rowCount.return_value = len(mock_table.column())  # two left post-removal
        mock_table.row.return_value = {"Instrument": "test_inst"}
        mock_ads.retrieve.return_value = mock_table
        self.model._log_workspaces = mock.MagicMock()
        self.model._log_workspaces.name.return_value = "test"

        self.model.remove_log_rows([0])

        mock_delrows.assert_called_once()
        new_name = f"{mock_table.row()['Instrument']}_{min(mock_table.column())}-{max(mock_table.column())}_logs"
        mock_rename.assert_called_with(InputWorkspace=self.model._log_workspaces.name(), OutputWorkspace=new_name)

    @patch(data_model_path + ".DeleteWorkspace")
    @patch(data_model_path + ".ADS")
    @patch(data_model_path + ".DeleteTableRows")
    def test_remove_last_run_from_log_table(self, mock_delrows, mock_ads, mock_delws):
        mock_table = mock.MagicMock()
        mock_table.rowCount.return_value = 0  # none left post-removal
        mock_ads.retrieve.return_value = mock_table
        self.model._log_workspaces = mock.MagicMock()
        name = "test"
        self.model._log_workspaces.name.return_value = name

        self.model.remove_log_rows([0])

        mock_delrows.assert_called_once()
        mock_delws.assert_called_with(name)
        self.assertEqual(None, self.model._log_workspaces)

    @patch(data_model_path + ".FittingDataModel.update_log_workspace_group")
    def test_update_workspace_name(self, mock_update_log_ws_group):
        self.model._data_workspaces.add("name1", loaded_ws=self.mock_ws, bgsub_ws=self.mock_ws, bg_params=[True, 80, 1000, False])
        self.model._data_workspaces.add("name2", loaded_ws=self.mock_ws, bgsub_ws=self.mock_ws)
        self.model._log_values = {"name1": {"log_name1": 1}, "name2": {"log_name1": 2}}

        self.model.update_workspace_name("name1", "new_name")

        self.assertEqual({"new_name": self.mock_ws, "name2": self.mock_ws}, self.model._data_workspaces.get_loaded_ws_dict())
        self.assertEqual({"new_name": self.mock_ws, "name2": self.mock_ws}, self.model._data_workspaces.get_bgsub_ws_dict())
        self.assertEqual({"name2": [], "new_name": [True, 80, 1000, False]}, self.model._data_workspaces.get_bg_params_dict())
        self.assertEqual({"new_name": {"log_name1": 1}, "name2": {"log_name1": 2}}, self.model._log_values)

    @patch(data_model_path + ".FittingDataModel.remove_all_log_rows")
    @patch(data_model_path + ".FittingDataModel.create_log_workspace_group")
    @patch(data_model_path + ".FittingDataModel.add_log_to_table")
    def test_update_logs_initial(self, mock_add_log, mock_create_log_group, mock_remove_all_log_rows):
        self.model._data_workspaces.add("name1", loaded_ws=self.mock_ws)
        self.model._log_workspaces = None

        self.model.update_log_workspace_group()
        mock_create_log_group.assert_called_once()
        mock_add_log.assert_called_with("name1", self.mock_ws, 0)

    @patch(data_model_path + ".FittingDataModel.make_log_table")
    @patch(data_model_path + ".FittingDataModel.make_runinfo_table")
    @patch(data_model_path + ".FittingDataModel.create_log_workspace_group")
    @patch(data_model_path + ".FittingDataModel.add_log_to_table")
    @patch(data_model_path + ".ADS")
    def test_update_logs_deleted(self, mock_ads, mock_add_log, mock_create_log_group, mock_make_runinfo, mock_make_log_table):
        self.model._data_workspaces.add("name1", loaded_ws=self.mock_ws)
        self.model._log_workspaces = mock.MagicMock()
        self.model._log_names = ["LogName1", "LogName2"]
        # simulate LogName2 and run_info tables being deleted
        mock_ads.doesExist = lambda ws_name: ws_name == "LogName1"

        self.model.update_log_workspace_group()
        mock_create_log_group.assert_not_called()
        mock_make_runinfo.assert_called_once()
        mock_make_log_table.assert_called_once_with("LogName2")
        self.model._log_workspaces.add.assert_any_call("run_info")
        self.model._log_workspaces.add.assert_any_call("LogName2")
        mock_add_log.assert_called_with("name1", self.mock_ws, 0)

    @patch(data_model_path + ".FittingDataModel.make_log_table")
    @patch(data_model_path + ".FittingDataModel.make_runinfo_table")
    @patch(data_model_path + ".get_setting")
    @patch(data_model_path + ".GroupWorkspaces")
    def test_create_log_workspace_group(self, mock_group, mock_get_setting, mock_make_runinfo, mock_make_log_table):
        log_names = ["LogName1", "LogName2"]
        mock_get_setting.return_value = ",".join(log_names)
        mock_group_ws = mock.MagicMock()
        mock_group.return_value = mock_group_ws

        self.model.create_log_workspace_group()

        mock_group.assert_called_once()
        for log in log_names:
            mock_group_ws.add.assert_any_call(log)
        self.assertEqual(self.model._log_names, log_names)
        mock_make_runinfo.assert_called_once()
        self.assertEqual(mock_make_log_table.call_count, len(log_names))

    @patch(data_model_path + ".write_table_row")
    @patch(data_model_path + ".ADS")
    @patch(data_model_path + ".FittingDataModel.update_log_group_name")
    @patch(data_model_path + ".AverageLogData")
    def test_add_log_to_table_already_averaged(self, mock_avglogs, mock_update_logname, mock_ads, mock_writerow):
        self._setup_model_log_workspaces()
        mock_ads.retrieve = lambda ws_name: [ws for ws in self.model._log_workspaces if ws.name() == ws_name][0]
        self.model._log_values = {"name1": {"LogName": [2, 1]}}
        self.model._log_names = ["LogName"]

        self.model.add_log_to_table("name1", self.mock_ws, 3)
        mock_writerow.assert_any_call(self.model._log_workspaces[0], ["instrument", 1, "bank 1", 1.0, "title"], 3)
        mock_writerow.assert_any_call(self.model._log_workspaces[1], [2, 1], 3)
        mock_avglogs.assert_not_called()
        mock_update_logname.assert_called_once()

    @patch(data_model_path + ".write_table_row")
    @patch(data_model_path + ".ADS")
    @patch(data_model_path + ".FittingDataModel.update_log_group_name")
    @patch(data_model_path + ".AverageLogData")
    def test_add_log_to_table_not_already_averaged(self, mock_avglogs, mock_update_logname, mock_ads, mock_writerow):
        self._setup_model_log_workspaces()
        mock_ads.retrieve = lambda ws_name: [ws for ws in self.model._log_workspaces if ws.name() == ws_name][0]
        self.model._log_values = {"name1": {}}
        self.model._log_names = ["LogName"]
        mock_avglogs.return_value = [1.0, 1.0]

        self.model.add_log_to_table("name1", self.mock_ws, 3)

        self.assertEqual(self.model._log_values["name1"]["LogName"], [1.0, 1.0])
        mock_writerow.assert_any_call(self.model._log_workspaces[1], [1.0, 1.0], 3)
        mock_avglogs.assert_called_with("name1", LogName="LogName", FixZero=False)
        mock_update_logname.assert_called_once()

    @patch(data_model_path + ".write_table_row")
    @patch(data_model_path + ".ADS")
    @patch(data_model_path + ".FittingDataModel.update_log_group_name")
    @patch(data_model_path + ".AverageLogData")
    def test_add_log_to_table_not_existing_in_ws(self, mock_avglogs, mock_update_logname, mock_ads, mock_writerow):
        self._setup_model_log_workspaces()
        mock_ads.retrieve = lambda ws_name: [ws for ws in self.model._log_workspaces if ws.name() == ws_name][0]
        self.model._log_values = {"name1": {}}
        self.model._log_names = ["LogName"]
        self.mock_run.getLogData.return_value = [self.mock_run.getLogData()[1]]  # only proton_charge

        self.model.add_log_to_table("name1", self.mock_ws, 3)

        self.assertTrue(all(isnan(self.model._log_values["name1"]["LogName"])))
        self.assertTrue(len(self.model._log_values["name1"]["LogName"]), 2)
        mock_avglogs.assert_not_called()
        mock_update_logname.assert_called_once()

    @patch(data_model_path + ".get_setting")
    @patch(data_model_path + ".ADS")
    def test_get_ws_sorted_by_primary_log_ascending(self, mock_ads, mock_getsetting):
        self.model._data_workspaces.add("name1", loaded_ws=self.mock_ws)
        self.model._data_workspaces.add("name2", loaded_ws=self.mock_ws)
        self.model._data_workspaces.add("name3", loaded_ws=self.mock_ws)
        mock_getsetting.side_effect = ["log", "true"]  # primary log, sort_ascending
        mock_log_table = mock.MagicMock()
        mock_log_table.column.return_value = [2, 0, 1]  # fake log values
        mock_ads.retrieve.return_value = mock_log_table

        ws_list = self.model.get_active_ws_sorted_by_primary_log()

        self.assertEqual(ws_list, ["name2", "name3", "name1"])

    @patch(data_model_path + ".get_setting")
    @patch(data_model_path + ".ADS")
    def test_get_ws_sorted_by_primary_log_descending(self, mock_ads, mock_getsetting):
        self.model._data_workspaces.add("name1", loaded_ws=self.mock_ws)
        self.model._data_workspaces.add("name2", loaded_ws=self.mock_ws)
        self.model._data_workspaces.add("name3", loaded_ws=self.mock_ws)
        mock_getsetting.side_effect = ["log", "false"]  # primary log, sort_ascending
        mock_log_table = mock.MagicMock()
        mock_log_table.column.return_value = [2, 0, 1]  # fake log values
        mock_ads.retrieve.return_value = mock_log_table

        ws_list = self.model.get_active_ws_sorted_by_primary_log()

        self.assertEqual(ws_list, ["name1", "name3", "name2"])

    @patch(data_model_path + ".get_setting")
    @patch(data_model_path + ".ADS")
    def test_get_ws_sorted_by_primary_log_not_specified(self, mock_ads, mock_getsetting):
        self.model._data_workspaces.add("name1", loaded_ws=self.mock_ws)
        self.model._data_workspaces.add("name2", loaded_ws=self.mock_ws)
        self.model._data_workspaces.add("name3", loaded_ws=self.mock_ws)
        mock_getsetting.side_effect = ["", "false"]  # primary log, sort_ascending

        ws_list = self.model.get_active_ws_sorted_by_primary_log()

        self.assertEqual(ws_list, list(self.model._data_workspaces.get_loaded_workpace_names())[::-1])
        mock_ads.retrieve.assert_not_called()

    def _setup_model_log_workspaces(self):
        # grouped ws acts like a container/list of ws here
        self.model._log_workspaces = [mock.MagicMock(), mock.MagicMock()]
        self.model._log_workspaces[0].name.return_value = "run_info"
        self.model._log_workspaces[1].name.return_value = "LogName"


if __name__ == "__main__":
    unittest.main()
