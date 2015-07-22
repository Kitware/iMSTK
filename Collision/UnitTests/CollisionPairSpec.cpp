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

#include "Core/CollisionPair.h"
#include "Collision/MeshCollisionModel.h"
#include "Core/ModelRepresentation.h"
#include "Core/MakeUnique.h"

using namespace bandit;

go_bandit([](){
    describe("Collision pair", []() {
        it("constructs ", []() {
            std::unique_ptr<CollisionPair> collisionPair(make_unique<CollisionPair>());
        });
        it("attaches models ", []() {
            std::unique_ptr<CollisionPair> collisionPair(make_unique<CollisionPair>());

            std::shared_ptr<ModelRepresentation> modelA = std::make_shared<MeshCollisionModel>();
            std::shared_ptr<ModelRepresentation> modelB = std::make_shared<MeshCollisionModel>();

            collisionPair->setModels(modelA,modelB);

            AssertThat(collisionPair->getFirst(), Equals(modelA));
            AssertThat(collisionPair->getSecond(), Equals(modelB));

        });
        it("attaches contacts ", []() {
            std::unique_ptr<CollisionPair> collisionPair(make_unique<CollisionPair>());

            float depth = 1.0;
            core::Vec3d contactPoint(0,0,1);
            core::Vec3d normal(1,0,0);
            collisionPair->addContact(depth,contactPoint,1,normal);

            std::shared_ptr<Contact> contact = collisionPair->getContacts().back();

            AssertThat(collisionPair->hasContacts(), IsTrue());
            AssertThat(contact->normal, Equals(normal));
            AssertThat(contact->point, Equals(contactPoint));
            AssertThat(contact->depth, Equals(depth));

        });
    });

});

