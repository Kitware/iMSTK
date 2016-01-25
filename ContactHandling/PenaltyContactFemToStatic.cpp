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

#include "ContactHandling/PenaltyContactFemToStatic.h"
#include "Core/CollisionManager.h"

namespace imstk {

PenaltyContactFemToStatic::PenaltyContactFemToStatic(bool typeBilateral) : PenaltyContactHandling(typeBilateral)
{
    type = PenaltyFemToStatic;
}

//---------------------------------------------------------------------------
PenaltyContactFemToStatic::PenaltyContactFemToStatic(
                                                    bool typeBilateral,
                                                    const std::shared_ptr<SceneObject>& sceneObjFirst,
                                                    const std::shared_ptr<DeformableSceneObject>& sceneObjSecond)
                                                    : PenaltyContactHandling(typeBilateral, sceneObjFirst, sceneObjSecond)
{
    type = PenaltyFemToStatic;
}

//---------------------------------------------------------------------------
PenaltyContactFemToStatic::~PenaltyContactFemToStatic()
{
}

//---------------------------------------------------------------------------
void PenaltyContactFemToStatic::computeUnilateralContactForces()
{
    auto femSceneObject = std::static_pointer_cast<DeformableSceneObject>(this->getSecondSceneObject());
    this->computeForces(femSceneObject);
}

//---------------------------------------------------------------------------
void PenaltyContactFemToStatic::computeBilateralContactForces()
{
}

//---------------------------------------------------------------------------
void PenaltyContactFemToStatic::computeForces(std::shared_ptr< DeformableSceneObject > sceneObject)
{
    if(sceneObject->computeContactForce())
    {
        auto model = sceneObject->getCollisionModel();
        if(!model)
        {
            return;
        }

        auto contactInfo = this->getCollisionPairs()->getContacts(model);
        sceneObject->setContactForcesToZero();
        this->clearContactForces();
        Vec3d force;
        Vec3d velocityProjection;
        int nodeDofID;
        for(auto &contact : contactInfo)
        {
            nodeDofID = 3 * contact->index;
            velocityProjection = sceneObject->getVelocity(nodeDofID);
            velocityProjection = contact->normal.dot(velocityProjection) * contact->normal;

            force = -stiffness * contact->depth * contact->normal - damping * velocityProjection;

            sceneObject->setContactForce(nodeDofID, contact->point, force);
            this->setContactForce(contact->index, force);
        }
    }
}

}
