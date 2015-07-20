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

#include "Core/ErrorLog.h"
#include "Collision/OctreeCell.h"

using namespace bandit;

go_bandit([](){
    describe("octree data structure", []() {
        OctreeCell treeCell;

        const int numberOfSubdivisions = OctreeCell::numberOfSubdivisions;

        core::Vec3d center(1.,1.,1.);
        treeCell.setCenter(center);

        float lenght = 14.5;
        treeCell.setLength(lenght);

        it("initializes properly", [&]() {
          AssertThat(numberOfSubdivisions, Equals(8));
          AssertThat(treeCell.isEmpty(), IsTrue());
          AssertThat(treeCell.getLevel(), Equals(0));
        });

        it("sets the center properly", [&]() {
          AssertThat(treeCell.getCenter()[0], Equals(1.));
          AssertThat(treeCell.getCenter()[1], Equals(1.));
          AssertThat(treeCell.getCenter()[2], Equals(1.));
        });

        it("sets the lenght properly", [&]() {
          AssertThat(treeCell.getLength(), Equals(14.5));
        });
    });
});


