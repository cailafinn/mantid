#ifndef GETALLEI_TEST_H_
#define GETALLEI_TEST_H_

#include <memory>
#include <boost/math/special_functions/fpclassify.hpp>
#include <cxxtest/TestSuite.h>
#include "MantidAlgorithms/GetAllEi.h"
#include "MantidKernel/TimeSeriesProperty.h"
#include "MantidTestHelpers/WorkspaceCreationHelper.h"


using namespace Mantid;
using namespace Mantid::Algorithms;
using namespace Mantid::API;
class GetAllEiTester : public GetAllEi
{
public:
  void find_chop_speed_and_delay(const API::MatrixWorkspace_sptr &inputWS,
    double &chop_speed,double &chop_delay){
      GetAllEi::findChopSpeedAndDelay(inputWS,chop_speed,chop_delay);
  }
  void findGuessOpeningTimes(const std::pair<double,double> &TOF_range,
    double ChopDelay,double Period,std::vector<double > & guess_opening_times){
      GetAllEi::findGuessOpeningTimes(TOF_range,ChopDelay,Period,guess_opening_times);
  }
  bool filterLogProvided()const{
    return m_useFilterLog;
  }
  double getAvrgLogValue(const API::MatrixWorkspace_sptr &inputWS, const std::string &propertyName){
    std::vector<Kernel::SplittingInterval> splitter;
    return GetAllEi::getAvrgLogValue(inputWS, propertyName,splitter);
  }
   API::MatrixWorkspace_sptr buildWorkspaceToFit(const API::MatrixWorkspace_sptr &inputWS, size_t &wsIndex0){
    return GetAllEi::buildWorkspaceToFit(inputWS,wsIndex0);
   }
  void findBinRanges(const MantidVec & eBins,
      const std::vector<double> & guess_energy,double Eresolution,std::vector<size_t> & irangeMin,
      std::vector<size_t> & irangeMax){
        GetAllEi::findBinRanges(eBins,guess_energy,Eresolution,irangeMin,irangeMax);
  }
  void setResolution(double newResolution){
    this->m_max_Eresolution = newResolution;
  }
  size_t calcDerivativeAndCountZeros(const std::vector<double> &bins,const std::vector<double> &signal,
    std::vector<double> &deriv){
      return GetAllEi::calcDerivativeAndCountZeros(bins,signal,deriv);
  }
};

class GetAllEiTest : public CxxTest::TestSuite
{
public:
  // This pair of boilerplate methods prevent the suite being created statically
  // This means the constructor isn't called when running other tests
  static GetAllEiTest *createSuite() { return new GetAllEiTest(); }
  static void destroySuite( GetAllEiTest *suite ) { delete suite; }

  GetAllEiTest(){
  }

public:
  void testName(){
    TS_ASSERT_EQUALS( m_getAllEi.name(), "GetAllEi" );
  }

  void testVersion(){
    TS_ASSERT_EQUALS( m_getAllEi.version(), 1 );
  }

