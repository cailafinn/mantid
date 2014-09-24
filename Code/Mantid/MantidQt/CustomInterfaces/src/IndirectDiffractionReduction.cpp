//----------------------
// Includes
//----------------------
#include "MantidQtCustomInterfaces/IndirectDiffractionReduction.h"

#include "MantidQtAPI/ManageUserDirectories.h"
#include "MantidAPI/AlgorithmManager.h"
#include "MantidKernel/Logger.h"
#include "MantidKernel/MultiFileNameParser.h"

#include <QDesktopServices>
#include <QUrl>

using namespace Mantid::API;
using namespace Mantid::Geometry;

//Add this class to the list of specialised dialogs in this namespace
namespace MantidQt
{
namespace CustomInterfaces
{

namespace // anon
{
  /// static logger
  Mantid::Kernel::Logger g_log("IndirectDiffractionReduction");

  // Helper function for use with std::transform.
  std::string toStdString(const QString & qString)
  {
    return qString.toStdString();
  }
} // anon namespace

DECLARE_SUBWINDOW(IndirectDiffractionReduction);

using namespace Mantid::API;
using namespace MantidQt::CustomInterfaces;

using MantidQt::API::BatchAlgorithmRunner;

//----------------------
// Public member functions
//----------------------
///Constructor
IndirectDiffractionReduction::IndirectDiffractionReduction(QWidget *parent) :
  UserSubWindow(parent), m_valInt(NULL), m_valDbl(NULL),
  m_settingsGroup("CustomInterfaces/DEMON"),
  m_batchAlgoRunner(new BatchAlgorithmRunner(parent))
{
  // Handles completion of the diffraction algorithm chain
  connect(m_batchAlgoRunner, SIGNAL(batchComplete(bool)), this, SLOT(plotResults(bool)));
}

///Destructor
IndirectDiffractionReduction::~IndirectDiffractionReduction()
{
  saveSettings();
}

/**
 * Runs a diffraction reduction when the user clieks Run.
 */
void IndirectDiffractionReduction::demonRun()
{
  QString instName = m_uiForm.cbInst->currentText();
  QString mode = m_uiForm.cbReflection->currentText();

  if(instName == "OSIRIS" && mode == "diffonly")
  {
    if(!m_uiForm.dem_rawFiles->isValid() || !validateVanCal())
    {
      showInformationBox("Invalid input.\nIncorrect entries marked with red star.");
      return;
    }

    runOSIRISdiffonlyReduction();
  }
  else
  {
    if(!m_uiForm.dem_rawFiles->isValid() || !validateRebin())
    {
      showInformationBox("Invalid input.\nIncorrect entries marked with red star.");
      return;
    }

    runGenericReduction(instName, mode);
  }
}

/**
 * Handles plotting result spectra from algorithm chains.
 *
 * @param error True if the chain was stopped due to error
 */
void IndirectDiffractionReduction::plotResults(bool error)
{
  // Nothing can be plotted
  if(error)
    return;

  QString instName = m_uiForm.cbInst->currentText();
  QString mode = m_uiForm.cbReflection->currentText();

  QString pyInput = "from mantidplot import plotSpectrum\n";
  if(m_uiForm.cbPlotType->currentText() == "Spectra")
  {
    for(auto it = m_plotWorkspaces.begin(); it != m_plotWorkspaces.end(); ++it)
      pyInput += "plotSpectrum('" + *it + "', 0)\n";
  }

  runPythonCode(pyInput);
}

/**
 * Runs a diffraction reduction for any instrument in any mode.
 *
 * @param instName Name of the instrument
 * @param mode Mode instrument is operating in (diffspec/diffonly)
 */
void IndirectDiffractionReduction::runGenericReduction(QString instName, QString mode)
{
  // Get rebin string
  QString rebinStart = m_uiForm.leRebinStart->text();
  QString rebinWidth = m_uiForm.leRebinWidth->text();
  QString rebinEnd = m_uiForm.leRebinEnd->text();

  QString rebin = "";
  if(!rebinStart.isEmpty() && !rebinWidth.isEmpty() && !rebinEnd.isEmpty())
      rebin = rebinStart + "," + rebinWidth + "," + rebinEnd;

  // Get detector range
  std::vector<long> detRange;
  detRange.push_back(m_uiForm.set_leSpecMin->text().toLong());
  detRange.push_back(m_uiForm.set_leSpecMax->text().toLong());

  // Get MSGDiffractionReduction algorithm instance
  IAlgorithm_sptr msgDiffReduction = AlgorithmManager::Instance().create("MSGDiffractionReduction");
  msgDiffReduction->initialize();

  std::vector<std::string> saveFormats;
  if(m_uiForm.ckGSS->isChecked())   saveFormats.push_back("gss");
  if(m_uiForm.ckNexus->isChecked()) saveFormats.push_back("nxs");
  if(m_uiForm.ckAscii->isChecked()) saveFormats.push_back("ascii");

  // Set algorithm properties
  msgDiffReduction->setProperty("Instrument", instName.toStdString());
  msgDiffReduction->setProperty("Mode", mode.toStdString());
  msgDiffReduction->setProperty("SumFiles", m_uiForm.dem_ckSumFiles->isChecked());
  msgDiffReduction->setProperty("InputFiles", m_uiForm.dem_rawFiles->getFilenames().join(",").toStdString());
  msgDiffReduction->setProperty("DetectorRange", detRange);
  msgDiffReduction->setProperty("RebinParam", rebin.toStdString());
  msgDiffReduction->setProperty("SaveFormats", saveFormats);
  msgDiffReduction->setProperty("OutputWorkspaceGroup", "IndirectDiffraction_Workspaces");

  m_batchAlgoRunner->addAlgorithm(msgDiffReduction);

  m_plotWorkspaces.clear();
  m_plotWorkspaces.push_back("IndirectDiffraction_Workspaces");

  m_batchAlgoRunner->executeBatchAsync();
}

/**
 * Runs a diffraction reduction for OSIRIS operating in diffonly mode using the OSIRISDiffractionReduction algorithm.
 */
void IndirectDiffractionReduction::runOSIRISdiffonlyReduction()
{
  // Get the files names from MWRunFiles widget, and convert them from Qt forms into stl equivalents.
  QStringList fileNames = m_uiForm.dem_rawFiles->getFilenames();
  std::vector<std::string> stlFileNames;
  stlFileNames.reserve(fileNames.size());
  std::transform(fileNames.begin(),fileNames.end(),std::back_inserter(stlFileNames), toStdString);

  // Use the file names to suggest a workspace name to use.  Report to logger and stop if unable to parse correctly.
  QString drangeWsName;
  QString tofWsName;
  try
  {
    QString nameBase = QString::fromStdString(Mantid::Kernel::MultiFileNameParsing::suggestWorkspaceName(stlFileNames));
    tofWsName = "'" + nameBase + "_tof'";
    drangeWsName = "'" + nameBase + "_dRange'";
  }
  catch(std::runtime_error & re)
  {
    g_log.error(re.what());
    return;
  }

  IAlgorithm_sptr osirisDiffReduction = AlgorithmManager::Instance().create("OSIRISDiffractionReduction");
  osirisDiffReduction->initialize();
  osirisDiffReduction->setProperty("Sample", m_uiForm.dem_rawFiles->getFilenames().join(",").toStdString());
  osirisDiffReduction->setProperty("Vanadium", m_uiForm.dem_vanadiumFile->getFilenames().join(",").toStdString());
  osirisDiffReduction->setProperty("CalFile", m_uiForm.dem_calFile->getFirstFilename().toStdString());
  osirisDiffReduction->setProperty("OutputWorkspace", drangeWsName.toStdString());
  m_batchAlgoRunner->addAlgorithm(osirisDiffReduction);

  BatchAlgorithmRunner::AlgorithmRuntimeProps inputFromReductionProps;
  inputFromReductionProps["InputWorkspace"] = drangeWsName.toStdString();

  IAlgorithm_sptr convertUnits = AlgorithmManager::Instance().create("ConvertUnits");
  convertUnits->initialize();
  convertUnits->setProperty("OutputWorkspace", tofWsName.toStdString());
  convertUnits->setProperty("Target", "TOF");
  m_batchAlgoRunner->addAlgorithm(convertUnits, inputFromReductionProps);

  BatchAlgorithmRunner::AlgorithmRuntimeProps inputFromConvUnitsProps;
  inputFromConvUnitsProps["InputWorkspace"] = tofWsName.toStdString();

  if ( m_uiForm.ckGSS->isChecked() )
  {
    QString gssFilename = tofWsName + ".gss";
    IAlgorithm_sptr saveGSS = AlgorithmManager::Instance().create("SaveGSS");
    saveGSS->initialize();
    saveGSS->setProperty("Filename", gssFilename.toStdString());
    m_batchAlgoRunner->addAlgorithm(saveGSS, inputFromConvUnitsProps);
  }

  if ( m_uiForm.ckNexus->isChecked() )
  {
    QString nexusFilename = drangeWsName + ".nxs";
    IAlgorithm_sptr saveNexus = AlgorithmManager::Instance().create("SaveNexusProcessed");
    saveNexus->initialize();
    saveNexus->setProperty("Filename", nexusFilename.toStdString());
    m_batchAlgoRunner->addAlgorithm(saveNexus, inputFromReductionProps);
  }

  if ( m_uiForm.ckAscii->isChecked() )
  {
    QString asciiFilename = drangeWsName + ".dat";
    IAlgorithm_sptr saveASCII = AlgorithmManager::Instance().create("SaveAscii");
    saveASCII->initialize();
    saveASCII->setProperty("Filename", asciiFilename.toStdString());
    m_batchAlgoRunner->addAlgorithm(saveASCII, inputFromReductionProps);
  }

  m_plotWorkspaces.clear();
  m_plotWorkspaces.push_back(tofWsName);
  m_plotWorkspaces.push_back(drangeWsName);

  m_batchAlgoRunner->executeBatchAsync();
}

/**
 * Loads an empty instrument and returns a pointer to the workspace.
 *
 * Optionally loads an IPF if a reflection was provided.
 *
 * @param instrumentName Name of an inelastic indiretc instrument (IRIS, OSIRIN, TOSCA, VESUVIO)
 * @param reflection Reflection mode to load parameters for (diffspec or diffonly)
 */
MatrixWorkspace_sptr IndirectDiffractionReduction::loadInstrument(std::string instrumentName, std::string reflection)
{
  std::string instWorkspaceName = "__empty_" + instrumentName;
  std::string idfPath = Mantid::Kernel::ConfigService::Instance().getString("instrumentDefinition.directory");

  if(!AnalysisDataService::Instance().doesExist(instWorkspaceName))
  {
    std::string parameterFilename = idfPath + instrumentName + "_Definition.xml";
    IAlgorithm_sptr loadAlg = AlgorithmManager::Instance().create("LoadEmptyInstrument");
    loadAlg->initialize();
    loadAlg->setProperty("Filename", parameterFilename);
    loadAlg->setProperty("OutputWorkspace", instWorkspaceName);
    loadAlg->execute();
  }

  MatrixWorkspace_sptr instWorkspace = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>(instWorkspaceName);

  // Load parameter file if a reflection was given
  if(!reflection.empty())
  {
    std::string ipfFilename = idfPath + instrumentName + "_diffraction_" + reflection + "_Parameters.xml";
    IAlgorithm_sptr loadParamAlg = AlgorithmManager::Instance().create("LoadParameterFile");
    loadParamAlg->initialize();
    loadParamAlg->setProperty("Filename", ipfFilename);
    loadParamAlg->setProperty("Workspace", instWorkspaceName);
    loadParamAlg->execute();
  }

  return instWorkspace;
}

/**
 * Handles loading an instrument and reflections when an instruiment is selected form the drop down.
 */
void IndirectDiffractionReduction::instrumentSelected(int)
{
  // If the interface is not shown, do not go looking for parameter files, etc.
  if(!m_uiForm.cbInst->isVisible())
    return;

  std::string instrumentName = m_uiForm.cbInst->currentText().toStdString();
  MatrixWorkspace_sptr instWorkspace = loadInstrument(instrumentName);
  Instrument_const_sptr instrument = instWorkspace->getInstrument();

  std::vector<std::string> analysers;
  boost::split(analysers, instrument->getStringParameter("refl-diffraction")[0], boost::is_any_of(","));

  m_uiForm.cbReflection->blockSignals(true);
  m_uiForm.cbReflection->clear();

  for(auto it = analysers.begin(); it != analysers.end(); ++it)
  {
    std::string reflection = *it;
    m_uiForm.cbReflection->addItem(QString::fromStdString(reflection));
  }

  reflectionSelected(m_uiForm.cbReflection->currentIndex());
  m_uiForm.cbReflection->blockSignals(false);
}

/**
 * Handles setting default spectra range when a reflection is slected from the drop down.
 */
void IndirectDiffractionReduction::reflectionSelected(int)
{
  std::string instrumentName = m_uiForm.cbInst->currentText().toStdString();
  std::string reflection = m_uiForm.cbReflection->currentText().toStdString();
  MatrixWorkspace_sptr instWorkspace = loadInstrument(instrumentName, reflection);
  Instrument_const_sptr instrument = instWorkspace->getInstrument();

  // Get default spectra range
  double specMin = instrument->getNumberParameter("spectra-min")[0];
  double specMax = instrument->getNumberParameter("spectra-max")[0];

  m_uiForm.set_leSpecMin->setText(QString::number(specMin));
  m_uiForm.set_leSpecMax->setText(QString::number(specMax));

  // Determine whether we need vanadium input
  std::vector<std::string> correctionVector = instrument->getStringParameter("Workflow.Diffraction.Correction");
  bool vanadiumNeeded = false;
  if(correctionVector.size() > 0)
    vanadiumNeeded = (correctionVector[0] == "Vanadium");

  if(vanadiumNeeded)
    m_uiForm.swVanadium->setCurrentIndex(0);
  else
    m_uiForm.swVanadium->setCurrentIndex(1);

  // Hide options that the current instrument config cannot process
  if(instrumentName == "OSIRIS" && reflection == "diffonly")
  {
    m_uiForm.dem_ckSumFiles->setToolTip("OSIRIS cannot sum files in diffonly mode");
    m_uiForm.dem_ckSumFiles->setEnabled(false);
  }
  else
  {
    m_uiForm.dem_ckSumFiles->setToolTip("");
    m_uiForm.dem_ckSumFiles->setEnabled(true);
  }
}

/**
 * Handles opening the directory manager window.
 */
void IndirectDiffractionReduction::openDirectoryDialog()
{
  MantidQt::API::ManageUserDirectories *ad = new MantidQt::API::ManageUserDirectories(this);
  ad->show();
  ad->setFocus();
}

/**
 * Handles the user clicking the help button.
 */
void IndirectDiffractionReduction::help()
{
  QString url = "http://www.mantidproject.org/Indirect_Diffraction_Reduction";
  QDesktopServices::openUrl(QUrl(url));
}

/**
 * Sets up UI components and Qt signal/slot connections.
 */
void IndirectDiffractionReduction::initLayout()
{
  m_uiForm.setupUi(this);

  connect(m_uiForm.pbHelp, SIGNAL(clicked()), this, SLOT(help()));
  connect(m_uiForm.pbManageDirs, SIGNAL(clicked()), this, SLOT(openDirectoryDialog()));
  connect(m_uiForm.pbRun, SIGNAL(clicked()), this, SLOT(demonRun()));

  connect(m_uiForm.cbInst, SIGNAL(currentIndexChanged(int)), this, SLOT(instrumentSelected(int)));
  connect(m_uiForm.cbReflection, SIGNAL(currentIndexChanged(int)), this, SLOT(reflectionSelected(int)));

  // Update run button based on state of raw files field
  connect(m_uiForm.dem_rawFiles, SIGNAL(fileTextChanged(const QString &)), this, SLOT(runFilesChanged()));
  connect(m_uiForm.dem_rawFiles, SIGNAL(findingFiles()), this, SLOT(runFilesFinding()));
  connect(m_uiForm.dem_rawFiles, SIGNAL(fileFindingFinished()), this, SLOT(runFilesFound()));

  m_valInt = new QIntValidator(this);
  m_valDbl = new QDoubleValidator(this);

  m_uiForm.set_leSpecMin->setValidator(m_valInt);
  m_uiForm.set_leSpecMax->setValidator(m_valInt);

  m_uiForm.leRebinStart->setValidator(m_valDbl);
  m_uiForm.leRebinWidth->setValidator(m_valDbl);
  m_uiForm.leRebinEnd->setValidator(m_valDbl);

  loadSettings();

  // Update invalid rebinning markers
  validateRebin();
}

void IndirectDiffractionReduction::initLocalPython()
{
  instrumentSelected(0);
}

void IndirectDiffractionReduction::loadSettings()
{
  QSettings settings;
  QString dataDir = QString::fromStdString(Mantid::Kernel::ConfigService::Instance().getString("datasearch.directories")).split(";")[0];

  settings.beginGroup(m_settingsGroup);
  settings.setValue("last_directory", dataDir);
  m_uiForm.dem_rawFiles->readSettings(settings.group());
  m_uiForm.dem_calFile->readSettings(settings.group());
  m_uiForm.dem_calFile->setUserInput(settings.value("last_cal_file").toString());
  m_uiForm.dem_vanadiumFile->setUserInput(settings.value("last_van_files").toString());
  settings.endGroup();
}

void IndirectDiffractionReduction::saveSettings()
{
  QSettings settings;

  settings.beginGroup(m_settingsGroup);
  settings.setValue("last_cal_file", m_uiForm.dem_calFile->getText());
  settings.setValue("last_van_files", m_uiForm.dem_vanadiumFile->getText());
  settings.endGroup();
}

/**
 * Validates the rebinning fields and updates invalid markers.
 *
 * @returns True if reinning options are valid, flase otherwise
 */
bool IndirectDiffractionReduction::validateRebin()
{
  QString rebStartTxt = m_uiForm.leRebinStart->text();
  QString rebStepTxt = m_uiForm.leRebinWidth->text();
  QString rebEndTxt = m_uiForm.leRebinEnd->text();

  bool rebinValid = true;
  // Need all or none
  if(rebStartTxt.isEmpty() && rebStepTxt.isEmpty() && rebEndTxt.isEmpty())
  {
    rebinValid = true;
    m_uiForm.valRebinStart->setText("");
    m_uiForm.valRebinWidth->setText("");
    m_uiForm.valRebinEnd->setText("");
  }
  else
  {
#define CHECK_VALID(text,validator)\
    if(text.isEmpty())\
    {\
      rebinValid = false;\
      validator->setText("*");\
    }\
    else\
    {\
      rebinValid = true;\
      validator->setText("");\
    }

    CHECK_VALID(rebStartTxt,m_uiForm.valRebinStart);
    CHECK_VALID(rebStepTxt,m_uiForm.valRebinWidth);
    CHECK_VALID(rebEndTxt,m_uiForm.valRebinEnd);

    if(rebinValid && rebStartTxt.toDouble() > rebEndTxt.toDouble())
    {
      rebinValid = false;
      m_uiForm.valRebinStart->setText("*");
      m_uiForm.valRebinEnd->setText("*");
    }
  }

  return rebinValid;
}

/**
 * Checks to see if the vanadium and cal file fields are valid.
 *
 * @returns True fo vanadium and calibration files are valid, false otherwise
 */
bool IndirectDiffractionReduction::validateVanCal()
{
  if(!m_uiForm.dem_calFile->isValid())
    return false;

  if(!m_uiForm.dem_vanadiumFile->isValid())
    return false;

  return true;
}

/**
 * Disables and shows message on run button indicating that run files have benn changed.
 */
void IndirectDiffractionReduction::runFilesChanged()
{
  m_uiForm.pbRun->setEnabled(false);
  m_uiForm.pbRun->setText("Editing...");
}

/**
 * Disables and shows message on run button to indicate searching for data files.
 */
void IndirectDiffractionReduction::runFilesFinding()
{
  m_uiForm.pbRun->setEnabled(false);
  m_uiForm.pbRun->setText("Finding files...");
}

/**
 * Updates run button with result of file search.
 */
void IndirectDiffractionReduction::runFilesFound()
{
  bool valid = m_uiForm.dem_rawFiles->isValid();
  m_uiForm.pbRun->setEnabled(valid);

  if(valid)
    m_uiForm.pbRun->setText("Run");
  else
    m_uiForm.pbRun->setText("Invalid Run");
}

}
}
