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

#include "ContactHandling/PenaltyContactFemToStatic.h"
#include "Collision/PlaneCollisionModel.h"
#include "Core/CollisionManager.h"
#include "SceneModels/StaticSceneObject.h"
#include "SceneModels/VegaFEMDeformableSceneObject.h"

using namespace bandit;

std::shared_ptr<StaticSceneObject> createStaticPlaneSceneObject()
{
    auto staticPlane = std::make_shared<StaticSceneObject>();

    std::shared_ptr<PlaneCollisionModel> plane = std::make_shared<PlaneCollisionModel>( core::Vec3d(0.0, 0.0, 0.0), core::Vec3d(0.0, 0.0, 1.0) );

    staticPlane->setModel(plane);

    return staticPlane;
}


std::shared_ptr<CollisionManager> createSampleCollisionPair()
{
    auto collisionPair = std::make_shared<CollisionManager>();

    float depth = 1.0;
    core::Vec3d contactPoint(0,0,1);
    core::Vec3d normal(1,0,0);

    collisionPair->addContact(depth,contactPoint,1,normal);

    collisionPair->getContacts()[0]->index = 0;

    return collisionPair;
}

go_bandit([]() {
    describe("Penalty contact handling:", []() {


        it("initializes properly ", []() {
            auto handler        = std::make_shared<PenaltyContactFemToStatic>(false);
            AssertThat(handler != nullptr, IsTrue());
            AssertThat(handler->getContactHandlingType() == ContactHandling::PenaltyFemToStatic, IsTrue());
        });

        it("attaches a collision pair ", []() {
            auto handler        = std::make_shared<PenaltyContactFemToStatic>(false);
            auto collisionPair  = createSampleCollisionPair();
            handler->setCollisionPairs(collisionPair);
            AssertThat(handler->getCollisionPairs() == collisionPair, IsTrue());
        });

        it("attaches a scene object ", []() {
            auto handler        = std::make_shared<PenaltyContactFemToStatic>(false);
            auto fem            = std::make_shared<VegaFEMDeformableSceneObject>();
            auto plane          = createStaticPlaneSceneObject();
            handler->setSceneObjects(plane,fem);
            AssertThat(handler->getFirstSceneObject() == plane, IsTrue());
            AssertThat(handler->getSecondSceneObject() == fem, IsTrue());
            AssertThat(handler->getFirstSceneObject()->getType() == core::ClassType::StaticSceneObject, IsTrue());
            AssertThat(handler->getSecondSceneObject()->getType() == core::ClassType::VegaFemSceneObject, IsTrue());
        });

        it("computes contact force ", []() {
            auto handler        = std::make_shared<PenaltyContactFemToStatic>(false);
            auto fem            = std::make_shared<VegaFEMDeformableSceneObject>();
            auto collisionPair  = createSampleCollisionPair();
            auto plane          = createStaticPlaneSceneObject();

            handler->setSceneObjects(plane,fem);
            handler->setCollisionPairs(collisionPair);

            auto state = fem->getCurrentState();
            state = std::make_shared<OdeSystemState>();
            state->resize(3);

            auto &v = state->getVelocities();
            v(0) = 1;
            v(1) = 1;
            v(2) = 1;

            // TODO: Add a more rigorous test.
            fem->setContactForce(0,core::Vec3d(-110000,0,0));
            auto &contactForce = fem->getContactForces();

            handler->resolveContacts();

            AssertThat(contactForce[0][0]==-110000, IsTrue());
            AssertThat(contactForce[0][1] == 0, IsTrue());
            AssertThat(contactForce[0][2] == 0, IsTrue());
        });
    });
});


