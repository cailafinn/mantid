// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
#include "MantidCrystal/TransformHKL.h"
#include "MantidAPI/IPeaksWorkspace.h"
#include "MantidAPI/Sample.h"
#include "MantidCrystal/SelectCellWithForm.h"
#include "MantidDataObjects/LeanElasticPeaksWorkspace.h"
#include "MantidDataObjects/PeaksWorkspace.h"
#include "MantidGeometry/Crystal/IndexingUtils.h"
#include "MantidGeometry/Crystal/OrientedLattice.h"
#include "MantidKernel/ArrayLengthValidator.h"
#include "MantidKernel/ArrayProperty.h"
#include "MantidKernel/BoundedValidator.h"

namespace Mantid::Crystal {
// Register the algorithm into the AlgorithmFactory
DECLARE_ALGORITHM(TransformHKL)

using namespace Mantid::Kernel;
using namespace Mantid::API;
using namespace Mantid::DataObjects;
using namespace Mantid::Geometry;

const std::string TransformHKL::name() const { return "TransformHKL"; }

int TransformHKL::version() const { return 1; }

const std::string TransformHKL::category() const { return "Crystal\\Peaks"; }

/** Initialize the algorithm's properties.
 */
void TransformHKL::init() {
  this->declareProperty(std::make_unique<WorkspaceProperty<IPeaksWorkspace>>("PeaksWorkspace", "", Direction::InOut),
                        "Input Peaks Workspace");

  auto mustBePositive = std::make_shared<BoundedValidator<double>>();
  mustBePositive->setLower(0.0);

  this->declareProperty(
      std::make_unique<PropertyWithValue<double>>("Tolerance", 0.15, std::move(mustBePositive), Direction::Input),
      "Indexing Tolerance (0.15)");

  std::vector<double> identity_matrix(9, 0.0);
  identity_matrix[0] = 1;
  identity_matrix[4] = 1;
  identity_matrix[8] = 1;
  auto threeBythree = std::make_shared<ArrayLengthValidator<double>>(9);
  this->declareProperty(
      std::make_unique<ArrayProperty<double>>("HKLTransform", std::move(identity_matrix), std::move(threeBythree)),
      "Specify 3x3 HKL transform matrix as a comma separated list of 9 "
      "numbers");

  this->declareProperty("FindError", true,
                        "Whether to obtain the error in the lattice parameters. "
                        "Set this to false if there are not enough peaks to do an optimization");

  this->declareProperty(std::make_unique<PropertyWithValue<int>>("NumIndexed", 0, Direction::Output),
                        "Gets set with the number of indexed peaks.");

  this->declareProperty(std::make_unique<PropertyWithValue<double>>("AverageError", 0.0, Direction::Output),
                        "Gets set with the average HKL indexing error.");
}

/** Execute the algorithm.
 */
void TransformHKL::exec() {
  IPeaksWorkspace_sptr ws = this->getProperty("PeaksWorkspace");
  if (!ws) {
    throw std::runtime_error("Could not read the peaks workspace");
  }

  OrientedLattice o_lattice = ws->mutableSample().getOrientedLattice();
  Matrix<double> UB = o_lattice.getUB();

  if (!IndexingUtils::CheckUB(UB)) {
    throw std::runtime_error("ERROR: The stored UB is not a valid orientation matrix");
  }

  std::vector<double> tran_vec = getProperty("HKLTransform");
  DblMatrix hkl_tran(tran_vec);

  std::ostringstream str_stream;
  str_stream << hkl_tran;
  std::string hkl_tran_string = str_stream.str();
  g_log.notice() << "Applying Tranformation " << hkl_tran_string << '\n';

  if (hkl_tran.numRows() != 3 || hkl_tran.numCols() != 3) {
    throw std::runtime_error("ERROR: The specified transform must be a 3 X 3 matrix.\n" + hkl_tran_string);
  }

  DblMatrix hkl_tran_inverse(hkl_tran);
  double det = hkl_tran_inverse.Invert();

  if (fabs(det) < 1.0e-5) {
    throw std::runtime_error("ERROR: The specified matrix is invalid (essentially singular.)" + hkl_tran_string);
  }
  if (det < 0) {
    std::ostringstream error_stream;
    error_stream << hkl_tran;
    throw std::runtime_error("ERROR: The determinant of the matrix is negative.\n" + str_stream.str());
  }
  double tolerance = this->getProperty("Tolerance");

  // Transform looks OK so update UB and
  // transform the hkls
  UB = UB * hkl_tran_inverse;
  g_log.notice() << "Transformed UB = " << UB << '\n';
  o_lattice.setUB(UB);

  Matrix<double> modHKL = o_lattice.getModHKL();

  Matrix<double> modUB = UB * modHKL;

  o_lattice.setModHKL(hkl_tran * modHKL);

  bool redetermine_error = this->getProperty("FindError");

  if (redetermine_error) {
    std::vector<double> sigabc(6);
    SelectCellWithForm::DetermineErrors(sigabc, UB, modUB, ws, tolerance);
    o_lattice.setError(sigabc[0], sigabc[1], sigabc[2], sigabc[3], sigabc[4], sigabc[5]);
  } else {
    o_lattice.setError(0, 0, 0, 0, 0, 0);
  }

  ws->mutableSample().setOrientedLattice(std::make_unique<OrientedLattice>(o_lattice));

  int n_peaks = ws->getNumberPeaks();

  // transform all the HKLs and record the new HKL
  // and q-vectors for peaks ORIGINALLY indexed
  int num_indexed = 0;
  std::vector<V3D> miller_indices;
  std::vector<V3D> q_vectors;
  for (int i = 0; i < n_peaks; i++) {
    IPeak &peak = ws->getPeak(i);
    V3D hkl(peak.getHKL());
    V3D ihkl(peak.getIntHKL());
    peak.setIntHKL(hkl_tran * ihkl);
    miller_indices.emplace_back(hkl_tran * hkl - modHKL * peak.getIntMNP());
    peak.setHKL(hkl_tran * hkl);
    q_vectors.emplace_back(peak.getQSampleFrame() - modUB * peak.getIntMNP() * 2 * M_PI);
    num_indexed++;
  }

  double average_error = IndexingUtils::IndexingError(UB, miller_indices, q_vectors);

  // Tell the user what happened.
  g_log.notice() << o_lattice << "\n";
  g_log.notice() << "Transformed Miller indices on previously valid indexed Peaks.\n";
  g_log.notice() << "Set hkl to 0,0,0 on peaks previously indexed out of tolerance.\n";
  g_log.notice() << "Now, " << num_indexed << " are indexed with average error " << average_error << "\n";

  // Save output properties
  this->setProperty("NumIndexed", num_indexed);
  this->setProperty("AverageError", average_error);
}

} // namespace Mantid::Crystal
