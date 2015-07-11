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

#include "smCollision/smSpatialHashCollision.h"
#include "smCore/smErrorLog.h"
#include "smCore/smPipe.h"
#include "smCore/smMakeUnique.h"

using namespace bandit;

go_bandit([](){
    describe("the spatial hash collision detector", []() {
        it("initializes properly ", []() {

            std::unique_ptr<smSpatialHashCollision> spatialHash(make_unique<smSpatialHashCollision>(10,1.0,1.0,1.0));

//             AssertThat( spatialHash->pipe->getElements(), Equals( 1 ) );
//             AssertThat( spatialHash->pipeTriangles->getElements(), Equals( 1 ) );
//             AssertThat( spatialHash->pipeModelPoints->getElements(), Equals( 1 ) );
        });
    });

});

