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
#include "VtkRendering/VtkViewer.h"
#include "Testing/TestMesh.h"

#include <bandit/bandit.h>
using namespace bandit;

go_bandit([](){

    describe("Vtk based viewer.", []() {
        it("constructs", []() {
            auto viewer = std::make_shared<VtkViewer>();
            AssertThat(viewer != nullptr, IsTrue());
        });

        it("initializes rendering pipeline", []() {
            auto viewer = std::make_shared<VtkViewer>();
            AssertThat(viewer->isValid(), IsTrue());
        });

        it("renders mesh", []() {
            auto mesh = makeSurfaceMesh();
//             mesh->getRenderDetail()->renderType |= SIMMEDTK_RENDER_NORMALS;
            auto viewer = std::make_shared<VtkViewer>();

//             auto scene = std::make_shared<Scene>();
//
//             auto cubeModel = std::make_shared<MeshModel>();
//             cubeModel->setModelMesh(makeSurfaceMesh());
//
//             auto renderDetail = std::make_shared<RenderDetail>(SIMMEDTK_RENDER_FACES | SIMMEDTK_RENDER_TEXTURE);
//             cubeModel->setRenderDetail(renderDetail);
//
//             auto cube = std::make_shared<StaticSceneObject>();
//             cube->setModel(cubeModel);
//
//             //Add the cube to the scene to be rendered
//             scene->addSceneObject(cube);

            mesh->assignTexture("/home/rortiz/tmp/CollisionHash_resources/textures/brick.jpg","cubeTex");
            viewer->addObject(mesh);

            viewer->exec();
        });

    });

});
