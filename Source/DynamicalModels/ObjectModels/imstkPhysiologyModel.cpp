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
#include "imstkParallelUtils.h"
#include "imstkPointSet.h"
#include "imstkTaskGraph.h"

/////////////////////////////////////////////////////////////////////////
// These includes are temporary for testing Pulse in iMSTK
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

#include "controller/Controller.h";

namespace imstk
{
PhysiologyModelConfig::PhysiologyModelConfig()
{
    initialize();
}

void
PhysiologyModelConfig::initialize()
{
   
}

PhysiologyModel::PhysiologyModel()
{
    m_solveNode = m_taskGraph->addFunction("PhysiologyModel_Solve", std::bind(&PhysiologyModel::solvePulse, this));
}

bool
PhysiologyModel::initialize()
{
    // Create the engine and load the patient
    m_pulseObj = CreatePulseEngine();
    m_pulseObj->GetLogger()->SetLogFile("./test_results/HowTo_Hemorrhage.log");
    m_pulseObj->GetLogger()->Info("HowTo_Hemorrhage");
    if (!m_pulseObj->SerializeFromFile(iMSTK_DATA_ROOT "/states/StandardMale@0s.json"))
    {
      m_pulseObj->GetLogger()->Error("Could not load Pulse state, check the error");
      return true;
    }

    //PulseConfiguration cfg(m_pulseObj->GetSubstanceManager());

    // The tracker is responsible for advancing the engine time and outputting the data requests below at each time step
    m_dT_s = m_pulseObj->GetTimeStep(TimeUnit::s);
    m_femoralCompartment = m_pulseObj->GetCompartments().GetLiquidCompartment(pulse::VascularCompartment::LeftLeg);
    return true;
}

void PhysiologyModel::solvePulse()
{
    // Create the engine and load the patient
    double timestep = m_pulseObj->GetTimeStep(TimeUnit::s);
    //std::cout << timestep << std::endl;
    //const SEEngineConfiguration* cfg = m_pulseObj->GetConfiguration();

    timestep = m_pulseObj->GetTimeStep(TimeUnit::s);
    //std::cout << timestep << std::endl;
    //m_pulseObj->GetTimeStep().SetValue(1.0 / 50.0, TimeUnit::s);
    m_pulseObj->AdvanceModelTime();
    //m_pulseObj->GetEngineTracker()->TrackData(m_pulseObj->GetSimulationTime(TimeUnit::s));
    m_femoralFlowRate = m_femoralCompartment->GetInFlow(VolumePerTimeUnit::mL_Per_s);

    //m_pulseObj->GetLogger()->Info(std::stringstream() << "Cardiac Output : " << m_pulseObj->GetCardiovascularSystem()->GetCardiacOutput(VolumePerTimeUnit::mL_Per_min) << VolumePerTimeUnit::mL_Per_min);
    //m_pulseObj->GetLogger()->Info(std::stringstream() << "Hemoglobin Content : " << m_pulseObj->GetBloodChemistrySystem()->GetHemoglobinContent(MassUnit::g) << MassUnit::g);
    //m_pulseObj->GetLogger()->Info(std::stringstream() << "Blood Volume : " << m_pulseObj->GetCardiovascularSystem()->GetBloodVolume(VolumeUnit::mL) << VolumeUnit::mL);
    //m_pulseObj->GetLogger()->Info(std::stringstream() << "Mean Arterial Pressure : " << m_pulseObj->GetCardiovascularSystem()->GetMeanArterialPressure(PressureUnit::mmHg) << PressureUnit::mmHg);
    //m_pulseObj->GetLogger()->Info(std::stringstream() << "Systolic Pressure : " << m_pulseObj->GetCardiovascularSystem()->GetSystolicArterialPressure(PressureUnit::mmHg) << PressureUnit::mmHg);
    //m_pulseObj->GetLogger()->Info(std::stringstream() << "Diastolic Pressure : " << m_pulseObj->GetCardiovascularSystem()->GetDiastolicArterialPressure(PressureUnit::mmHg) << PressureUnit::mmHg);
    //m_pulseObj->GetLogger()->Info(std::stringstream() << "Heart Rate : " << m_pulseObj->GetCardiovascularSystem()->GetHeartRate(FrequencyUnit::Per_min) << "bpm");;
    //m_pulseObj->GetLogger()->Info("Finished");
}

void
PhysiologyModel::initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink)
{
    // Setup graph connectivity
    m_taskGraph->addEdge(source, m_solveNode);
    m_taskGraph->addEdge(m_solveNode, sink);
}

//void
//SPHModel::computeTimeStepSize()
//{
//    m_dt = (this->m_timeStepSizeType == TimeSteppingType::Fixed) ? m_defaultDt : computeCFLTimeStepSize();
//}

//Real
//SPHModel::computeCFLTimeStepSize()
//{
//    auto maxVel = ParallelUtils::findMaxL2Norm(getState().getVelocities());
//
//    // dt = CFL * 2r / max{|| v ||}
//    Real timestep = maxVel > Real(1e-6) ?
//                    m_modelParameters->m_CFLFactor * (Real(2.0) * m_modelParameters->m_particleRadius / maxVel) :
//                    m_modelParameters->m_maxTimestep;
//
//    // clamp the time step size to be within a given range
//    if (timestep > m_modelParameters->m_maxTimestep)
//    {
//        timestep = m_modelParameters->m_maxTimestep;
//    }
//    else if (timestep < m_modelParameters->m_minTimestep)
//    {
//        timestep = m_modelParameters->m_minTimestep;
//    }
//
//    return timestep;
//}

} // end namespace imstk
