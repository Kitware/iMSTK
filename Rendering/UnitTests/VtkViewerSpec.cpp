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

#include <memory>

// iMSTK includes
#include "Geometry/MeshModel.h"
#include "Core/RenderDetail.h"
#include "SceneModels/StaticSceneObject.h"
#include "Core/Scene.h"
#include "Rendering/VTKViewer.h"
#include "Testing/TestMesh.h"
#include "IO/IOMesh.h"

#include "IO/InitIO.h"
#include "Rendering/InitVTKRendering.h"

#include <bandit/bandit.h>
using namespace bandit;

go_bandit([](){
    imstk::InitVTKRendering();
    imstk::InitIODelegates();
    describe("VTK based viewer.", []() {
        it("constructs", []() {
            auto viewer = std::make_shared<imstk::VTKViewer>();
            AssertThat(viewer != nullptr, IsTrue());
        });

        it("initializes rendering pipeline", []() {
            auto viewer = std::make_shared<imstk::VTKViewer>();
            AssertThat(viewer->isValid(), IsTrue());
        });


    });

});
