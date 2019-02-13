# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2019 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +
#  This file is part of the mantidqt package
#
#
from __future__ import absolute_import, unicode_literals

from qtpy.QtCore import Qt
from qtpy.QtWidgets import QMessageBox

from mantidqt.utils.qt import load_ui

form, base = load_ui(__file__, "ui/main.ui")
plots_form, plots_base = load_ui(__file__, "ui/section_plots.ui")


class PlotsSettingsView(plots_base, plots_form):
    def __init__(self, parent=None):
        super(PlotsSettingsView, self).__init__(parent)
        self.setupUi(self)
        self.setVisible(False)


class SettingsView(base, form):
    def __init__(self, parent, presenter):
        super(SettingsView, self).__init__(parent)
        self.setupUi(self)
        self.setAttribute(Qt.WA_DeleteOnClose, True)

        self.presenter = presenter

        self.sections.currentRowChanged.connect(self.presenter.action_current_row_changed)



    def closeEvent(self, event):
        self.deleteLater()
        super(SettingsView, self).closeEvent(event)

    def ask_before_close(self):
        reply = QMessageBox.question(self, self.presenter.ASK_BEFORE_CLOSE_TITLE,
                                     self.presenter.ASK_BEFORE_CLOSE_MESSAGE, QMessageBox.Yes, QMessageBox.No)
        return True if reply == QMessageBox.Yes else False
