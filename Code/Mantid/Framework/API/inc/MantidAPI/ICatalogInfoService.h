#ifndef MANTID_API_ICATLOGINFOSERVICE_H_
#define MANTID_API_ICATLOGINFOSERVICE_H_

#include "MantidAPI/DllConfig.h"

namespace Mantid
{
  namespace API
  {
   /**
    This class is responsible for interfacing with the Information Data Service (IDS)
    to upload and download files to and from the archives.

    @author Jay Rainey, ISIS Rutherford Appleton Laboratory
    @date 24/02/2010

    Copyright &copy; 2010 ISIS Rutherford Appleton Laboratory & NScD Oak Ridge National Laboratory

    This file is part of Mantid.

    Mantid is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    Mantid is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    File change history is stored at: <https://github.com/mantidproject/mantid>.
    Code Documentation is available at: <http://doxygen.mantidproject.org>
    */
    class DLLExport ICatalogInfoService
    {
      public:
        // Virtual destructor
        virtual ~ICatalogInfoService(){};
        /// Obtain url to download a file from.
        virtual void getDownloadURL(const long long&,std::string&) = 0;
        /// Obtain the url to upload a file to.
        virtual const std::string getUploadURL(const std::string&,const std::string&,const std::string&) = 0;
    };
  }
}

#endif /*MANTID_API_ICATLOGINFOSERVICE_H_*/
