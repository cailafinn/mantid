// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
#pragma once
#include "../Runs/IRunsPresenter.h"
#include "Common/Clipboard.h"
#include "Common/DllConfig.h"
#include "GUI/Common/IPlotter.h"
#include "IRunsTablePresenter.h"
#include "IRunsTableView.h"
#include "JobsViewUpdater.h"
#include "MantidQtWidgets/Common/Batch/IJobTreeView.h"
#include "Reduction/RunsTable.h"
#include <memory>

namespace MantidQt {
namespace CustomInterfaces {
namespace ISISReflectometry {

namespace Colour {
constexpr const char *DEFAULT = "#ffffff";          // white
constexpr const char *INVALID = "#dddddd";          // very pale grey
constexpr const char *RUNNING = "#f0e442";          // pale yellow
constexpr const char *SUCCESS = "#d0f4d0";          // pale green
constexpr const char *WARNING = "#e69f00";          // pale orange
constexpr const char *FAILURE = "#accbff";          // pale blue
constexpr const char *CHILDREN_SUCCESS = "#e8f4e8"; // very pale green
} // namespace Colour

class MANTIDQT_ISISREFLECTOMETRY_DLL RunsTablePresenter : public IRunsTablePresenter, public RunsTableViewSubscriber {
public:
  RunsTablePresenter(IRunsTableView *view, std::vector<std::string> const &instruments, double thetaTolerance,
                     ReductionJobs reductionJobs, const IPlotter &plotter);

  void notifyRemoveAllRowsAndGroupsRequested() override;

  // IRunsTablePresenter overrides
  void acceptMainPresenter(IRunsPresenter *mainPresenter) override;
  RunsTable const &runsTable() const override;
  RunsTable &mutableRunsTable() override;
  void mergeAdditionalJobs(ReductionJobs const &jobs) override;
  void notifyInstrumentChanged(std::string const &instrumentName) override;
  void notifyBatchLoaded() override;
  void setTablePrecision(int &precision) override;
  void resetTablePrecision() override;
  void settingsChanged() override;

  // RunsTableViewSubscriber overrides
  void notifyResumeReductionRequested() override;
  void notifyPauseReductionRequested() override;
  void notifyInsertRowRequested() override;
  void notifyInsertGroupRequested() override;
  void notifyDeleteRowRequested() override;
  void notifyDeleteGroupRequested() override;
  void notifyFilterChanged(std::string const &filterValue) override;
  void notifyChangeInstrumentRequested() override;
  void notifyExpandAllRequested() override;
  void notifyCollapseAllRequested() override;
  void notifyPlotSelectedPressed() override;
  void notifyPlotSelectedStitchedOutputPressed() override;
  void notifyFillDown() override;
  void notifyReductionPaused() override;
  void notifyReductionResumed() override;
  void notifyAutoreductionPaused() override;
  void notifyAutoreductionResumed() override;
  void notifyAnyBatchReductionPaused() override;
  void notifyAnyBatchReductionResumed() override;
  void notifyAnyBatchAutoreductionPaused() override;
  void notifyAnyBatchAutoreductionResumed() override;

  // JobTreeViewSubscriber overrides
  void notifyCellTextChanged(MantidQt::MantidWidgets::Batch::RowLocation const &itemIndex, int column,
                             std::string const &oldValue, std::string const &newValue) override;
  void notifySelectionChanged() override;
  void notifyRowInserted(MantidQt::MantidWidgets::Batch::RowLocation const &newRowLocation) override;
  void notifyAppendAndEditAtChildRowRequested() override;
  void notifyAppendAndEditAtRowBelowRequested() override;
  void notifyEditAtRowAboveRequested() override;
  void notifyRemoveRowsRequested(
      std::vector<MantidQt::MantidWidgets::Batch::RowLocation> const &locationsOfRowsToRemove) override;
  void notifyCutRowsRequested() override;
  void notifyCopyRowsRequested() override;
  void notifyPasteRowsRequested() override;
  void notifyFilterReset() override;
  void notifyRowStateChanged() override;
  void notifyRowStateChanged(std::optional<std::reference_wrapper<Item const>> item) override;
  void notifyRowModelChanged() override;
  void notifyRowModelChanged(std::optional<std::reference_wrapper<Item const>> item) override;

private:
  void applyGroupStylingToRow(MantidWidgets::Batch::RowLocation const &location);
  void applyStylingToParent(Row const &row);
  void clearInvalidCellStyling(std::vector<MantidQt::MantidWidgets::Batch::Cell> &cells);
  void clearInvalidCellStyling(MantidQt::MantidWidgets::Batch::Cell &cell);
  void applyInvalidCellStyling(MantidQt::MantidWidgets::Batch::Cell &cell);

