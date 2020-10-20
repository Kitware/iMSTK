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
#include "imstkTaskGraph.h"

// Pulse 
#include "PulsePhysiologyEngine.h"
#include "CommonDataModel.h"
#include "engine/SEEngineTracker.h"
#include "engine/SEDataRequest.h"
#include "engine/SEDataRequestManager.h"
#include "engine/SEAction.h"
#include "utils/DataTrack.h"
#include "patient/actions/SEHemorrhage.h"
#include "properties/SEScalarVolumePerTime.h"
#include "compartment/SECompartmentManager.h"

#include "properties/SEScalarTime.h"
#include "engine/SEEngineTracker.h"
#include "compartment/SECompartmentManager.h"
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

namespace imstk
{

PhysiologyModel::PhysiologyModel() : AbstractDynamicalModel(DynamicalModelType::Physiology)
{
    m_solveNode = m_taskGraph->addFunction("PhysiologyModel_Solve", std::bind(&PhysiologyModel::solve, this));
}

bool
PhysiologyModel::initialize()
{
    // Create the engine and load the patient
    m_pulseObj = CreatePulseEngine();
    m_pulseObj->GetLogger()->LogToConsole(m_config->m_enableLogging);

    std::string patientFile;
    switch (m_config->m_basePatient)
    {
    case patientPhysiology::StandardMale:
            patientFile = iMSTK_DATA_ROOT "/PhysiologyStates/StandardMale.json";
            break;
    case patientPhysiology::StandardFemale:
            patientFile = iMSTK_DATA_ROOT "/PhysiologyStates/StandardFemale.json";
            break;
    default:
        LOG(WARNING) << "Could not find the patient. Initializing to StandardMale";
        patientFile = iMSTK_DATA_ROOT "/PhysiologyStates/StandardMale.json";
    }

    CHECK(m_pulseObj->SerializeFromFile(patientFile)) << "Could not load Pulse state file.";

    // Submit data requests
    for (auto dataPair : m_dataPairs)
    {
        m_pulseObj->GetEngineTracker()->GetDataRequestManager().CreatePhysiologyDataRequest(dataPair.first, dataPair.second);
    }

    return true;
}

void 
PhysiologyModel::addDataRequest(const std::string& property, SEDecimalFormat* dfault /*= nullptr*/)
{
    m_dataPairs.push_back(PhysiologyDataRequestPair(property, dfault));

    
}

const SECompartment* 
PhysiologyModel::getCompartment(const physiologyCompartmentType type, const std::string& compartmentName)
{
    switch (type)
    {
    case physiologyCompartmentType::Gas:
        return (SECompartment*)m_pulseObj->GetCompartments().GetGasCompartment(compartmentName);
        break;
    case physiologyCompartmentType::Liquid:
        return (SECompartment*)m_pulseObj->GetCompartments().GetLiquidCompartment(compartmentName);
        break;
    case physiologyCompartmentType::Thermal:
        return (SECompartment*)m_pulseObj->GetCompartments().GetThermalCompartment(compartmentName);
        break;
    case physiologyCompartmentType::Tissue:
        return (SECompartment*)m_pulseObj->GetCompartments().GetTissueCompartment(compartmentName);
        break;
    default:
        LOG(WARNING) << "Could not find the compartment type";
        return nullptr;
    }
}

void 
PhysiologyModel::solve()
{
    // Process all actions that are currently stored
    for (auto action : m_actions)
    {
        m_pulseObj->ProcessAction(*action->getAction().get());
    }

    // Advance physiology model in time
    m_pulseObj->AdvanceModelTime(m_config->m_timeStep, TimeUnit::s);

    m_currentTime += m_config->m_timeStep;
}

void 
PhysiologyModel::initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink)
{
    // Setup graph connectivity
    m_taskGraph->addEdge(source, m_solveNode);
    m_taskGraph->addEdge(m_solveNode, sink);
}

void 
Hemorrhage::setRate(double val /*in milliLiters/sec*/)
{
    m_hemorrhage->GetRate().SetValue(val, VolumePerTimeUnit::mL_Per_s);
}

void 
Hemorrhage::setType(const Type t)
{
    (t == Type::External) ? m_hemorrhage->SetType(eHemorrhage_Type::External) :
        m_hemorrhage->SetType(eHemorrhage_Type::External);
}

void 
Hemorrhage::SetCompartment(const std::string& name)
{
    m_hemorrhage->SetCompartment(name);
}

double 
Hemorrhage::getRate() const
{
    return m_hemorrhage->GetRate().GetValue(VolumePerTimeUnit::mL_Per_s);
}

std::shared_ptr<SEPatientAction> 
Hemorrhage::getAction()
{
    return std::dynamic_pointer_cast<SEPatientAction>(m_hemorrhage);
}

}// imstk
