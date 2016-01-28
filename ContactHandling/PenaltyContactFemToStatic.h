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

#ifndef CONTACT_HANDLING_PENALTYCONTACTFEMTOSTATIC_H
#define CONTACT_HANDLING_PENALTYCONTACTFEMTOSTATIC_H

// iMSTK includes
#include "ContactHandling/PenaltyContactHandling.h"

namespace imstk {

///
/// @brief Penalty based for contact handling
///
class PenaltyContactFemToStatic : public PenaltyContactHandling
{
public:
    PenaltyContactFemToStatic(bool typeBilateral);

    PenaltyContactFemToStatic(bool typeBilateral,
                             const std::shared_ptr<InteractionSceneModel>& sceneObjFirst,
                             const std::shared_ptr<InteractionSceneModel>& sceneObjSecond);

    virtual ~PenaltyContactFemToStatic();

    /// \brief Get the forces on one the first scene object using penalty method
    virtual void computeUnilateralContactForces() override;

    /// \brief Get the forces on both the scene objects using penalty method
    virtual void computeBilateralContactForces() override;

    /// \brief Get the forces on both the scene objects using penalty method
    virtual void computeForces(std::shared_ptr<InteractionSceneModel> sceneObject);

};

}

#endif // CONTACT_HANDLING_PENALTY_FEM_TO_STATIC_SCENE_OBJECT
