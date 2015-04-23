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

#include "smEvent/smEventHandler.h"

using namespace bandit;

go_bandit([](){
    describe("Event handler", []() {
        it("constructs ", []() {
            std::unique_ptr<smEventHandler2> smEventHandler(make_unique<smEventHandler2>());
        });
        it("registers events ", []() {
            std::unique_ptr<smEventHandler2> smEventHandler(make_unique<smEventHandler2>());

            smEventHandler->attachEvent()
            std::shared_ptr<smModelRepresentation> modelA = std::make_shared<smMeshCollisionModel>();
            std::shared_ptr<smModelRepresentation> modelB = std::make_shared<smMeshCollisionModel>();

            collisionPair->setModels(modelA,modelB);

            AssertThat(collisionPair->getFirst(), Equals(modelA));
            AssertThat(collisionPair->getSecond(), Equals(modelB));

        });

    });

});

