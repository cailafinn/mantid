// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2021 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
#pragma once

#include "MantidAPI/Algorithm.h"
#include "MantidAlgorithms/DllConfig.h"

namespace Mantid {
namespace Algorithms {

/** Stitches overlapping spectra from multiple 2D workspaces.
 */
class MANTID_ALGORITHMS_DLL Stitch : public API::Algorithm {
public:
  const std::string name() const override;
  int version() const override;
  const std::string category() const override;
  const std::string summary() const override;
  std::map<std::string, std::string> validateInputs() override;

private:
  void init() override;
  void exec() override;
  void scale(Mantid::API::MatrixWorkspace_sptr wsToMatch, Mantid::API::MatrixWorkspace_sptr wsToScale,
             Mantid::API::MatrixWorkspace_sptr scaleFactorsWorkspace, const std::vector<std::string> &inputs);
  Mantid::API::MatrixWorkspace_sptr merge(const std::vector<std::string> &workspaces);
  std::vector<std::string> scaleManual(const std::vector<std::string> &, const std::vector<double> &scaleFactors,
                                       Mantid::API::MatrixWorkspace_sptr scaleFactorsWorkspace);
  void recordScaleFactor(Mantid::API::MatrixWorkspace_sptr scaleFactorWorkspace,
                         Mantid::API::MatrixWorkspace_sptr medianWorkspace,
                         Mantid::API::MatrixWorkspace_sptr scaledWorkspace, const std::vector<std::string> &inputs);
  void cloneWorkspaces(const std::vector<std::string> &inputs);
};

} // namespace Algorithms
} // namespace Mantid
