//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidAlgorithms/SimpleRebin.h"
#include "MantidDataObjects/Workspace2D.h"
#include "MantidDataObjects/Workspace1D.h"
#include "MantidKernel/PropertyWithValue.h"
#include "MantidAPI/WorkspaceProperty.h"
#include "MantidAPI/Workspace.h"

#include <sstream>
#include <numeric>
#include <math.h>

namespace Mantid
{
  namespace Algorithms
  {

    // Register the class into the algorithm factory
    DECLARE_ALGORITHM(SimpleRebin)

    using namespace Kernel;
    using API::WorkspaceProperty;
    using API::Workspace_sptr;
    using API::Workspace;
    //    using DataObjects::Workspace1D_sptr;
    //    using DataObjects::Workspace1D;

    // Get a reference to the logger
    Logger& SimpleRebin::g_log = Logger::get("SimpleRebin");

    /** Initialisation method. Does nothing at present.
    * 
    */
    void SimpleRebin::init()
    {
      declareProperty(new WorkspaceProperty<Workspace>("InputWorkspace","",Direction::Input));  
      declareProperty(new WorkspaceProperty<Workspace>("OutputWorkspace","",Direction::Output));

      //      BoundedValidator<double> *mustBePositive = new BoundedValidator<double>();
      //      mustBePositive->setLower(0);
      declareProperty("xlo",0.0);
      declareProperty("delta",0.0);
      declareProperty("xhi",0.0);
      declareProperty("dist",1);

    }

    /** Executes the rebin algorithm
    * 
    *  @throw runtime_error Thrown if 
    */
    void SimpleRebin::exec()
    {
      // retrieve the properties
      double xlo = getProperty("xlo");
      double delta = getProperty("delta");
      double xhi = getProperty("xhi");
      int dist = getProperty("dist");

      // Get the input workspace
      Workspace_sptr inputW = getProperty("InputWorkspace");

      // Create vectors to hold input parameters
      std::vector<double> xbounds;
      std::vector<double> xsteps;

      // create input vector from input properties
      xbounds.push_back(xlo);
      xbounds.push_back(xhi);
      xsteps.push_back(delta);

      // workspace independent determination of length
      int histnumber = inputW->size()/inputW->blocksize();
      std::vector<double> XValues_new;
      // create new output X axis
      int ntcnew = newAxis(xbounds,xsteps,XValues_new);
      // make output Workspace the same type is the input, but with new length of signal array
      API::Workspace_sptr outputW = API::WorkspaceFactory::Instance().create(inputW->id(),histnumber,ntcnew,ntcnew-1);

      for (int hist=0; hist <  histnumber;hist++)
      {
        // get const references to input Workspace arrays (no copying)
        const std::vector<double>& XValues = inputW->dataX(hist);
        const std::vector<double>& YValues = inputW->dataY(hist);
        const std::vector<double>& YErrors = inputW->dataE(hist);
 
        //get references to output workspace data (no copying)
        std::vector<double>& YValues_new=outputW->dataY(hist);
        std::vector<double>& YErrors_new=outputW->dataE(hist);

        // output data arrays are implicitly filled by function
        rebin(XValues,YValues,YErrors,XValues_new,YValues_new,YErrors_new, dist);
        // Populate the output workspace X values
        outputW->dataX(hist)=XValues_new;
      }

      // Assign it to the output workspace property
      setProperty("OutputWorkspace",outputW);

      return;  
    }


    /** Rebins the data according to new output X array
    *
    * @param xold - old x array of data
    * @param xnew - new x array of data
    * @param yold - old y array of data
    * @param ynew - new y array of data
    * @param eold - old error array of data
    * @param enew - new error array of data
    * @param distribution - flag defining if distribution data (1) or not (0)
    * @throw runtime_error Thrown if algorithm cannot execute
    * @throw invalid_argument Thrown if input to function is incorrect
    **/
    void SimpleRebin::rebin(const std::vector<double>& xold, const std::vector<double>& yold, const std::vector<double>& eold,
      const std::vector<double>& xnew, std::vector<double>& ynew, std::vector<double>& enew, int distribution)

