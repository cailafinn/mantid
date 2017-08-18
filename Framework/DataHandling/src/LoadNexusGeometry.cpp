﻿//-----------------------------------------
// Includes
//-----------------------------------------
#include "MantidDataHandling/LoadNexusGeometry.h"

#include "MantidAPI/InstrumentDataService.h"
#include "MantidAPI/RegisterFileLoader.h"

#include "MantidGeometry/Instrument/Detector.h"
#include "MantidGeometry/Instrument/InstrumentVisitor.h"

namespace Mantid {
namespace DataHandling
{
//Register the algorithm into the algorithm factory
DECLARE_NEXUS_FILELOADER_ALGORITHM(LoadNexusGeometry)

///Initialises algorithm
void LoadNexusGeometry::init()
{
    declareProperty("InstrumentName", " ", "Name of Instrument");
}

//Executes algorithm
void LoadNexusGeometry::exec()
{
    //Add instrument to data service
    std::string instName = getPropertyValue("InstrumentName");
    Geometry::Instrument_sptr instrument(new Geometry::Instrument(instName));
    this->inst = instrument;

    API::InstrumentDataService::Instance().add(instName, inst);

}
//Set confidence level for successful loading
int LoadNexusGeometry::confidence(Kernel::NexusDescriptor &descriptor) const
{ return 0; }

//Add component to instrument
Geometry::IComponent* LoadNexusGeometry::addComponent(std::string &name, Eigen::Vector3d &position, Geometry::Instrument_sptr instrument)
{
    Geometry::IComponent *component(new Geometry::ObjCompAssembly(name));
    component->setPos (position(0), position(1), position(2));
    instrument->add(component);
    //Return the component
    return component;
}

//Add source to instrument
void LoadNexusGeometry::addSource(std::string &name, Eigen::Vector3d &position, Geometry::Instrument_sptr instrument)
{
    auto *source(this->addComponent(name, position, instrument));
    instrument->markAsSource(source);
}

//Add sample to instrument
void LoadNexusGeometry::addSample(std::string &name, Eigen::Vector3d &position, Geometry::Instrument_sptr instrument)
{
    auto *sample(this->addComponent(name, position, instrument));
    instrument->markAsSamplePos(sample);
}

//Add detector to instrument
void LoadNexusGeometry::addDetector(std::string &name, Eigen::Vector3d &position, int detId, Geometry::ICompAssembly *parent, Geometry::Instrument_sptr instrument)
{
    Geometry::Detector detector(name, detId, parent);
    detector.setPos(position(0), position(1), position(2));
    instrument->add(&detector);
    instrument->markAsDetectorIncomplete(&detector);
}

}
}
