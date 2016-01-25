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

#include "Collision/SpatialHashCollision.h"
#include "Core/ErrorLog.h"
#include "Core/MakeUnique.h"

using namespace bandit;

go_bandit([](){
    describe("the spatial hash collision detector", []() {
        it("initializes properly ", []() {

            auto spatialHash = imstk::make_unique<imstk::SpatialHashCollision>(10,1.0,1.0,1.0);
            AssertThat(spatialHash != nullptr, IsTrue());
        });
    });

});

