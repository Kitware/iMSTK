/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

=========================================================================*/

#pragma once

#include "gtest/gtest.h"

#include <memory>

#include "imstkCamera.h"
#include "imstkDirectionalLight.h"
#include "imstkGeometry.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneObject.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

using namespace imstk;

class RenderTest : public testing::Test
{
public:
    void runFor(const int seconds);
    void runAllMaterials();

protected:
    void SetUp() override;

    virtual void createGeometry() = 0;

    void updateMaterial();

    void applyColor();

    // Render Frame
    std::shared_ptr<Scene>     scene;
    std::shared_ptr<VTKViewer> viewer;
    std::shared_ptr<DirectionalLight> light;

    // Render Contents
    std::shared_ptr<Geometry>       geom;
    std::shared_ptr<RenderMaterial> renderMaterial;
    std::shared_ptr<VisualModel>    visualModel;
    std::shared_ptr<SceneObject>    sceneObj;

    double      elapsedTime = 0;
    bool        complete    = false;
    int         displayMode;
    int         color;
    int         shadingModel;
    int         blendMode;
    std::string dm, c, sm, bm;
};