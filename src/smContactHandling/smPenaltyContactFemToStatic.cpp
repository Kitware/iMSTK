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

#include "smPenaltyContactFemToStatic.h"

#include "smCore/smCollisionPair.h"

smPenaltyContactFemToStatic::smPenaltyContactFemToStatic(bool typeBilateral) : smPenaltyContactHandling(typeBilateral)
{
    type = smContactHandlingType::PenaltyFemToStatic;
}

smPenaltyContactFemToStatic::smPenaltyContactFemToStatic(
                                                    bool typeBilateral,
                                                    const std::shared_ptr<smSceneObject>& sceneObjFirst,
                                                    const std::shared_ptr<smSceneObject>& sceneObjSecond)
                                                    : smPenaltyContactHandling(typeBilateral, sceneObjFirst, sceneObjSecond)
{
    type = smContactHandlingType::PenaltyFemToStatic;
}

smPenaltyContactFemToStatic::~smPenaltyContactFemToStatic()
{
}

void smPenaltyContactFemToStatic::computeUnilateralContactForces()
{
    int penetratedNode, nodeDofID;
    const double stiffness = 1.0e4, damping = 1.0e5;
    smVec3d velocityProjection;

    std::vector<std::shared_ptr<smContact>> contactInfo = this->getCollisionPairs()->getContacts();

    if (this->getSecondSceneObject()->getType() == SIMMEDTK_SMVEGAFEMSCENEOBJECT
        && this->getFirstSceneObject()->getType() == SIMMEDTK_SMSTATICSCENEOBJECT)
    {
        auto femSceneObject = std::static_pointer_cast<smVegaFemSceneObject>(this->getSecondSceneObject());

        femSceneObject->setContactForcesToZero();
        smVec3d force;
        for (int i = 0; i < contactInfo.size(); i++)
        {
            nodeDofID = 3 * contactInfo[i]->index;
            velocityProjection = femSceneObject->getVelocityOfNodeWithDofID(nodeDofID);
            velocityProjection = contactInfo[i]->normal.dot(velocityProjection) * contactInfo[i]->normal;

            force = stiffness * -contactInfo[i]->depth * contactInfo[i]->normal - damping * velocityProjection;

            femSceneObject->setContactForceOfNodeWithDofID(nodeDofID, force);

        }
    }
    else
    {
        std::cout << "Error: Scene objects don't match the required types in 'smPenaltyContactFemToStatic' \n";
    }
}

void smPenaltyContactFemToStatic::computeBilateralContactForces()
{
}
