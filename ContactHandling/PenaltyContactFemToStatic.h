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
class PenaltyContactFemToStatic : public PenaltyContactHandling
{
public:
    PenaltyContactFemToStatic(bool typeBilateral);

    PenaltyContactFemToStatic(bool typeBilateral,
                             const std::shared_ptr<SceneObject>& sceneObjFirst,
                             const std::shared_ptr<SceneObjectDeformable>& sceneObjSecond);

    virtual ~PenaltyContactFemToStatic();

    /// \brief Get the forces on one the first scene object using penalty method
    virtual void computeUnilateralContactForces() override;

    /// \brief Get the forces on both the scene objects using penalty method
    virtual void computeBilateralContactForces() override;

    /// \brief Get the forces on both the scene objects using penalty method
    virtual void computeForces(std::shared_ptr<SceneObjectDeformable> sceneObject)
    {
        if(sceneObject->computeContactForce())
        {
            auto model = sceneObject->getModel();
            if(!model)
            {
                return;
            }

            auto contactInfo = this->getCollisionPairs()->getContacts(model);
            sceneObject->setContactForcesToZero();
            core::Vec3d force;
            core::Vec3d velocityProjection;
            int nodeDofID;
            for(auto &contact : contactInfo)
            {
                nodeDofID = 3 * contact->index;
                velocityProjection = sceneObject->getVelocity(nodeDofID);
                velocityProjection = contact->normal.dot(velocityProjection) * contact->normal;

                force = -stiffness * contact->depth * contact->normal - damping * velocityProjection;

                sceneObject->setContactForce(nodeDofID, contact->point, force);
            }
        }
    }

};

#endif // SMPENALTY_FEM_TO_STATIC_SCENE_OBJECT
