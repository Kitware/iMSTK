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

#include <bandit/bandit.h>

#include "Core/CollisionManager.h"
#include "Collision/MeshCollisionModel.h"
#include "Core/Model.h"
#include "Core/MakeUnique.h"

using namespace bandit;

go_bandit([](){
    describe("Collision pair", []() {
        it("constructs ", []() {
            auto collisionPair = imstk::make_unique<imstk::CollisionManager>();
            AssertThat(collisionPair != nullptr, IsTrue());
        });
        it("attaches models ", []() {
            auto collisionPair = imstk::make_unique<imstk::CollisionManager>();

            auto modelA = std::make_shared<imstk::MeshCollisionModel>();
            auto modelB = std::make_shared<imstk::MeshCollisionModel>();

            collisionPair->setModels(modelA,modelB);

            AssertThat(collisionPair->getFirst(), Equals(modelA));
            AssertThat(collisionPair->getSecond(), Equals(modelB));

        });
        it("attaches contacts ", []() {
            auto collisionPair = imstk::make_unique<imstk::CollisionManager>();

            float depth = 1.0;
            imstk::Vec3d contactPoint(0,0,1);
            imstk::Vec3d normal(1,0,0);
            collisionPair->addContact(depth,contactPoint,1,normal);

            auto contact = collisionPair->getContacts().back();

            AssertThat(collisionPair->hasContacts(), IsTrue());
            AssertThat(contact->normal, Equals(normal));
            AssertThat(contact->point, Equals(contactPoint));
            AssertThat(contact->depth, Equals(depth));

        });
    });

});

