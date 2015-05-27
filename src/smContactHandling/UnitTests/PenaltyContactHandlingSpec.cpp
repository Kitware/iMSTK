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

#include <bandit/bandit.h>

#include <memory>

#include "smContactHandling/smPenaltyContactHandling.h"
#include "smCollision/smPlaneCollisionModel.h"
#include "smCollision/smCollisionPair.h"
#include "smSimulators/smVegaFemSceneObject.h"
#include "smCore/smStaticSceneObject.h"

using namespace bandit;

std::shared_ptr<smStaticSceneObject> createStaticPlaneSceneObject()
{

	std::shared_ptr<smStaticSceneObject> staticPlane = std::make_shared<smStaticSceneObject>();

    std::shared_ptr<smPlaneCollisionModel> plane = std::make_shared<smPlaneCollisionModel>( smVec3d(0.0, 0.0, 0.0), smVec3d(0.0, 0.0, 1.0) );

	staticPlane->setModel(plane);

    return staticPlane;
}


std::shared_ptr<smCollisionPair> createSampleCollisionPair()
{

	std::shared_ptr<smCollisionPair> collisionPair = std::make_shared<smCollisionPair>();

    float depth = 1.0;
    smVec3d contactPoint(0,0,1);
    smVec3d normal(1,0,0);

    collisionPair->addContact(depth,contactPoint,normal);

	collisionPair->getContacts()[0]->index = 0;

    return collisionPair;

}

go_bandit([](){
    describe("\n\nPENALTY CONTACT HANDLING:", []() {

		it("checks initialization 1", []() {
			std::shared_ptr<smPenaltyContactHandling> handler = std::make_shared<smPenaltyContactHandling>(true);
			AssertThat(handler != nullptr, IsTrue());

        });

        it("checks initialization 2", []() {
            std::shared_ptr<smPenaltyContactHandling> handler = std::make_shared<smPenaltyContactHandling>(true);
			AssertThat(handler->getContactHandlingType() == SIMMEDTK_CONTACT_PENALTY_BILATERAL, IsTrue());

        });

		it("checks collision pairs ", []() {

            std::shared_ptr<smPenaltyContactHandling> handler = std::make_shared<smPenaltyContactHandling>(true);

            std::shared_ptr<smCollisionPair> collisionPair = createSampleCollisionPair();

            handler->setCollisionPairs(collisionPair);

            std::shared_ptr<smCollisionPair> cp = handler->getCollisionPairs();

			AssertThat(cp->hasContacts() == true, IsTrue());

			AssertThat(cp->getNumberOfContacts() == collisionPair->getNumberOfContacts(), IsTrue());

         });

		it("checks scene Objects ", []() {

            std::shared_ptr<smPenaltyContactHandling> penaltyCH = std::make_shared<smPenaltyContactHandling>(true);

			std::shared_ptr<smStaticSceneObject> planeSO = createStaticPlaneSceneObject();

			std::shared_ptr<smVegaFemSceneObject> femSO = std::make_shared<smVegaFemSceneObject>(nullptr, "__none");

			penaltyCH->setSceneObjects(femSO, planeSO);

			AssertThat(penaltyCH->getFirstSceneObject() != nullptr, IsTrue());

			AssertThat(penaltyCH->getSecondSceneObject() != nullptr, IsTrue());

            AssertThat(penaltyCH->getFirstSceneObject()->getType() == SIMMEDTK_SMVEGAFEMSCENEOBJECT, IsTrue());

            AssertThat(penaltyCH->getSecondSceneObject()->getType() == SIMMEDTK_SMSTATICSCENEOBJECT, IsTrue());

         });

		it("checks contact forces ", []() {

			std::shared_ptr<smPenaltyContactHandling> penaltyCH = std::make_shared<smPenaltyContactHandling>(false);

			std::shared_ptr<smStaticSceneObject> planeSO = createStaticPlaneSceneObject();

			std::shared_ptr<smVegaFemSceneObject> femSO = std::make_shared<smVegaFemSceneObject>(nullptr, "__none");
            std::shared_ptr<smCollisionPair> collisionPair = createSampleCollisionPair();

            penaltyCH->setCollisionPairs(collisionPair);
            smVec3d v;
            v << 1, 1, 1;

			femSO->uvel = v.data();

            smVec3d f;
            f << 0, 0, 0;
			femSO->f_ext = f.data();

			femSO->f_contact.resize(3);

			penaltyCH->setSceneObjects(femSO, planeSO);

			penaltyCH->resolveContacts();

			AssertThat(femSO->f_contact[0]==98000, IsTrue());
			AssertThat(femSO->f_contact[1] == 0, IsTrue());
			AssertThat(femSO->f_contact[2] == 0, IsTrue());

            femSO->uvel = nullptr;
            femSO->f_ext = nullptr;
		});
    });
});


