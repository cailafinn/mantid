#ifndef MANTIDQT_CUSTOMINTERFACES_IENGGDIFFMULTIRUNFITTINGWIDGETVIEW_H_
#define MANTIDQT_CUSTOMINTERFACES_IENGGDIFFMULTIRUNFITTINGWIDGETVIEW_H_

#include "MantidAPI/MatrixWorkspace.h"

#include <boost/optional.hpp>
#include <vector>

namespace MantidQt {
namespace CustomInterfaces {

class IEnggDiffMultiRunFittingWidgetView {
public:
  virtual ~IEnggDiffMultiRunFittingWidgetView() = default;

  /// Update the list of loaded run numbers and bank IDs
  virtual void
  updateRunList(const std::vector<std::pair<int, size_t>> &runLabels) = 0;
};

} // CustomInterfaces
} // MantidQt

#endif // MANTIDQT_CUSTOMINTERFACES_IENGGDIFFMULTIRUNFITTINGWIDGETVIEW_H_
