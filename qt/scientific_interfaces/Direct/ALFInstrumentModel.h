// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2022 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
#pragma once

#include "DllConfig.h"
#include "MantidAPI/MatrixWorkspace_fwd.h"
#include "MantidGeometry/IDetector.h"

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace Mantid::Geometry {
class ComponentInfo;
class DetectorInfo;
} // namespace Mantid::Geometry

namespace MantidQt {

namespace MantidWidgets {
class InstrumentActor;
}

namespace CustomInterfaces {

class MANTIDQT_DIRECT_DLL IALFInstrumentModel {

public:
  virtual ~IALFInstrumentModel() = default;

  virtual std::optional<std::string> loadAndTransform(std::string const &filename) = 0;

  virtual std::string loadedWsName() const = 0;
  virtual std::size_t runNumber() const = 0;

  virtual void setSelectedDetectors(Mantid::Geometry::ComponentInfo const &componentInfo,
                                    std::vector<std::size_t> const &detectorIndices) = 0;

  virtual Mantid::API::MatrixWorkspace_sptr
  generateOutOfPlaneAngleWorkspace(MantidQt::MantidWidgets::InstrumentActor const &actor) const = 0;
};

class MANTIDQT_DIRECT_DLL ALFInstrumentModel final : public IALFInstrumentModel {

public:
  ALFInstrumentModel();

  std::optional<std::string> loadAndTransform(const std::string &filename) override;

  inline std::string loadedWsName() const noexcept override { return "ALFData"; };
  std::size_t runNumber() const override;

  void setSelectedDetectors(Mantid::Geometry::ComponentInfo const &componentInfo,
                            std::vector<std::size_t> const &detectorIndices) override;

  Mantid::API::MatrixWorkspace_sptr
  generateOutOfPlaneAngleWorkspace(MantidQt::MantidWidgets::InstrumentActor const &actor) const override;

private:
  void collectXAndYData(MantidQt::MantidWidgets::InstrumentActor const &actor, std::vector<double> &x,
                        std::vector<double> &y, std::vector<double> &e) const;
  void collectAndSortYByX(std::map<double, double> &xy, std::map<double, double> &xe,
                          MantidQt::MantidWidgets::InstrumentActor const &actor,
                          Mantid::API::MatrixWorkspace_const_sptr const &workspace,
                          Mantid::Geometry::ComponentInfo const &componentInfo,
                          Mantid::Geometry::DetectorInfo const &detectorInfo) const;
  std::size_t numberOfDetectorsPerTube(MantidQt::MantidWidgets::InstrumentActor const &actor) const;
  std::size_t numberOfTubes(MantidQt::MantidWidgets::InstrumentActor const &actor) const;

  std::vector<std::size_t> m_detectorIndices;
};

} // namespace CustomInterfaces
} // namespace MantidQt
