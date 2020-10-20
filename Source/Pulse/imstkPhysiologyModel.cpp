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
#include <PulsePhysiologyEngine.h>
#include <engine/SEEngineTracker.h>
#include <engine/SEDataRequestManager.h>
#include <patient/actions/SEHemorrhage.h>
#include <compartment/SECompartmentManager.h>

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
    case PatientPhysiology::StandardMale:
        patientFile = iMSTK_DATA_ROOT "/PhysiologyStates/StandardMale.json";
        break;
    case PatientPhysiology::StandardFemale:
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
PhysiologyModel::getCompartment(const PhysiologyCompartmentType type, const std::string& compartmentName) const
{
    switch (type)
    {
    case PhysiologyCompartmentType::Gas:
        return (SECompartment*)m_pulseObj->GetCompartments().GetGasCompartment(compartmentName);
        break;
    case PhysiologyCompartmentType::Liquid:
        return (SECompartment*)m_pulseObj->GetCompartments().GetLiquidCompartment(compartmentName);
        break;
    case PhysiologyCompartmentType::Thermal:
        return (SECompartment*)m_pulseObj->GetCompartments().GetThermalCompartment(compartmentName);
        break;
    case PhysiologyCompartmentType::Tissue:
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
    for (auto i : m_actions)
    {
        m_pulseObj->ProcessAction(*i.second->getAction().get());
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
}// imstk
