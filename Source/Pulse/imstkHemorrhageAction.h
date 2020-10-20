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

#pragma once

#include "imstkPhysiologyModel.h"

class SEHemorrhage;

namespace imstk
{
class HemorrhageAction : public PhysiologyAction
{
public:
    enum class Type { External = 0, Internal };

    HemorrhageAction(const Type t, const std::string& name)
    {
        this->setType(t);
        this->SetCompartment("pulse::" + name);
    }

public:
    ///
    /// \brief Set the rate of hemorrhage
    ///
    void setRate(double val /*in milliLiters/sec*/);

    ///
    /// \brief Set the rate of hemorrhage
    ///
    void setType(const Type t);

    ///
    /// \brief Set the vascular compartment for hemorrhage
    /// The string is expected to be pulse::VascularCompartment::<name_of_compartment>
    /// (Refer: PulsePhysiologyEngine.h)
    ///
    void SetCompartment(const std::string& name);

    ///
    /// \brief Get the rate of hemorrhage in milliLiters/sec
    ///
    double getRate() const;

    ///
    /// \brief
    ///
    std::shared_ptr<SEPatientAction> getAction();

protected:
    std::shared_ptr<SEHemorrhage> m_hemorrhage;
};
}