// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2019 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
#pragma once
#include "../DllConfig.h"
#include <QString>
#include <QStringList>
#include <string>

namespace MantidQt {
namespace CustomInterfaces {
namespace InterfaceUtils {

/// The function to use to check whether input data should be restricted based on its name.
/// This is defined, rather than calling SettingsHelper::restrictInputDataByName() directly, to make it
/// possible to override it in tests in order to mock out the SettingsHelper.
MANTIDQT_INELASTIC_DLL extern std::function<bool()> restrictInputDataByName;

MANTIDQT_INELASTIC_DLL std::string getInterfaceProperty(std::string const &interfaceName,
                                                        std::string const &propertyName, std::string const &attribute);

MANTIDQT_INELASTIC_DLL QStringList getExtensions(std::string const &interfaceName);
MANTIDQT_INELASTIC_DLL QStringList getCalibrationExtensions(std::string const &interfaceName);

MANTIDQT_INELASTIC_DLL QStringList getSampleFBSuffixes(std::string const &interfaceName);
MANTIDQT_INELASTIC_DLL QStringList getSampleWSSuffixes(std::string const &interfaceName);

MANTIDQT_INELASTIC_DLL QStringList getVanadiumFBSuffixes(std::string const &interfaceName);
MANTIDQT_INELASTIC_DLL QStringList getVanadiumWSSuffixes(std::string const &interfaceName);

MANTIDQT_INELASTIC_DLL QStringList getResolutionFBSuffixes(std::string const &interfaceName);
MANTIDQT_INELASTIC_DLL QStringList getResolutionWSSuffixes(std::string const &interfaceName);

MANTIDQT_INELASTIC_DLL QStringList getCalibrationFBSuffixes(std::string const &interfaceName);
MANTIDQT_INELASTIC_DLL QStringList getCalibrationWSSuffixes(std::string const &interfaceName);

MANTIDQT_INELASTIC_DLL QStringList getContainerFBSuffixes(std::string const &interfaceName);
MANTIDQT_INELASTIC_DLL QStringList getContainerWSSuffixes(std::string const &interfaceName);

MANTIDQT_INELASTIC_DLL QStringList getCorrectionsFBSuffixes(std::string const &interfaceName);
MANTIDQT_INELASTIC_DLL QStringList getCorrectionsWSSuffixes(std::string const &interfaceName);

MANTIDQT_INELASTIC_DLL QPair<double, double> convertTupleToQPair(std::tuple<double, double> const &doubleTuple);
MANTIDQT_INELASTIC_DLL std::pair<double, double> convertTupleToPair(std::tuple<double, double> const &doubleTuple);
MANTIDQT_INELASTIC_DLL QString makeQStringNumber(double value, int precision);

} // namespace InterfaceUtils
} // namespace CustomInterfaces
} // namespace MantidQt
