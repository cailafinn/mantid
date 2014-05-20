#ifndef MANTID_SINQ_POLDISOURCESPECTRUM_H
#define MANTID_SINQ_POLDISOURCESPECTRUM_H

#include "MantidSINQ/DllConfig.h"
#include "MantidGeometry/Instrument.h"
#include "MantidKernel/Interpolation.h"

#include "boost/shared_ptr.hpp"

namespace Mantid {
namespace Poldi {

/** PoldiSourceSpectrum :
 *
  Class that represents the source spectrum at POLDI. It is constructed
  from a list of Wavelength/Intensity-pairs and actual intensities for
  a certain wavelength are obtained by interpolation.

      @author Michael Wedel, Paul Scherrer Institut - SINQ
      @date 13/05/2014

  Copyright © 2014 PSI-MSS

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

  File change history is stored at: <https://github.com/mantidproject/mantid>
  Code Documentation is available at: <http://doxygen.mantidproject.org>
*/
using namespace Mantid::Kernel;
using namespace Mantid::Geometry;

class MANTID_SINQ_DLL PoldiSourceSpectrum {
public:
    PoldiSourceSpectrum(Interpolation spectrum);
    PoldiSourceSpectrum(Instrument_const_sptr poldiInstrument);

    ~PoldiSourceSpectrum() { }

    double intensity(double wavelength) const;

protected:
    void setSpectrumFromInstrument(Instrument_const_sptr poldiInstrument);
    IComponent_const_sptr getSourceComponent(Instrument_const_sptr poldiInstrument);
    Parameter_sptr getSpectrumParameter(IComponent_const_sptr source, ParameterMap_sptr instrumentParameterMap);
    void setSpectrum(Parameter_sptr spectrumParameter);

    Interpolation m_spectrum;
};

typedef boost::shared_ptr<PoldiSourceSpectrum> PoldiSourceSpectrum_sptr;

}
}


#endif // MANTID_SINQ_POLDISOURCESPECTRUM_H
