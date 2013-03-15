from PyQt4 import QtGui, uic, QtCore
from functools import partial
from reduction_gui.widgets.base_widget import BaseWidget
from reduction_gui.reduction.diffraction.diffraction_sample_data_setup_script import SampleSetupScript
import reduction_gui.widgets.util as util
import ui.diffraction.ui_diffraction_sample_setup

class SampleSetupWidget(BaseWidget):
    """
        Widget that presents sample setup options to the user.
    """ 
    ## Widget name (Name of the tab)
    name = "Sample Run Setup"
    
    def __init__(self, parent=None, state=None, settings=None, data_type=None):
        super(SampleSetupWidget, self).__init__(parent, state, settings, data_type=data_type)
        
        #class SamSetFrame(QtGui.QFrame, ui.inelastic.ui_dgs_sample_setup.Ui_Frame): 
        class SamSetFrame(QtGui.QFrame, ui.diffraction.ui_diffraction_sample_setup.Ui_Frame): 
            def __init__(self, parent=None):
                QtGui.QFrame.__init__(self, parent)
                self.setupUi(self)
                
        self._content = SamSetFrame(self)
        self._layout.addWidget(self._content)
        self._instrument_name = settings.instrument_name
        self._facility_name = settings.facility_name
        self.initialize_content()
        
        if state is not None:
            self.set_state(state)
        else:
            self.set_state(SampleSetupScript(self._instrument_name))

        """ Debug Output """
        dbmsg = "Instrument Name: %s   @  Facility: %s\n" % (self._instrument_name, self._facility_name)
        if state is not None:
            dbmsg += "State is %s" % (state)
        else:
            dbmsg += "State is None"
        print "[DBx857 Sample-Setup] %s" % (dbmsg)

        return
        
    def initialize_content(self):
        # Constraints
        dv = QtGui.QDoubleValidator(self._content.ei_guess_edit)
        dv.setBottom(0.0)
        self._content.ei_guess_edit.setValidator(dv)
        if "SNS" != self._facility_name:
            util.set_valid(self._content.ei_guess_edit, False)
        self._content.tzero_guess_edit.setValidator(QtGui.QDoubleValidator(self._content.tzero_guess_edit))
        self._content.etr_low_edit.setValidator(QtGui.QDoubleValidator(self._content.etr_low_edit))
        self._content.etr_width_edit.setValidator(QtGui.QDoubleValidator(self._content.etr_width_edit))
        self._content.etr_high_edit.setValidator(QtGui.QDoubleValidator(self._content.etr_high_edit))
        self._content.monitor1_specid_edit.setValidator(QtGui.QIntValidator(self._content.monitor1_specid_edit))
        self._content.monitor2_specid_edit.setValidator(QtGui.QIntValidator(self._content.monitor2_specid_edit))
        
        # Default states
        self._handle_tzero_guess(self._content.use_ei_guess_chkbox.isChecked())
        
        # Connections
        self.connect(self._content.sample_browse, QtCore.SIGNAL("clicked()"), 
                     self._sample_browse)
        self.connect(self._content.detcal_browse, QtCore.SIGNAL("clicked()"), 
                     self._detcal_browse)
        self.connect(self._content.hardmask_browse, QtCore.SIGNAL("clicked()"), 
                     self._hardmask_browse)
        self.connect(self._content.grouping_browse, QtCore.SIGNAL("clicked()"), 
                     self._grouping_browse)
        self.connect(self._content.use_ei_guess_chkbox, QtCore.SIGNAL("stateChanged(int)"),
                     self._handle_tzero_guess)
        
        # Validated widgets
        self._connect_validated_lineedit(self._content.sample_edit)
        self._connect_validated_lineedit(self._content.ei_guess_edit)

        return

    def _handle_tzero_guess(self, is_enabled):
        self._content.tzero_guess_label.setEnabled(is_enabled)
        self._content.tzero_guess_edit.setEnabled(is_enabled)
        self._content.tzero_guess_unit_label.setEnabled(is_enabled)

    def _check_and_set_lineedit_content(self, lineedit, content):
        lineedit.setText(content)
        util.set_valid(lineedit, not lineedit.text().isEmpty())
        
    def _connect_validated_lineedit(self, ui_ctrl):
        call_back = partial(self._validate_edit, ctrl=ui_ctrl)
        self.connect(ui_ctrl, QtCore.SIGNAL("editingFinished()"), call_back)
        self.connect(ui_ctrl, QtCore.SIGNAL("textEdited(QString)"), call_back)
        self.connect(ui_ctrl, QtCore.SIGNAL("textChanged(QString)"), call_back)

    def _validate_edit(self, ctrl=None):
        is_valid = True
        if ctrl.text().isEmpty():
            is_valid = False
        util.set_valid(ctrl, is_valid)

    def _sample_browse(self):
        fname = self.data_browse_dialog()
        if fname:
            self._content.sample_edit.setText(fname)   

    def _detcal_browse(self):
        fname = self.data_browse_dialog()
        if fname:
            self._content.sample_edit.setText(fname)   

    def _hardmask_browse(self):
        fname = self.data_browse_dialog()
        if fname:
            self._content.hardmask_edit.setText(fname)   

    def _grouping_browse(self):
        fname = self.data_browse_dialog()
        if fname:
            self._content.grouping_edit.setText(fname)   
            
    def set_state(self, state):
        """
            Populate the UI elements with the data from the given state.
            @param state: SampleSetupScript object
        """
        self._check_and_set_lineedit_content(self._content.sample_edit,
                                             state.sample_file)
        self._content.output_ws_edit.setText(state.output_wsname)
        self._content.detcal_edit.setText(state.detcal_file)
        if "SNS" != self._facility_name:
            self._check_and_set_lineedit_content(self._content.ei_guess_edit, 
                                                 state.incident_energy_guess)
        self._content.use_ei_guess_chkbox.setChecked(state.use_ei_guess)
        self._content.tzero_guess_edit.setText(QtCore.QString(str(state.tzero_guess)))
        self._content.monitor1_specid_edit.setText(QtCore.QString(str(state.monitor1_specid)))
        self._content.monitor2_specid_edit.setText(QtCore.QString(str(state.monitor2_specid)))
        self._content.et_range_box.setChecked(state.rebin_et)
        self._content.etr_low_edit.setText(state.et_range_low)
        self._content.etr_width_edit.setText(state.et_range_width)
        self._content.etr_high_edit.setText(state.et_range_high)
        self._content.et_is_distribution_cb.setChecked(state.et_is_distribution)
        self._content.hardmask_edit.setText(state.hardmask_file)
        self._content.grouping_edit.setText(state.grouping_file)
        self._content.show_workspaces_cb.setChecked(state.show_workspaces)
    
    def get_state(self):
        """
            Returns an object with the state of the interface
        """
        s = SampleSetupScript(self._instrument_name)
        s.sample_file = self._content.sample_edit.text()
        s.output_wsname = self._content.output_ws_edit.text()
        s.detcal_file = self._content.detcal_edit.text()
        s.incident_energy_guess = self._content.ei_guess_edit.text()
        s.use_ei_guess = self._content.use_ei_guess_chkbox.isChecked()
        s.tzero_guess = util._check_and_get_float_line_edit(self._content.tzero_guess_edit)
        # s.monitor1_specid = int(self._content.monitor1_specid_edit.text())
        # s.monitor2_specid = int(self._content.monitor2_specid_edit.text())
        s.rebin_et = self._content.et_range_box.isChecked()
        s.et_range_low = self._content.etr_low_edit.text()
        s.et_range_width = self._content.etr_width_edit.text()
        s.et_range_high = self._content.etr_high_edit.text()
        s.et_is_distribution = self._content.et_is_distribution_cb.isChecked()
        s.hardmask_file = self._content.hardmask_edit.text()
        s.grouping_file = self._content.grouping_edit.text()   
        s.show_workspaces = self._content.show_workspaces_cb.isChecked() 
        return s
