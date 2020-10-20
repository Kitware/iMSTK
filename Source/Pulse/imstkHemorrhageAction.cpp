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

#include "imstkHemorrhageAction.h"

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
void
HemorrhageAction::setRate(double val /*in milliLiters/sec*/)
{
    m_hemorrhage->GetRate().SetValue(val, VolumePerTimeUnit::mL_Per_s);
}

void
HemorrhageAction::setType(const Type t)
{
    (t == Type::External) ? m_hemorrhage->SetType(eHemorrhage_Type::External) :
        m_hemorrhage->SetType(eHemorrhage_Type::External);
}

void
HemorrhageAction::SetCompartment(const std::string& name)
{
    m_hemorrhage->SetCompartment(name);
}

double
HemorrhageAction::getRate() const
{
    return m_hemorrhage->GetRate().GetValue(VolumePerTimeUnit::mL_Per_s);
}

std::shared_ptr<SEPatientAction>
HemorrhageAction::getAction()
{
    return std::dynamic_pointer_cast<SEPatientAction>(m_hemorrhage);
}
}