    {
      int i,iold = 0,inew = 0;
      double xo_low, xo_high, xn_low, xn_high, delta(0.0), width;
      int size_yold=yold.size();
      int size_ynew=ynew.size();
      int size_x=xold.size();

      // put in g_log stuff later about histogram data
      if(size_yold != size_x-1)
      {
        g_log.error("SimpleRebin: rebinning not possible on point data ");
        throw std::invalid_argument("SimpleRebin: rebinning not possible on point data");
      }
      while((inew < size_ynew) && (iold < size_yold))
      {
        xo_low = xold[iold];
        xo_high = xold[iold+1];
        xn_low = xnew[inew];
        xn_high = xnew[inew+1];
        if ( xn_high <= xo_low )
        {
          inew++;		/* old and new bins do not overlap */
        }
        else if ( xo_high <= xn_low )
        {
          iold++;		/* old and new bins do not overlap */
        }
        else
        {
          //        delta is the overlap of the bins on the x axis
          delta = std::min(xo_high, xn_high) - std::max(xo_low, xn_low);
          width = xo_high - xo_low;
          if ( (delta <= 0.0) || (width <= 0.0) )
          {            
            g_log.error("SimpleRebin: no bin overlap detected");
            throw std::runtime_error("no bin overlap detected");            
          }
          /*
          *        yoldp contains counts/unit time, ynew contains counts
          *	       enew contains counts**2
          *        ynew has been filled with zeros on creation
          */          
          if(distribution)
          {
            // yold/eold data is distribution
            ynew[inew] += yold[iold]*delta;
            enew[inew] += eold[iold]*eold[iold]*delta*width;
          }
          else
          {
            // yold/eold data is not distribution
            // do implicit division of yold by width in summing.... avoiding the need for temporary yold array
            // this method is ~7% faster and uses less memory
            ynew[inew] += yold[iold]*delta/width; //yold=yold/width
            // eold=eold/width, so divide by width**2 compared with distribution calculation
            enew[inew] += eold[iold]*eold[iold]*delta/width; 
          }
          if ( xn_high > xo_high )
          {
            iold++;
          }
          else
          {
            inew++;
          }
        }
      }

      if(distribution)
      {
        /*
        * convert back to counts/unit time
        */
        for(i=0; i<size_ynew; i++)
        {
          {
            width = xnew[i+1]-xnew[i];
            if (width != 0.0)
            {
              ynew[i] /= width;
              enew[i] = (double)sqrt((double)enew[i]) / width;
            }
            else
            {
              g_log.error("SimpleRebin: consecutive output X values are the same");
              throw std::invalid_argument("Invalid output X array, contains consecutive X values");
            }
          }
        }
      }
      else
      {
        //non distribution , just square root final error value
        for(i=0; i<size_ynew; i++)
          enew[i]=(double)sqrt((double)enew[i]);
      }
      return; //without problems
    }

    /** Creates a new  output X array  according to specific boundary defnitions
    *
    * @param xbounds - array defining limits of particular step size
    * @param xsteps - contains the bin size to use within the limits deined by xbounds
    * e.g. step xstep[0] between xbounds[0] and xbounds[1]
    *      step xstep[1] between xbounds[1] and xbounds[2]
    * @param xnew - new output workspace x array
    * @throw invalid_argument Thrown if input to function is incorrect
    **/
    int SimpleRebin::newAxis(const std::vector<double>& xbounds,
      const std::vector<double>& xsteps, 
      std::vector<double>& xnew)
    {
      double xcurr, xs;
      int i(1), j(0), inew(1);
      int isteps=xsteps.size();
      int ibounds=xbounds.size();

      if(ibounds != isteps+1)
      {
        g_log.error("SimpleRebin: length  of xbounds/xsteps arrays incompatible ");
        throw std::invalid_argument("SimpleRebin: length  of xbounds/xsteps arrays incompatible");
      }

      xcurr = xbounds[0];
      xnew.push_back(xcurr);

      while( (i < ibounds) && (j < isteps) )
      {
        // if step is negative then it is logarithmic step
        if ( xsteps[j] >= 0.0)        
          xs = xsteps[j];        
        else 
          xs = xcurr * fabs(xsteps[j]);
        /* continue stepping unless we get to almost where we want to */
        if ( (xcurr + xs) < (xbounds[i] - (xs * 1.E-6)) )
        {
          xcurr += xs;
        }
        else
        {
          xcurr = xbounds[i];
          i++;
          j++;
        }
        xnew.push_back(xcurr);
        inew++;
      }
      //returns length of new x array or -1 if failure
      return( (i == ibounds) && (j == isteps) ? inew : -1 );
    }

  } // namespace Algorithm
} // namespace Mantid
