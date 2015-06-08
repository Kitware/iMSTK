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

#include "smContactHandling/smPenaltyContactHandling.h"
#include "smSimulators/smVegaFemSceneObject.h"

smPenaltyContactHandling::smPenaltyContactHandling( bool typeBilateral )
{
    if( typeBilateral )
    {
        contactHandlingType = SIMMEDTK_CONTACT_PENALTY_BILATERAL;
    }
    else
    {
        contactHandlingType = SIMMEDTK_CONTACT_PENALTY_UNILATERAL;
    }
}

smPenaltyContactHandling::smPenaltyContactHandling( bool typeBilateral,
        const std::shared_ptr<smSceneObject>& sceneObjFirst,
        const std::shared_ptr<smSceneObject>& sceneObjSecond )
{
    if( typeBilateral )
    {
        contactHandlingType = SIMMEDTK_CONTACT_PENALTY_UNILATERAL;
    }
    else
    {
        contactHandlingType = SIMMEDTK_CONTACT_PENALTY_BILATERAL;
    }

    this->collidingSceneObjects.first = sceneObjFirst;
    this->collidingSceneObjects.second = sceneObjSecond;
}

const smContactHandlingType &smPenaltyContactHandling::getContactHandlingType() const
{
    return contactHandlingType;
}

void smPenaltyContactHandling::resolveContacts()
{
    if( contactHandlingType == SIMMEDTK_CONTACT_PENALTY_UNILATERAL )
    {
        computeUnilateralContactForces();
    }
    else
    {
        computeBilateralContactForces();
    }
}

void smPenaltyContactHandling::computeUnilateralContactForces()
{
    int penetratedNode, nodeDofID;
    const double stiffness = 1.0e5, damping = 2000.0;
    smVec3d velocityProjection;

    std::vector<std::shared_ptr<smContact>> contactInfo = this->getCollisionPairs()->getContacts();

    std::cout << "# contacts: " << contactInfo.size() << std::endl;

    if( this->getFirstSceneObject()->getType() == SIMMEDTK_SMVEGAFEMSCENEOBJECT
            && this->getSecondSceneObject()->getType() == SIMMEDTK_SMSTATICSCENEOBJECT )
    {
        auto femSceneObject = std::static_pointer_cast<smVegaFemSceneObject>( this->getFirstSceneObject() );

        femSceneObject->setContactForcesToZero();
        smVec3d force;
        for( int i = 0; i < contactInfo.size(); i++ )
        {
            nodeDofID = 3 * contactInfo[i]->index;

            velocityProjection = smVec3d( femSceneObject->uvel[nodeDofID], femSceneObject->uvel[nodeDofID + 1], femSceneObject->uvel[nodeDofID + 2] );
            velocityProjection = contactInfo[i]->normal.dot( velocityProjection ) * contactInfo[i]->normal;

            force = stiffness * contactInfo[i]->depth * contactInfo[i]->normal - damping * velocityProjection;

            femSceneObject->f_contact[nodeDofID] += force( 0 );
            femSceneObject->f_contact[nodeDofID + 1] += force( 1 );
            femSceneObject->f_contact[nodeDofID + 2] += force( 2 );

        }

        femSceneObject->applyContactForces();
    }

}

void smPenaltyContactHandling::computeBilateralContactForces()
{


}
