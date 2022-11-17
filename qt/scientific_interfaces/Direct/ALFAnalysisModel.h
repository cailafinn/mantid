// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2022 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
#pragma once

#include "DllConfig.h"
#include "MantidAPI/IFunction_fwd.h"
#include "MantidAPI/MatrixWorkspace_fwd.h"

#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace MantidQt {
namespace CustomInterfaces {

class MANTIDQT_DIRECT_DLL IALFAnalysisModel {

public:
  virtual ~IALFAnalysisModel() = default;

  virtual std::optional<std::string> setExtractedWorkspace(Mantid::API::MatrixWorkspace_sptr workspace,
                                                           std::size_t const runNumber) = 0;

  virtual void doFit(std::string const &wsName, std::pair<double, double> const &range) = 0;
  virtual void calculateEstimate(std::string const &workspaceName, std::pair<double, double> const &range) = 0;

  virtual void setPeakCentre(double const centre) = 0;
  virtual double peakCentre() const = 0;

  virtual std::string fitStatus() const = 0;

  virtual std::size_t numberOfTubes() const = 0;

  virtual void clearTwoThetas() = 0;
  virtual void addTwoTheta(double const twoTheta) = 0;
  virtual std::optional<double> averageTwoTheta() const = 0;
  virtual std::vector<double> allTwoThetas() const = 0;
};

class MANTIDQT_DIRECT_DLL ALFAnalysisModel final : public IALFAnalysisModel {

public:
  ALFAnalysisModel();

  std::optional<std::string> setExtractedWorkspace(Mantid::API::MatrixWorkspace_sptr workspace,
                                                   std::size_t const runNumber) override;

  void doFit(std::string const &wsName, std::pair<double, double> const &range) override;
  void calculateEstimate(std::string const &workspaceName, std::pair<double, double> const &range) override;

  void setPeakCentre(double const centre) override;
  double peakCentre() const override;

  std::string fitStatus() const override;

  std::size_t numberOfTubes() const override;

  void clearTwoThetas() override;
  void addTwoTheta(double const twoTheta) override;
  std::optional<double> averageTwoTheta() const override;
  inline std::vector<double> allTwoThetas() const noexcept override { return m_twoThetas; };

private:
  std::string extractedWsName(Mantid::API::MatrixWorkspace_const_sptr const &workspace,
                              std::size_t const runNumber) const;
  Mantid::API::IFunction_sptr calculateEstimate(Mantid::API::MatrixWorkspace_sptr &workspace,
                                                std::pair<double, double> const &range);

  Mantid::API::IFunction_sptr m_function;
  std::string m_fitStatus;
  std::vector<double> m_twoThetas;
  Mantid::API::MatrixWorkspace_sptr m_extractedWorkspace;
};

} // namespace CustomInterfaces
} // namespace MantidQt
