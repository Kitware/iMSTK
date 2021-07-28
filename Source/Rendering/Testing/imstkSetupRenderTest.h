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
#include "imstkKeyboardSceneControl.h"
#include "imstkMouseSceneControl.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneObject.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

using namespace imstk;

namespace
{
void
run_for(SimulationManager* driver, int i)
{
    std::thread t(&SimulationManager::start, driver);

    std::this_thread::sleep_for(std::chrono::seconds(i));

    driver->requestStatus(ModuleDriverStopped);
    t.join();
}
}

class RenderTest : public testing::Test
{
public:

    void SetUp() override
    {
        scene  = std::make_shared<Scene>("Render Test Scene");
        viewer = std::make_shared<VTKViewer>("Viewer");
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene in its own thread
        sceneManager = std::make_shared<SceneManager>("Scene Manager");
        sceneManager->setActiveScene(scene);

        driver = std::make_shared<SimulationManager>();
        driver->addModule(viewer);
        driver->addModule(sceneManager);

        mouseControl = std::make_shared<MouseSceneControl>(viewer->getMouseDevice());
        mouseControl->setSceneManager(sceneManager);
        viewer->addControl(mouseControl);

        keyControl = std::make_shared<KeyboardSceneControl>(viewer->getKeyboardDevice());
        keyControl->setSceneManager(sceneManager);
        keyControl->setModuleDriver(driver);
        viewer->addControl(keyControl);

        driver->requestStatus(ModuleDriverRunning);
    }

    void addGeometry(std::shared_ptr<Geometry> geom, Color color)
    {
      scene->getActiveCamera()->setPosition(Vec3d(3.0, 5.0, 3.0));
      auto light = std::make_shared<DirectionalLight>();
      light->setDirection(Vec3d(5.0, -8.0, -5.0));
      light->setIntensity(1.0);
      scene->addLight("light", light);

      geom->scale(0.5, Geometry::TransformType::ConcatenateToTransform);
      geom->rotate(Vec3d(0.0, 1.0, 0), PI_2, Geometry::TransformType::ConcatenateToTransform);
      geom->translate(Vec3d(0.0, 0.0, 0.0), Geometry::TransformType::ConcatenateToTransform);

      auto renderMaterial = std::make_shared<RenderMaterial>();
      renderMaterial->setColor(color);
      renderMaterial->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
      renderMaterial->setPointSize(6.0);
      renderMaterial->setLineWidth(4.0);

      auto visualModel = std::make_shared<VisualModel>(geom);
      visualModel->setRenderMaterial(renderMaterial);

      auto sceneObj = std::make_shared<SceneObject>("SceneObject");
      sceneObj->addVisualModel(visualModel);
      scene->addSceneObject(sceneObj);
    }

    std::shared_ptr<Scene>                scene;
    std::shared_ptr<VTKViewer>            viewer;
    std::shared_ptr<SceneManager>         sceneManager;
    std::shared_ptr<SimulationManager>    driver;
    std::shared_ptr<MouseSceneControl>    mouseControl;
    std::shared_ptr<KeyboardSceneControl> keyControl;
};