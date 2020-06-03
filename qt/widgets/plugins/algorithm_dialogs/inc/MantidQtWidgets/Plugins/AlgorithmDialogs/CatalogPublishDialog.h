// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2013 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
#pragma once

#include "MantidQtWidgets/Common/AlgorithmDialog.h"
#include "ui_CatalogPublishDialog.h"

namespace MantidQt {
namespace CustomDialogs {
/**
 This class gives specialised dialog for the CatalogPublish algorithm.

 @author Jay Rainey, ISIS Rutherford Appleton Laboratory
 @date 14/01/2014
*/
class CatalogPublishDialog : public API::AlgorithmDialog {
  Q_OBJECT

public:
  /// Constructor
  CatalogPublishDialog(QWidget *parent = nullptr);

private:
  /// Create the inital layout.
  void initLayout() override;
  /// Populate the investigation number combo-box with investigations that the
  /// user can publish to.
  void populateUserInvestigations();

private slots:
  /// When the "browse" button is clicked open a file browser.
  void workspaceSelected(const QString &wsName);
  /// Set the "FileName" property when a file is selected from the file browser.
  void fileSelected();
  /// Diables fields on dialog to improve usability
  void disableDialog();
  /// Set session property when user selects an investigation to publish to.
  void setSessionProperty(int index);

protected:
  /// Overridden to enable dataselector validators
  void accept() override;

protected:
  /// The form generated by QT Designer.
  Ui::CatalogPublishDialog m_uiForm;
};
} // namespace CustomDialogs
} // namespace MantidQt
