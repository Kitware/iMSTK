/*=========================================================================

Library: iMSTK

Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
& Imaging in Medicine, Rensselaer Polytechnic Institute.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0.txt

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

=========================================================================*/

#include "imstkPhysiologyModel.h"
#include "imstkLogger.h"
//#include "imstkParallelUtils.h"
#include "imstkTaskGraph.h"

/////////////////////////////////////////////////////////////////////////
// These includes are for Pulse
#include "PulsePhysiologyEngine.h"
#include "engine/SEEngineTracker.h"
#include "engine/SEDataRequest.h"
#include "properties/SEScalarTime.h"
#include "CommonDataModel.h"
#include "engine/SEDataRequestManager.h"
#include "engine/SEEngineTracker.h"
#include "compartment/SECompartmentManager.h"
#include "patient/actions/SEHemorrhage.h"
#include "patient/actions/SESubstanceCompoundInfusion.h"
#include "system/physiology/SEBloodChemistrySystem.h"
#include "system/physiology/SECardiovascularSystem.h"
#include "system/physiology/SEEnergySystem.h"
#include "system/physiology/SERespiratorySystem.h"
#include "substance/SESubstanceManager.h"
#include "substance/SESubstanceCompound.h"
#include "properties/SEScalar0To1.h"
#include "properties/SEScalarFrequency.h"
#include "properties/SEScalarMass.h"
#include "properties/SEScalarMassPerVolume.h"
#include "properties/SEScalarPressure.h"
#include "properties/SEScalarTemperature.h"
#include "properties/SEScalarTime.h"
#include "properties/SEScalarVolume.h"
#include "properties/SEScalarVolumePerTime.h"
#include "compartment/fluid/SELiquidCompartmentGraph.h"


namespace imstk
{
// empty for now, but we can populate config params if needed
PhysiologyModelConfig::PhysiologyModelConfig()
{
    initialize();
}

// empty for now, but we can populate config params if needed
void
PhysiologyModelConfig::initialize()
{}

PhysiologyModel::PhysiologyModel()
{
    m_solveNode = m_taskGraph->addFunction("PhysiologyModel_Solve", std::bind(&PhysiologyModel::solvePulse, this));
}

bool
PhysiologyModel::initialize()
{
    // Create the engine and load the patient
    m_pulseObj = CreatePulseEngine();
    m_pulseObj->GetLogger()->LogToConsole(false);

    if (!m_pulseObj->SerializeFromFile(iMSTK_DATA_ROOT "/states/StandardMale@0s.json"))
    {
      //m_pulseObj->GetLogger()->Error("Could not load Pulse state, check the error");
      return true;
    }

    // Pulse hemorrhage action
    // here, we can add any Pulse actions that we want
    m_hemorrhageLeg = std::make_shared<SEHemorrhage>();
    m_hemorrhageLeg->SetType(eHemorrhage_Type::External);
    m_hemorrhageLeg->SetCompartment(pulse::VascularCompartment::RightLeg);//the location of the hemorrhage

    //PulseConfiguration cfg;
    //cfg.GetTimeStep().SetValue(1 / 100, TimeUnit::s);
    //m_->SetConfigurationOverride(&cfg);

    // The tracker is responsible for advancing the engine time and outputting the data requests below at each time step
    m_femoralCompartment = m_pulseObj->GetCompartments().GetLiquidCompartment(pulse::VascularCompartment::LeftLeg);
    return true;
}

void PhysiologyModel::solvePulse()
{
    // Hemorrhage Starts - instantiate a hemorrhage action and have the engine process it
    m_hemorrhageLeg->GetRate().SetValue(m_hemorrhageRate, VolumePerTimeUnit::mL_Per_min);//the rate of hemorrhage
    m_pulseObj->ProcessAction(*m_hemorrhageLeg);

    m_pulseObj->AdvanceModelTime(m_dT_s, TimeUnit::s);

    m_femoralFlowRate = m_femoralCompartment->GetInFlow(VolumePerTimeUnit::mL_Per_s);

    //uncomment these to get vitals
    //m_pulseObj->GetLogger()->Info(std::stringstream() << "Cardiac Output : " << m_pulseObj->GetCardiovascularSystem()->GetCardiacOutput(VolumePerTimeUnit::mL_Per_min) << VolumePerTimeUnit::mL_Per_min);
    //m_pulseObj->GetLogger()->Info(std::stringstream() << "Hemoglobin Content : " << m_pulseObj->GetBloodChemistrySystem()->GetHemoglobinContent(MassUnit::g) << MassUnit::g);
    //m_pulseObj->GetLogger()->Info(std::stringstream() << "Blood Volume : " << m_pulseObj->GetCardiovascularSystem()->GetBloodVolume(VolumeUnit::mL) << VolumeUnit::mL);
    //m_pulseObj->GetLogger()->Info(std::stringstream() << "Mean Arterial Pressure : " << m_pulseObj->GetCardiovascularSystem()->GetMeanArterialPressure(PressureUnit::mmHg) << PressureUnit::mmHg);
    //m_pulseObj->GetLogger()->Info(std::stringstream() << "Systolic Pressure : " << m_pulseObj->GetCardiovascularSystem()->GetSystolicArterialPressure(PressureUnit::mmHg) << PressureUnit::mmHg);
    //m_pulseObj->GetLogger()->Info(std::stringstream() << "Diastolic Pressure : " << m_pulseObj->GetCardiovascularSystem()->GetDiastolicArterialPressure(PressureUnit::mmHg) << PressureUnit::mmHg);
    //m_pulseObj->GetLogger()->Info(std::stringstream() << "Heart Rate : " << m_pulseObj->GetCardiovascularSystem()->GetHeartRate(FrequencyUnit::Per_min) << "bpm");;
    //m_pulseObj->GetLogger()->Info("Finished");
}

void PhysiologyModel::initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink)
{
    // Setup graph connectivity
    m_taskGraph->addEdge(source, m_solveNode);
    m_taskGraph->addEdge(m_solveNode, sink);
}

//SEScalarTime& PhysiologyModel::GetTimeStep()
//{
//    if (m_timeStep == nullptr)
//      m_timeStep = new SEScalarTime();
//    return *m_timeStep;
//}
//const std::shared_ptr<SELiquidCompartment> PhysiologyModel::getFemoralCompartment()
//{
//    return m_femoralCompartment;
//}
//
//const std::shared_ptr<SEHemorrhage> PhysiologyModel::getHemorrhageModel()
//{
//    return m_hemorrhageLeg;
//}

}// end namespace imstk
