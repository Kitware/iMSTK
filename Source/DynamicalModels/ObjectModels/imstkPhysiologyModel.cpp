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
#include "properties/SEScalarVolumePerTimeArea.h"
#include "compartment/fluid/SELiquidCompartmentGraph.h"
#include "utils/DataTrack.h"



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

    CHECK(m_pulseObj->SerializeFromFile(iMSTK_DATA_ROOT "/states/StandardMale@0s.json")) << "Could not load Pulse state file.";

    setUpDataRequests();

    // Pulse hemorrhage action
    // here, we can add any Pulse actions that we want
    m_hemorrhageLeg = std::make_shared<SEHemorrhage>();
    m_hemorrhageLeg->SetType(eHemorrhage_Type::External);
    m_hemorrhageLeg->SetCompartment(pulse::VascularCompartment::RightLeg);//the location of the hemorrhage

    // The tracker is responsible for advancing the engine time and outputting the data requests below at each time step
    m_femoralCompartment = m_pulseObj->GetCompartments().GetLiquidCompartment(pulse::VascularCompartment::LeftLeg);

    m_dT_s = 0.0;
    m_totalTime = 0.0;
    m_femoralFlowRate = 0;
    m_hemorrhageRate = 0;
    return true;
}

void PhysiologyModel::setUpDataRequests()
{
    // Setup data requests
    m_pulseObj->GetEngineTracker()->GetDataRequestManager().CreatePhysiologyDataRequest("DiastolicArterialPressure", PressureUnit::mmHg);
    m_pulseObj->GetEngineTracker()->GetDataRequestManager().CreatePhysiologyDataRequest("HeartRate", FrequencyUnit::Per_min);
    m_pulseObj->GetEngineTracker()->GetDataRequestManager().CreatePhysiologyDataRequest("MeanArterialPressure", PressureUnit::mmHg);
    m_pulseObj->GetEngineTracker()->GetDataRequestManager().CreatePhysiologyDataRequest("SystolicArterialPressure", PressureUnit::mmHg);
    m_pulseObj->GetEngineTracker()->GetDataRequestManager().SetResultsFilename("pulse_vitals.csv");

    m_aorta = m_pulseObj->GetCompartments().GetLiquidCompartment(pulse::VascularCompartment::Aorta);
}

void PhysiologyModel::solvePulse()
{
    // Hemorrhage Starts - instantiate a hemorrhage action and have the engine process it
    m_hemorrhageLeg->GetRate().SetValue(m_hemorrhageRate, VolumePerTimeUnit::mL_Per_s);//the rate of hemorrhage
    m_pulseObj->ProcessAction(*m_hemorrhageLeg);

    m_pulseObj->AdvanceModelTime(m_dT_s, TimeUnit::s);

    m_femoralFlowRate = m_femoralCompartment->GetInFlow(VolumePerTimeUnit::mL_Per_s);

    m_pulseObj->GetEngineTracker()->GetDataTrack().Probe("Aorta Pressure (mmHg)", m_aorta->GetPressure(PressureUnit::mmHg));
    m_pulseObj->GetEngineTracker()->GetDataTrack().Probe("Hemorrhage Rate (mL/s)", m_hemorrhageLeg->GetRate(VolumePerTimeUnit::mL_Per_s));

    m_pulseObj->GetEngineTracker()->TrackData(m_totalTime);

    // Check for hypovolumic shock
    if (m_pulseObj->GetCardiovascularSystem()->GetBloodVolume(VolumeUnit::mL) <= (0.65 * 5517.734437810328))
    {
        LOG(INFO) << "Patient in hypovolemic shock at time: " << m_totalTime;
    }

    // Check for cardiogenic shock
    if (m_pulseObj->GetCardiovascularSystem()->GetCardiacIndex(VolumePerTimeAreaUnit::L_Per_min_m2) < 2.2 &&
        m_pulseObj->GetCardiovascularSystem()->GetSystolicArterialPressure(PressureUnit::mmHg) < 90.0 &&
        m_pulseObj->GetCardiovascularSystem()->GetPulmonaryCapillariesWedgePressure(PressureUnit::mmHg) > 15.0)
    {
        LOG(INFO) << "Patient in cardiogenic shock at time: " << m_totalTime;

        /// \event Patient: Cardiogenic Shock: Cardiac Index has fallen below 2.2 L/min-m^2, Systolic Arterial Pressure is below 90 mmHg, and Pulmonary Capillary Wedge Pressure is above 15.0.
        /// \cite dhakam2008review
    }
    m_totalTime += m_dT_s;
}

void PhysiologyModel::initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink)
{
    // Setup graph connectivity
    m_taskGraph->addEdge(source, m_solveNode);
    m_taskGraph->addEdge(m_solveNode, sink);
}

}// end namespace imstk
