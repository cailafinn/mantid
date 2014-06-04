#include "MantidICat/CatalogAlgorithmHelper.h"
#include "MantidICat/CatalogGetDataSets.h"
#include "MantidKernel/MandatoryValidator.h"
#include "MantidAPI/CatalogManager.h"
#include "MantidAPI/WorkspaceProperty.h"

namespace Mantid
{
  namespace ICat
  {
    DECLARE_ALGORITHM(CatalogGetDataSets)

    /// Initialisation methods
    void CatalogGetDataSets::init()
    {
      declareProperty("InvestigationId","",boost::make_shared<Kernel::MandatoryValidator<std::string>>(),
          "ID of the selected investigation");
      declareProperty("Session","","The session information of the catalog to use.");
      declareProperty(new API::WorkspaceProperty<API::ITableWorkspace> ("OutputWorkspace", "", Kernel::Direction::Output),
          "The name of the workspace to store the result of datasets search ");
    }

    /// exec methods
    void CatalogGetDataSets::exec()
    {
      CatalogAlgorithmHelper().checkIfLoggedIn();
      auto workspace = API::WorkspaceFactory::Instance().createTable("TableWorkspace");
      API::CatalogManager::Instance().getCatalog(getPropertyValue("Session"))->getDataSets(getProperty("InvestigationId"),workspace);
      setProperty("OutputWorkspace",workspace);
    }

  }
}
