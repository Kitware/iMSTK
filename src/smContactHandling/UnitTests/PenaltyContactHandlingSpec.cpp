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

auto createStaticPlaneSceneObject()
{

	auto staticPlane = std::make_shared<smStaticSceneObject>();

    std::shared_ptr<smPlaneCollisionModel> plane = std::make_shared<smPlaneCollisionModel>( smVec3d(0.0, 0.0, 0.0), smVec3d(0.0, 0.0, 1.0) );

	staticPlane->setModel(plane);

    return staticPlane;
}


auto createSampleCollisionPair()
{

	auto collisionPair = std::make_shared<smCollisionPair>();

    float depth = 1.0;
    smVec3d contactPoint(0,0,1);
    smVec3d normal(1,0,0);

    collisionPair->addContact(depth,contactPoint,normal);

	collisionPair->getContacts()[0]->index = 0;

    return collisionPair;

}

go_bandit([](){
    describe("Penalty contact handling:", []() {

        auto handler        = std::make_shared<smPenaltyContactHandling>(false);
		auto fem            = std::make_shared<smVegaFemSceneObject>(nullptr, "__none");
        auto plane          = createStaticPlaneSceneObject();
        auto collisionPair  = createSampleCollisionPair();

		it("initializes properly ", [&]() {
			AssertThat(handler != nullptr, IsTrue());
			AssertThat(handler->getContactHandlingType() == SIMMEDTK_CONTACT_PENALTY_UNILATERAL, IsTrue());
        });

		it("attches a collision pair ", [&]() {
            handler->setCollisionPairs(collisionPair);
            AssertThat(handler->getCollisionPairs() == collisionPair, IsTrue());
         });

		it("attaches a scene object ", [&]() {
			handler->setSceneObjects(fem, plane);

            AssertThat(handler->getFirstSceneObject() == fem, IsTrue());
            AssertThat(handler->getSecondSceneObject() == plane, IsTrue());
            AssertThat(handler->getFirstSceneObject()->getType() == SIMMEDTK_SMVEGAFEMSCENEOBJECT, IsTrue());
            AssertThat(handler->getSecondSceneObject()->getType() == SIMMEDTK_SMSTATICSCENEOBJECT, IsTrue());

         });

		it("computes contact force ", [&]() {
            handler->setCollisionPairs(collisionPair);
            smVec3d v;
            v << 1, 1, 1;

			fem->uvel = v.data();

            smVec3d f;
            f << 0, 0, 0;
			fem->f_ext = f.data();

			fem->f_contact.resize(3);

			handler->setSceneObjects(fem, plane);

			handler->resolveContacts();

			AssertThat(fem->f_contact[0]==98000, IsTrue());
			AssertThat(fem->f_contact[1] == 0, IsTrue());
			AssertThat(fem->f_contact[2] == 0, IsTrue());

            fem->uvel = nullptr;
            fem->f_ext = nullptr;
		});
    });
});


