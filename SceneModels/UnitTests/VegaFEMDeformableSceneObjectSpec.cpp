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
#include <memory>
#include <iostream>

// iMSTK includes
#include "SceneModels/VegaFEMDeformableSceneObject.h"
#include "IO/InitIO.h"
#include "Testing/ReadPaths.h"

using namespace bandit;

go_bandit([]()
{
    imstk::InitIODelegates();
    auto paths = imstk::ReadPaths("./SceneModelsConfig.paths");

    describe("Vega Deformable Scene Object", [&]()
    {
        auto sceneObject = std::make_shared<imstk::VegaFEMDeformableSceneObject>(std::get<imstk::Path::Binary>(paths)+"/box.veg",
                                                                          std::get<imstk::Path::Binary>(paths)+"/box.config");
        it("constructs", [&]()
        {
            AssertThat(sceneObject != nullptr, IsTrue());
        });
        it("updates", [&]()
        {
            sceneObject->update(0.01);
            auto sum1 = sceneObject->getCurrentState()->getPositions().sum();
            auto sum2 = sceneObject->getCurrentState()->getVelocities().sum();
            bool validState = std::isfinite(sum1) && std::isfinite(sum2);
            AssertThat(validState, IsTrue());
        });
     });

});
