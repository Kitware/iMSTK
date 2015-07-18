// This file is part of the SimMedTK project.
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
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//---------------------------------------------------------------------------


#ifndef SMPENALTY_FEM_TO_STATIC_SCENE_OBJECT
#define SMPENALTY_FEM_TO_STATIC_SCENE_OBJECT

// SimMedTK includes
#include "ContactHandling/PenaltyContactHandling.h"
#include "Simulators/VegaFemSceneObject.h"

///
/// @brief Penalty based for contact handling
///
class smPenaltyContactFemToStatic : public smPenaltyContactHandling
{
public:
    smPenaltyContactFemToStatic(bool typeBilateral);

    smPenaltyContactFemToStatic(bool typeBilateral,
                             const std::shared_ptr<smSceneObject>& sceneObjFirst,
                             const std::shared_ptr<smSceneObject>& sceneObjSecond);

    virtual ~smPenaltyContactFemToStatic();

    /// \brief Get the forces on one the first scene object using penalty method
    virtual void computeUnilateralContactForces() override;

    /// \brief Get the forces on both the scene objects using penalty method
    virtual void computeBilateralContactForces() override;

};

#endif // SMPENALTY_FEM_TO_STATIC_SCENE_OBJECT