  void removeGroupsFromView(std::vector<int> const &groupIndicesOrderedLowToHigh);
  void removeGroupsFromModel(std::vector<int> const &groupIndicesOrderedLowToHigh);
  void removeRowsFromModel(std::vector<MantidQt::MantidWidgets::Batch::RowLocation> rows);
  void showAllCellsOnRowAsValid(MantidWidgets::Batch::RowLocation const &itemIndex);

  void removeRowsAndGroupsFromView(std::vector<MantidQt::MantidWidgets::Batch::RowLocation> const &locations);
  void removeAllRowsAndGroupsFromView();
  void removeRowsAndGroupsFromModel(std::vector<MantidQt::MantidWidgets::Batch::RowLocation> locations);
  void removeAllRowsAndGroupsFromModel();

  void appendRowsToGroupsInView(std::vector<int> const &groupIndices);
  void appendRowsToGroupsInModel(std::vector<int> const &groupIndices);
  void appendEmptyGroupInModel();
  void appendEmptyGroupInView();
  void insertEmptyGroupInModel(int beforeGroup);
  void insertEmptyGroupInView(int beforeGroup);
  void appendRowAndGroup();
  void ensureAtLeastOneGroupExists();
  void insertEmptyRowInModel(int groupIndex, int beforeRow);
  std::vector<std::string> cellTextFromViewAt(MantidWidgets::Batch::RowLocation const &location) const;
  void showCellsAsInvalidInView(MantidWidgets::Batch::RowLocation const &itemIndex,
                                std::vector<int> const &invalidColumns);
  void updateGroupName(MantidQt::MantidWidgets::Batch::RowLocation const &itemIndex, int column,
                       std::string const &oldValue, std::string const &newValue);
  void updateRowField(MantidQt::MantidWidgets::Batch::RowLocation const &itemIndex, int column,
                      std::string const &oldValue, std::string const &newValue);
  void updateWidgetEnabledState();

  void pasteRowsOntoRows(std::vector<MantidWidgets::Batch::RowLocation> &replacementRoots);
  void pasteRowsOntoGroup(std::vector<MantidWidgets::Batch::RowLocation> &replacementRoots);
  void pasteGroupsOntoGroups(std::vector<MantidWidgets::Batch::RowLocation> &replacementRoots);
  void pasteGroupsAtEnd();

  using UpdateCellFunc = void (*)(MantidWidgets::Batch::Cell &cell);
  using UpdateCellWithTooltipFunc = void (*)(MantidWidgets::Batch::Cell &cell, std::string const &tooltip);

  void forAllCellsAt(MantidWidgets::Batch::RowLocation const &location, UpdateCellFunc updateFunc);
  void forAllCellsAt(MantidWidgets::Batch::RowLocation const &location, UpdateCellWithTooltipFunc updateFunc,
                     std::string const &tooltip);
  void setRowStylingForItem(MantidWidgets::Batch::RowLocation const &rowLocation, Item const &item);
  void updateProgressBar();

  void notifyTableChanged();

  bool isProcessing() const;
  bool isAutoreducing() const;
  bool isAnyBatchProcessing() const;
  bool isAnyBatchAutoreducing() const;

  static auto constexpr DEPTH_LIMIT = 2;

  IRunsTableView *m_view;
  RunsTable m_model;
  Clipboard m_clipboard;
  JobsViewUpdater m_jobViewUpdater;
  IRunsPresenter *m_mainPresenter;
  const IPlotter &m_plotter;

  friend class Encoder;
  friend class Decoder;
  friend class CoderCommonTester;
};
} // namespace ISISReflectometry
} // namespace CustomInterfaces
} // namespace MantidQt