  void testInit()
  {
    TS_ASSERT_THROWS_NOTHING( m_getAllEi.initialize() );
    TS_ASSERT( m_getAllEi.isInitialized() );
  }
  //
  void test_validators_work(){

    MatrixWorkspace_sptr ws = WorkspaceFactory::Instance().create("Workspace2D",2, 11, 10);
    m_getAllEi.initialize();
    m_getAllEi.setProperty("Workspace",ws);
    m_getAllEi.setProperty("OutputWorkspace","monitor_peaks");
    TSM_ASSERT_THROWS("should throw runtime error on as spectra ID should be positive",m_getAllEi.setProperty("Monitor1SpecID",0),std::invalid_argument);
    m_getAllEi.setProperty("Monitor1SpecID",1);
    m_getAllEi.setProperty("Monitor2SpecID",2);
    TSM_ASSERT_THROWS("should throw runtime error on validation as no appropriate logs are defined",m_getAllEi.execute(),std::runtime_error);
    auto log_messages = m_getAllEi.validateInputs();
    TSM_ASSERT_EQUALS("Three logs should fail",log_messages.size(),2);
    // add invalid property type
    ws->mutableRun().addLogData(new Kernel::PropertyWithValue<double>("Chopper_Speed",10.));
    auto log_messages2 = m_getAllEi.validateInputs();
    TSM_ASSERT_EQUALS("Two logs should fail",log_messages2.size(),2);

    TSM_ASSERT_DIFFERS("should fail for different reason ",log_messages["ChopperSpeedLog"],log_messages2["ChopperSpeedLog"]);
    // add correct property type:
    ws->mutableRun().clearLogs();
    ws->mutableRun().addLogData(new Kernel::TimeSeriesProperty<double>("Chopper_Speed"));
    log_messages = m_getAllEi.validateInputs();
    TSM_ASSERT_EQUALS("One log should fail",log_messages.size(),1);
    TSM_ASSERT("Filter log is not provided ",!m_getAllEi.filterLogProvided());
    ws->mutableRun().addLogData(new Kernel::TimeSeriesProperty<double>("Chopper_Delay"));
    ws->mutableRun().addLogData(new Kernel::TimeSeriesProperty<double>("proton_charge"));
    log_messages = m_getAllEi.validateInputs();
    TSM_ASSERT_EQUALS("All logs are defined",log_messages.size(),0);
    TSM_ASSERT("Filter log is provided ",m_getAllEi.filterLogProvided());

    m_getAllEi.setProperty("Monitor1SpecID",3);
    log_messages = m_getAllEi.validateInputs();
    TSM_ASSERT_EQUALS("Workspace should not have spectra with ID=3",log_messages.size(),1);
  }
  //
  void test_get_chopper_speed(){

    MatrixWorkspace_sptr ws = WorkspaceFactory::Instance().create("Workspace2D",2, 11, 10);
    std::unique_ptr<Kernel::TimeSeriesProperty<double> > chopSpeed(new Kernel::TimeSeriesProperty<double>("Chopper_Speed"));
    m_getAllEi.initialize();
    m_getAllEi.setProperty("Workspace",ws);
    m_getAllEi.setProperty("OutputWorkspace","monitor_peaks");
    m_getAllEi.setProperty("Monitor1SpecID",1);
    m_getAllEi.setProperty("Monitor2SpecID",2);

    for(int i=0;i<10;i++){
      chopSpeed->addValue( Kernel::DateAndTime(10000+10*i, 0), 1.);
    }
    for(int i=0;i<10;i++){
      chopSpeed->addValue( Kernel::DateAndTime(100+10*i, 0), 10.);
    }
    for(int i=0;i<10;i++){
      chopSpeed->addValue( Kernel::DateAndTime(10*i, 0), 100.);
    }
    ws->mutableRun().addLogData(chopSpeed.release());

    // Test sort log by run time.
    TSM_ASSERT_THROWS("Attempt to get log without start/stop time set should fail",
      m_getAllEi.getAvrgLogValue(ws,"ChopperSpeedLog"),std::runtime_error);

    ws->mutableRun().setStartAndEndTime(Kernel::DateAndTime(90,0),Kernel::DateAndTime(10000,0));
    double val = m_getAllEi.getAvrgLogValue(ws,"ChopperSpeedLog");
    TS_ASSERT_DELTA(val,(10*10+100.)/11.,1.e-6);

    ws->mutableRun().setStartAndEndTime(Kernel::DateAndTime(100,0),Kernel::DateAndTime(10000,0));
    val = m_getAllEi.getAvrgLogValue(ws,"ChopperSpeedLog");
    TS_ASSERT_DELTA(val,10.,1.e-6);

    // Test sort log by log.
    std::unique_ptr<Kernel::TimeSeriesProperty<double> > chopDelay(new Kernel::TimeSeriesProperty<double>("Chopper_Delay"));
    std::unique_ptr<Kernel::TimeSeriesProperty<double> > goodFram(new Kernel::TimeSeriesProperty<double>("proton_charge"));

    for(int i=0;i<10;i++){
      auto time = Kernel::DateAndTime(200+10*i, 0);
      chopDelay->addValue(time , 10.);
      if (i<2){
        goodFram->addValue(time,1);
      }else{
        goodFram->addValue(time,0);
      }

    }
    for(int i=0;i<10;i++){
      auto time =  Kernel::DateAndTime(100+10*i, 0);
      chopDelay->addValue(time , 0.1);
      goodFram->addValue(time,1);
    }
    for(int i=0;i<10;i++){
      auto time =  Kernel::DateAndTime(10*i, 0);
      chopDelay->addValue(time , 1.);
      goodFram->addValue(time,0);
    }
    ws->mutableRun().addLogData(chopDelay.release());
    ws->mutableRun().addLogData(goodFram.release());
    // Run validate as this will set up property, which indicates filter log presence
    auto errors  = m_getAllEi.validateInputs();
    TSM_ASSERT_EQUALS("All logs are defined now",errors.size(),0);

    double chop_speed,chop_delay;
    m_getAllEi.find_chop_speed_and_delay(ws,chop_speed,chop_delay);
    TSM_ASSERT_DELTA("Chopper delay should have special speed ",(10*0.1+20)/12., chop_delay,1.e-6);

    goodFram.reset(new Kernel::TimeSeriesProperty<double>("proton_charge"));
    for(int i=0;i<10;i++){
      auto time =  Kernel::DateAndTime(100+10*i, 0);
      goodFram->addValue(time,1);
    }

    ws->mutableRun().addProperty(goodFram.release(),true);
    m_getAllEi.find_chop_speed_and_delay(ws,chop_speed,chop_delay);
    TSM_ASSERT_DELTA("Chopper delay should have special speed",0.1, chop_delay,1.e-6);

  }
  void test_guess_opening_times(){

    std::pair<double,double> TOF_range(5,100);
    double t0(6),Period(10);
    std::vector<double> guess_tof;
    m_getAllEi.findGuessOpeningTimes(TOF_range,t0,Period,guess_tof);
    TSM_ASSERT_EQUALS("should have 10 periods within the specified interval",guess_tof.size(),10);

    guess_tof.resize(0);
    t0 = TOF_range.first;
    m_getAllEi.findGuessOpeningTimes(TOF_range,t0,Period,guess_tof);
    TSM_ASSERT_EQUALS("Still should be 10 periods within the specified interval",guess_tof.size(),10);

    t0 = TOF_range.second;
    TSM_ASSERT_THROWS("Should throw out of range",m_getAllEi.findGuessOpeningTimes(TOF_range,t0,Period,guess_tof),std::runtime_error);

    t0 = 1;
    guess_tof.resize(0);
    m_getAllEi.findGuessOpeningTimes(TOF_range,t0,Period,guess_tof);
    TSM_ASSERT_EQUALS(" should be 9 periods within the specified interval",guess_tof.size(),9);

    guess_tof.resize(0);
    t0 = 21;
    TOF_range.first = 20;
    m_getAllEi.findGuessOpeningTimes(TOF_range,t0,Period,guess_tof);
    TSM_ASSERT_EQUALS(" should be 8 periods within the specified interval",guess_tof.size(),8);
 
  }
  //
   void test_internalWS_to_fit(){
     Mantid::DataObjects::Workspace2D_sptr tws = 
       WorkspaceCreationHelper::create2DWorkspaceWithFullInstrument(5,100,true);
    auto det1=tws->getDetector(0);
    auto det2=tws->getDetector(4);
    auto spec1 = tws->getSpectrum(0);
    auto spec2 = tws->getSpectrum(4);
    auto detID1 = spec1->getDetectorIDs();
    auto detID2 = spec2->getDetectorIDs();

    m_getAllEi.initialize();
    m_getAllEi.setProperty("Workspace",tws);
    m_getAllEi.setProperty("OutputWorkspace","monitor_peaks");
    m_getAllEi.setProperty("Monitor1SpecID",1);
    m_getAllEi.setProperty("Monitor2SpecID",5);

    size_t wsIndex0;
    auto wws = m_getAllEi.buildWorkspaceToFit(tws, wsIndex0);

    auto det1p = wws->getDetector(0);
    auto det2p = wws->getDetector(1);
    TSM_ASSERT_EQUALS("should be the same first detector position",
      det1p->getRelativePos(),det1->getRelativePos());
    TSM_ASSERT_EQUALS("should be the same second detector position",
      det2p->getRelativePos(),det2->getRelativePos());

    TSM_ASSERT_EQUALS("Detector's ID for the first spectrum and new workspace should coincide",
      *(detID1.begin()),(*wws->getSpectrum(0)->getDetectorIDs().begin()));
    TSM_ASSERT_EQUALS("Detector's ID for the second spectrum and new workspace should coincide",
      *(detID2.begin()),(*wws->getSpectrum(1)->getDetectorIDs().begin()));
    auto pSpec1= wws->getSpectrum(0);
    auto pSpec2= wws->getSpectrum(1);
    auto Xsp1 = pSpec1->dataX();
    auto Xsp2 = pSpec2->dataX();
    size_t nSpectra = Xsp2.size();
    TS_ASSERT_EQUALS(nSpectra,101);
    TS_ASSERT(boost::math::isinf(Xsp1[nSpectra-1]));
    TS_ASSERT(boost::math::isinf(Xsp2[nSpectra-1]));

    //for(size_t i=0;i<Xsp1.size();i++){
    //  TS_ASSERT_DELTA(Xsp1[i],Xsp2[i],1.e-6);
    //}
   }
   void test_binRanges(){
     std::vector<size_t> bin_min,bin_max;
     //Index           0 1 2 3 4 5 6 7 8 9  10 11 12 13
     double debin[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,15};
     std::vector<double> ebin(debin,debin+sizeof(debin)/sizeof(double));

     double dguess[]= {1,2,10,15};
     std::vector<double> guess(dguess,dguess+sizeof(dguess)/sizeof(double));
     m_getAllEi.findBinRanges(ebin,guess,0.01,bin_min,bin_max);

     TS_ASSERT_EQUALS(bin_min.size(),4)
     TS_ASSERT_EQUALS(bin_max.size(),4)

     TS_ASSERT_EQUALS(bin_min[0],0);
     TS_ASSERT_EQUALS(bin_max[0],1);
     TS_ASSERT_EQUALS(bin_min[1],0);
     TS_ASSERT_EQUALS(bin_max[1],2);
     TS_ASSERT_EQUALS(bin_min[2],8);
     TS_ASSERT_EQUALS(bin_max[2],10);
     TS_ASSERT_EQUALS(bin_min[3],12);
     TS_ASSERT_EQUALS(bin_max[3],13);
   }
   void test_calcDerivative(){
     double sig[]={1,2,3,4,5,6};
     std::vector<double> signal(sig,sig+sizeof(sig)/sizeof(double));
     double bin[]={2,3,4,5,6,7,8};
     std::vector<double> bins(bin,bin+sizeof(bin)/sizeof(double));

     std::vector<double> deriv;
     size_t nZer = m_getAllEi.calcDerivativeAndCountZeros(bins,signal,deriv);
     TS_ASSERT_EQUALS(nZer,0);
     TS_ASSERT_DELTA(deriv[0],deriv[1],1.e-9);
     TS_ASSERT_DELTA(deriv[0],deriv[5],1.e-9);
     TS_ASSERT_DELTA(deriv[0],deriv[2],1.e-9);
     TS_ASSERT_DELTA(deriv[0],1.,1.e-9);

     double bin1[]={0,1,3,6,10,15,21};
     std::vector<double> bins1(bin1,bin1+sizeof(bin1)/sizeof(double));
     nZer = m_getAllEi.calcDerivativeAndCountZeros(bins1,signal,deriv);
     TS_ASSERT_EQUALS(nZer,0);
     TS_ASSERT_DELTA(deriv[0],deriv[1],1.e-9);
     TS_ASSERT_DELTA(deriv[0],deriv[5],1.e-9);
     TS_ASSERT_DELTA(deriv[0],deriv[2],1.e-9);
     TS_ASSERT_DELTA(deriv[0],0,1.e-9);

     bins.resize(101);
     signal.resize(100);
     for(size_t i=0;i<101;i++){
       bins[i]=double(i)*0.1;
     }
     for(size_t i=0;i<100;i++){
       signal[i]=std::sin(0.5*(bins[i]+bins[i+1]));
     }
     nZer = m_getAllEi.calcDerivativeAndCountZeros(bins,signal,deriv);
     TS_ASSERT_EQUALS(nZer,3);
     for(size_t i=0;i<99;i++){ // intentionally left boundary point -- its accuracy is much lower
        TSM_ASSERT_DELTA("At i="+std::to_string(i),deriv[i],10.*std::cos(0.5*(bins[i]+bins[i+1])),1.e-1);
     }


   }


private:
  GetAllEiTester m_getAllEi;

};

#endif
