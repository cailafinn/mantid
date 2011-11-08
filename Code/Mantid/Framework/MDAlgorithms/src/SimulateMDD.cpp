/*WIKI* 


*WIKI*/
//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidMDAlgorithms/SimulateMDD.h"
#include "MantidMDAlgorithms/TobyFitSimulate.h"
#include <math.h>

#include <gsl/gsl_errno.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_roots.h>
#include <algorithm>

#include "MantidKernel/Tolerance.h"
#include "MantidGeometry/Math/mathSupport.h"
#include "MantidKernel/Matrix.h"
#include "MantidAPI/IMDIterator.h"

using namespace Mantid::API;
using namespace Mantid::Kernel;

namespace Mantid
{
    namespace MDAlgorithms
    {
        // Register the class into the algorithm factory
        DECLARE_ALGORITHM(SimulateMDD)
        // Constructor
        SimulateMDD::SimulateMDD() //: m_sobol(false), m_randSeed(12345678),m_randGen(NULL)
        {
        }

        SimulateMDD::~SimulateMDD()
        {}

        void SimulateMDD::init()
        {
            //declareProperty(new WorkspaceProperty<>("InputWorkspace","",Direction::Input), "Name of the input Workspace");
            // for testing just use a dummy string for the input workspace name
            declareProperty("InputMDWorkspace","",Direction::Input );
            //

            // declare functions - for now only use background model
            // may have different backgrounds for different cuts, which will require an extension
            // 
            declareProperty("ForegroundModel","",Direction::Input );
            declareProperty("ForegroundModelP1",0.0, "First param of bg model", Direction::Input);
            declareProperty("ForegroundModelP2",0.0, "Second param of bg model", Direction::Input);
            declareProperty("ForegroundModelP3",0.0, "Third param of bg model", Direction::Input);
            declareProperty("BackgroundModel","",Direction::Input );
            declareProperty("BackgroundModelP1",0.0, "Background bp1", Direction::Input);
            declareProperty("BackgroundModelP2",0.0, "Background bp2", Direction::Input);
            declareProperty("BackgroundModelP3",0.0, "Background bp3", Direction::Input);
            declareProperty("BackgroundModelP4",0.0, "Background bp4", Direction::Input);
            declareProperty("BackgroundModelP5",0.0, "Background bp5", Direction::Input);
            declareProperty("BackgroundModelP6",0.0, "Background bp6", Direction::Input);
            declareProperty("BackgroundModelP7",0.0, "Background bp7", Direction::Input);
            declareProperty("BackgroundModelP8",0.0, "Background bp8", Direction::Input);
            // In future, should be a new MDData workspace with the simulated results
            declareProperty("OutputMDWorkspace","",Direction::Output);
            declareProperty("Residual", -1.0, Direction::Output);
        }

