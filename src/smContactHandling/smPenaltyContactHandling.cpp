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

#include "smCollision/smContactHandling.h"
#include "smSimulators/smVegaFemSceneObject.h"

smPenaltyCollisionHandling(bool typeBilateral)
{
	if (typeBilateral)
		contactHandlingType = SIMMEDTK_CONTACT_PENALTY_UNILATERAL;
	else
		contactHandlingType = SIMMEDTK_CONTACT_PENALTY_BILATERAL;
}

smPenaltyCollisionHandling(smContactHandlingType contactHandlingType,
                             const std::shared_ptr<smSceneObject>& sceneObjFirst,
                             const std::shared_ptr<smSceneObject>& sceneObjSecond)
{
	if (typeBilateral)
		contactHandlingType = SIMMEDTK_CONTACT_PENALTY_UNILATERAL;
	else
		contactHandlingType = SIMMEDTK_CONTACT_PENALTY_BILATERAL;

	this->collidingSceneObjects.first = sceneObjFirst;
	this->collidingSceneObjects.second = sceneObjSecond;
}

smContactHandlingType smPenaltyCollisionHandling::getContactHandlingType()
{
    return contactHandlingType;
}

void smPenaltyCollisionHandling::processContacts()
{
	if (typeBilateral)
	{
		computeUnilateralContactForces();
	}
	else
	{
		computeBilateralContactForces();
	}
}

void smPenaltyCollisionHandling::computeUnilateralContactForces()
{
	int penetratedNode, nodeDofID;
	const double stiffness=1.0e5, damping=2000.0;
	smVec3d velocityProjection;

	std::vector<std::shared_ptr<smContact>> contactInfo = this->getCollisionPairs()->getContacts();

	if( this->getFirstSceneObject()->getType() == SIMMEDTK_SMVEGAFEMSCENEOBJECT 
		&& this->getSecondSceneObject()->getType() == SIMMEDTK_SMSTATICSCENEOBJECT)
	{
		auto femSceneObject = std::static_pointer_cast<smVegaFemSceneObject>(this->getFirstSceneObject());

		femSceneObject->setContactForcesToZero();
		
		for (int i = 0; i < this->getCollisionPairs()->size(); i++)
		{
			nodeDofID = 3*contactInfo->nodeIndex;

			velocityProjection = smVec3d(femSceneObject->uvel[nodeDofID], femSceneObject->uvel[nodeDofID+1], femSceneObject->uvel[nodeDofID+2]);
			velocityProjection = contactInfo->normal*velocityProjection.transpose();

			femSceneObject->f_contact(nodeDofID    ) += stiffness*contactInfo->depth*contactInfo->normal - damping*velocityProjection(0,0);
			femSceneObject->f_contact(nodeDofID + 1) += stiffness*contactInfo->depth*contactInfo->normal - damping*velocityProjection(0,1);
			femSceneObject->f_contact(nodeDofID + 2) += stiffness*contactInfo->depth*contactInfo->normal - damping*velocityProjection(0,2);

		}

		femSceneObject->applyContactForces();
	}
		
}

void smPenaltyCollisionHandling::computeBilateralContactForces()
{


}