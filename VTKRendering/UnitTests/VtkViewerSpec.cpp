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

#include <memory>

// SimMedTK includes
#include "Geometry/MeshModel.h"
#include "Core/RenderDetail.h"
#include "Core/StaticSceneObject.h"
#include "Core/Scene.h"
#include "VTKRendering/VTKViewer.h"
#include "Testing/TestMesh.h"
#include "IO/IOMesh.h"

#include "IO/InitIO.h"
#include "VTKRendering/InitVTKRendering.h"

#include <bandit/bandit.h>
using namespace bandit;

go_bandit([](){
    InitVTKRendering();
    InitIODelegates();
    describe("VTK based viewer.", []() {
        it("constructs", []() {
            auto viewer = std::make_shared<VTKViewer>();
            AssertThat(viewer != nullptr, IsTrue());
        });

        it("initializes rendering pipeline", []() {
            auto viewer = std::make_shared<VTKViewer>();
            AssertThat(viewer->isValid(), IsTrue());
        });

//         it("renders mesh", []() {
//             auto io = std::make_shared<IOMesh>();
//             io->read("/home/rortiz/tmp/CollisionHash_resources/models/liver.obj");
//
//             auto mesh = std::static_pointer_cast<SurfaceMesh>(io->getMesh());
//             auto viewer = std::make_shared<VTKViewer>();
//             TextureManager::addTexture("/home/rortiz/tmp/CollisionHash_resources/textures/voronoi.jpg","blood");
//             mesh->getRenderDetail()->renderType |= SIMMEDTK_RENDER_NORMALS;
//             mesh->assignTexture("blood");
//             viewer->addObject(mesh);
//
//         });

    });

});
