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

// SimMedTK includes
#include "Mesh/VegaVolumetricMesh.h"
#include "Mesh/SurfaceMesh.h"
#include "IO/IOMesh.h"
#include "IO/InitIO.h"
#include "Testing/ReadPaths.h"

using namespace bandit;

go_bandit([](){
    InitIODelegates();
    auto paths = imstk::ReadPaths("./MeshConfig.paths");

    describe("Vega Volumetric Mesh", [&]() {
        auto ioMesh = std::make_shared<IOMesh>();
        ioMesh->read(std::get<imstk::Path::Source>(paths)+"/SampleMesh.veg");
        auto vegaMesh = std::static_pointer_cast<VegaVolumetricMesh>(ioMesh->getMesh());

        it("constructs", []() {
            std::shared_ptr<VegaVolumetricMesh> vegaMesh = std::make_shared<VegaVolumetricMesh>();
            AssertThat(vegaMesh != nullptr, IsTrue());
        });
        it("generates graph", [&]() {

            AssertThat(vegaMesh->getMeshGraph() != nullptr, IsTrue());
        });
        it("attaches surface mesh", [&]() {
            std::shared_ptr<SurfaceMesh> surfaceMesh = std::make_shared<SurfaceMesh>();

            // These vertices coincide with vertices on the volume mesh. So the weights take a special form.
            auto &vertices = surfaceMesh->getVertices();
            vertices.emplace_back(-2.44627, -0.903874999999999, -1.711465);
            vertices.emplace_back(-2.008655, -0.762779999999999, -1.63081);
            vertices.emplace_back(-2.248035, -0.599385, -1.41836);

            vegaMesh->attachSurfaceMesh(surfaceMesh,2.0);

            auto &weights = vegaMesh->getAttachedWeights(surfaceMesh);
            AssertThat(weights[0] == 1 && weights[5] == 1 && weights[10] == 1, IsTrue());
        });
    });
});
