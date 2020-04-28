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

#include "imstkSimulationManager.h"
#include "imstkSceneObject.h"
#include "imstkLight.h"
#include "imstkCamera.h"
#include "imstkSceneManager.h"
#include "imstkCollisionGraph.h"
#include "imstkDebugRenderGeometry.h"
#include "imstkAPIUtilities.h"
#include "imstkVTKViewer.h"
#include "imstkVTKRenderer.h"
#include "imstkVTKTextStatusManager.h"
#include "imstkScene.h"

#include <thread>
#include <chrono>

using namespace imstk;

std::shared_ptr<DebugRenderPoints>
addPointsDebugRendering(const std::shared_ptr<Scene>& scene)
{
    auto debugPoints = std::make_shared<DebugRenderPoints>("Debug Points");
    auto material    = std::make_shared<RenderMaterial>();
    material->setDebugColor(Color::Blue);
    auto vizModel = std::make_shared<VisualModel>(debugPoints, material);
    scene->addDebugVisualModel(vizModel);

    return debugPoints;
}

std::shared_ptr<DebugRenderLines>
addLinesDebugRendering(const std::shared_ptr<Scene>& scene)
{
    auto debugLines = std::make_shared<DebugRenderLines>("Debug Lines");
    auto material   = std::make_shared<RenderMaterial>();
    material->setBackFaceCulling(false);
    material->setDebugColor(Color::Green);
    material->setLineWidth(2.0);
    auto vizModel = std::make_shared<VisualModel>(debugLines, material);
    scene->addDebugVisualModel(vizModel);
    
    return debugLines;
}

std::shared_ptr<DebugRenderTriangles>
addTrianglesDebugRendering(const std::shared_ptr<Scene>& scene)
{
    auto debugTriangles = std::make_shared<DebugRenderTriangles>("Debug Triangles");
    auto material       = std::make_shared<RenderMaterial>();
    material->setBackFaceCulling(false);
    material->setDebugColor(Color::Red);
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    auto vizModel = std::make_shared<VisualModel>(debugTriangles, material);
    
    return debugTriangles;
}

Vec3d
getRandomPositions(double radius)
{
    return radius * Vec3d(2.0 * static_cast<double>(rand()) / static_cast<double>(RAND_MAX) - 1.0,
                          2.0 * static_cast<double>(rand()) / static_cast<double>(RAND_MAX) - 1.0,
                          2.0 * static_cast<double>(rand()) / static_cast<double>(RAND_MAX) - 1.0);
}

///
/// \brief This example demonstrates debug rendering in iMSTK
///
int
main()
{
    // simManager and Scene
    auto simManager = std::make_shared<SimulationManager>();
    auto scene      = simManager->createNewScene("Debug rendering example");
    simManager->setActiveScene(scene);

    // Get the VTKViewer
    auto viewer = std::dynamic_pointer_cast<VTKViewer>(simManager->getViewer());
    viewer->getVtkRenderWindow()->SetSize(1920, 1080);

    auto statusManager = viewer->getTextStatusManager();
    statusManager->setStatusFontSize(VTKTextStatusManager::Custom, 30);
    statusManager->setStatusFontColor(VTKTextStatusManager::Custom, Color::Orange);

    // Get VTK Renderer
    auto renderer = std::dynamic_pointer_cast<VTKRenderer>(viewer->getActiveRenderer());
    LOG_IF(FATAL, (!renderer)) << "Invalid renderer: Only VTKRenderer is supported for debug rendering";

    auto debugPoints    = addPointsDebugRendering(scene);
    auto debugLines     = addLinesDebugRendering(scene);
    auto debugTriangles = addTrianglesDebugRendering(scene);

    int mode  = 0; // 0: add point, 1: add line, 2: add triangle
    int count = 0; // The number of times cycling between modes

    auto updateFunc =
        [&](Module*) {
            if (count > 15)
            {
                count = 0;
                debugPoints->clear();
                debugLines->clear();
                debugTriangles->clear();
            }

            if (mode % 3 == 0)
            {
                debugPoints->appendVertex(getRandomPositions(15.0));
            }
            else if (mode % 3 == 1)
            {
                auto p     = getRandomPositions(50.0);
                auto shift = getRandomPositions(1.0);
                debugLines->appendVertex(p + shift);
                debugLines->appendVertex(-p + shift);
            }
            else
            {
                auto shift = getRandomPositions(10.0);
                debugTriangles->appendVertex(getRandomPositions(5.0) + shift);
                debugTriangles->appendVertex(getRandomPositions(5.0) + shift);
                debugTriangles->appendVertex(getRandomPositions(5.0) + shift);

                mode = -1;
                ++count;
            }
            ++mode;

            debugPoints->setDataModified(true);
            debugLines->setDataModified(true);
            debugTriangles->setDataModified(true);

            // Must call to update render data
            for (auto& delegate : renderer->getDebugRenderDelegates())
            {
                delegate->updateDataSource();
            }

            statusManager->setCustomStatus("Primatives: " +
                           std::to_string(debugPoints->getNumVertices()) + " (points) | " +
                           std::to_string(debugLines->getNumVertices() / 2) + " (lines) | " +
                           std::to_string(debugTriangles->getNumVertices() / 3) + " (triangles)"
                );

            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        };
    simManager->getSceneManager(scene)->setPostUpdateCallback(updateFunc);

    // Set Camera configuration
    auto cam = scene->getCamera();
    cam->setPosition(Vec3d(0, 0, 50));
    cam->setFocalPoint(Vec3d(0, 0, 0));

    // Light
    {
        auto light = std::make_shared<DirectionalLight>("Light 1");
        light->setFocalPoint(Vec3d(-1, -1, -1));
        light->setIntensity(1);
        scene->addLight(light);
    }
    {
        auto light = std::make_shared<DirectionalLight>("Light 2");
        light->setFocalPoint(Vec3d(1, -1, -1));
        light->setIntensity(1);
        scene->addLight(light);
    }

    // Run
    simManager->start();

    return 0;
}
