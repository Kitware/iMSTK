// This file is part of the iMSTK project.
//
// Copyright (c) Kitware, Inc.
//
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef CONTACT_HANDLING_PENALTYCONTACTHANDLING_H
#define CONTACT_HANDLING_PENALTYCONTACTHANDLING_H

// iMSTK includes
#include "Core/ContactHandling.h"
#include "Core/CollisionManager.h"

namespace imstk {

/// \brief Penalty based for contact handling
class PenaltyContactHandling : public ContactHandling
{
public:
    PenaltyContactHandling(bool typeBilateral);

    PenaltyContactHandling(bool typeBilateral,
                           const std::shared_ptr<SceneObject>& sceneObjFirst,
                           const std::shared_ptr<SceneObject>& sceneObjSecond);

    virtual ~PenaltyContactHandling();

    void resolveContacts();

    /// \brief Get the forces on one the first scene object using penalty method
    virtual void computeUnilateralContactForces() = 0;

    /// \brief Get the forces on both the scene objects using penalty method
    virtual void computeBilateralContactForces() = 0;

    ///
    /// \brief Set stiffness coefficient
    ///
    void setStiffness(const double stiffnessCoeff)
    {
        this->stiffness = stiffnessCoeff;
    }

    ///
    /// \brief Get the stiffness coefficient
    ///
    double getStiffness() const
    {
        return this->stiffness;
    }

    ///
    /// \brief Set stiffness coefficient
    ///
    void setDamping(const double dampingValue)
    {
        this->damping = dampingValue;
    }

    ///
    /// \brief Get the stiffness coefficient
    ///
    double getDamping() const
    {
        return this->damping;
    }

protected:
    double stiffness;
    double damping;
};

}

#endif // CONTACT_HANDLING_PENALTY_CONTACTHANDLING_H
