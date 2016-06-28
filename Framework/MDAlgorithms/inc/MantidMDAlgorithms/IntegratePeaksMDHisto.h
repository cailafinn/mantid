#ifndef MANTID_MDALGORITHMS_INTEGRATEPEAKSMDHISTO_H_
#define MANTID_MDALGORITHMS_INTEGRATEPEAKSMHISTO_H_

#include "MantidAPI/Algorithm.h"
#include "MantidAPI/IMDHistoWorkspace_fwd.h"
#include "MantidDataObjects/PeaksWorkspace.h"
#include "MantidKernel/System.h"
#include "MantidDataObjects/MDHistoWorkspace.h"
#include "MantidAPI/IMDEventWorkspace_fwd.h"
#include "MantidDataObjects/MDEventWorkspace.h"

namespace Mantid {
namespace MDAlgorithms {

/** Integrate single-crystal peaks in reciprocal-space.
 *
 * @author Vickie Lynch
 * @date 2016-06-23
 */
class DLLExport IntegratePeaksMDHisto : public API::Algorithm {
public:
  IntegratePeaksMDHisto();
  /// Algorithm's name for identification
  const std::string name() const override { return "IntegratePeaksMDHisto"; };
  /// Summary of algorithms purpose
  const std::string summary() const override {
    return "Integrate single-crystal peaks in reciprocal space, for "
           "MDHistoWorkspaces.";
  }

  /// Algorithm's version for identification
  int version() const override { return 1; };
  /// Algorithm's category for identification
  const std::string category() const override { return "MDAlgorithms\\Peaks"; }

private:
  /// Initialise the properties
  void init() override;
  /// Run the algorithm
  void exec() override;
 

  DataObjects::MDHistoWorkspace_sptr normalize(
    int h, int k, int l, double box, int gridPts,
     API::MatrixWorkspace_sptr flux,  API::MatrixWorkspace_sptr sa,
     API::IMDEventWorkspace_sptr ws);
  void integratePeak(DataObjects::MDHistoWorkspace_sptr out, double& intensity, double& errorSquared, int gridPts);
};

} // namespace Mantid
} // namespace DataObjects

#endif /* MANTID_MDALGORITHMS_INTEGRATEPEAKSMDHISTO_H_ */