        void SimulateMDD::exec()
        {
            std::string inputMDwrkspc = getProperty("InputMDWorkspace");
            std::string bgmodel = getProperty("BackgroundModel");
            const double bgparaP1 = getProperty("BackgroundModelP1");
            const double bgparaP2 = getProperty("BackgroundModelP2");
            const double bgparaP3 = getProperty("BackgroundModelP3");
            const double bgparaP4 = getProperty("BackgroundModelP4");
            const double bgparaP5 = getProperty("BackgroundModelP5");
            const double bgparaP6 = getProperty("BackgroundModelP6");
            const double bgparaP7 = getProperty("BackgroundModelP7");
            const double bgparaP8 = getProperty("BackgroundModelP8");

            std::string fgmodel = getProperty("ForegroundModel");
            const double fgparaP1 = getProperty("ForegroundModelP1");
            const double fgparaP2 = getProperty("ForegroundModelP2");
            const double fgparaP3 = getProperty("ForegroundModelP3");
            std::string inputMDoutput = getProperty("OutputMDWorkspace");

            //boost::shared_ptr<Mantid::API::Workspace> inputWS = getProperty("InputWorkspaceTMP");
            imdwCut = boost::dynamic_pointer_cast<Mantid::API::IMDWorkspace>(AnalysisDataService::Instance().retrieve(inputMDwrkspc));
            std::vector<double> cellBg;
            SimBackground(cellBg, bgmodel,bgparaP1,bgparaP2,bgparaP3,bgparaP4,bgparaP5,bgparaP6,bgparaP7,bgparaP8);
            // currently this does nothing, but will be called in future
            if(fgmodel.compare("")!=0) {
                TobyFitSimulate* tfSim = new TobyFitSimulate();
                tfSim->SimForeground(imdwCut,fgmodel,fgparaP1,fgparaP2,fgparaP3);
            }
            double residual=0;
            // TO DO add in foreground component
            double weightSq;
            IMDIterator * it = imdwCut->createIterator();
            size_t i=0;
            do
            {
//                weightSq=1./pow(imdwCut->getCell(i).getError(),2);
//                residual+=pow(cellBg[i]-imdwCut->getCell(i).getSignal(),2)*weightSq;
                weightSq = 1./pow(it->getNormalizedError(),2);
                residual += pow(cellBg[i] - it->getNormalizedSignal(), 2) * weightSq;
                i++;
            } while( it->next() );
            delete it;
            setProperty("Residual", residual);
        }


        void SimulateMDD::SimBackground(std::vector<double>& cellBg, const std::string & bgmodel,const double bgparaP1, const double bgparaP2, const double bgparaP3,
                                        const double bgparaP4, const double bgparaP5, const double bgparaP6,
                                        const double bgparaP7, const double bgparaP8)
        {
          IMDIterator * it = imdwCut->createIterator();
          size_t i = 0;
          // loop over cells of cut
          do
          {
            double bgsum=0.;
            double eps,phi;
            size_t numEvents = it->getNumEvents();
            // Assume that the energy (centre point) is in coordinate.t of first point vertex
            if( ! bgmodel.compare("QuadEnTrans"))
            {
              for(size_t j=0; j<numEvents; j++)
              {
                eps = it->getInnerPosition(j, 3);
                bgsum+=bgparaP1+eps*(bgparaP2+eps*bgparaP3);
              }
            }
            else if( ! bgmodel.compare("ExpEnTrans"))
            {
              for(size_t j=0; j<numEvents; j++)
              {
                eps = it->getInnerPosition(j, 3);
                bgsum+=bgparaP1+bgparaP2*exp(-eps/bgparaP3);
              }
            }
            else if( ! bgmodel.compare("QuadEnTransAndPhi"))
            {
              double dphi,deps;
              for(size_t j=0; j<numEvents; j++)
              {
                eps = it->getInnerPosition(j, 3);
                deps=eps-bgparaP1;
                phi=0.; // TO DO: get phi of detector
                dphi=phi-bgparaP2;
                bgsum+=bgparaP3+bgparaP4*deps+bgparaP5*dphi+bgparaP6*deps*deps+2.*bgparaP7*deps*dphi+bgparaP8*dphi*dphi;
              }
            }
            else if( ! bgmodel.compare("ExpEnTransAndPhi"))
            {
              for(size_t j=0; j<numEvents; j++)
              {
                eps = it->getInnerPosition(j, 3);
                phi=0.; // TO DO: get phi of detector
                bgsum+=bgparaP3*exp(-(eps-bgparaP1)/bgparaP4 - (phi-bgparaP2)/bgparaP5 );
              }
            }
            else
            {
              std::string message="undefined background model: "+bgmodel;
              g_log.error(message);
              throw std::invalid_argument(message);
            }
            // Would pre size cellBg vector, but no getNCell method for IMDWorkspace
            if(cellBg.size()<i+1)
              cellBg.push_back(bgsum);
            else
              cellBg[i]+=bgsum;
            i++;
          } while (it->next());

        }

    } // namespace Algorithms
} // namespace Mantid
