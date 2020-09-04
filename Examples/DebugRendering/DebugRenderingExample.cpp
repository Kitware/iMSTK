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

#include "imstkCamera.h"
#include "imstkDebugRenderGeometry.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLight.h"
#include "imstkLogger.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkVisualModel.h"
#include "imstkVTKRenderDelegate.h"
#include "imstkVTKRenderer.h"
#include "imstkVTKTextStatusManager.h"
#include "imstkVTKViewer.h"

using namespace imstk;

std::shared_ptr<DebugRenderPoints>
addPointsDebugRendering(const std::shared_ptr<Scene>& scene)
{
    imstkNew<DebugRenderPoints> debugPoints("Debug Points");
    imstkNew<RenderMaterial> material;
    material->setDisplayMode(RenderMaterial::DisplayMode::Points);
    material->setPointSize(6.);
    imstkNew<VisualModel> visualModel(debugPoints.get(), material);
    scene->addDebugVisualModel(visualModel);

    return debugPoints;
}

std::shared_ptr<DebugRenderLines>
addLinesDebugRendering(const std::shared_ptr<Scene>& scene)
{
    imstkNew<DebugRenderLines> debugLines("Debug Lines");
    imstkNew<RenderMaterial> material;
    material->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);
    //material->setBackFaceCulling(false);
    material->setEdgeColor(Color::Green);
    material->setLineWidth(4.0);
    imstkNew<VisualModel> visualModel(debugLines.get(), material);
    scene->addDebugVisualModel(visualModel);

    return debugLines;
}

std::shared_ptr<DebugRenderTriangles>
addTrianglesDebugRendering(const std::shared_ptr<Scene>& scene)
{
    imstkNew<DebugRenderTriangles> debugTriangles("Debug Triangles");
    imstkNew<RenderMaterial> material;
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    material->setBackFaceCulling(false);
    material->setColor(Color::Red);
    imstkNew<VisualModel> visualModel(debugTriangles.get(), material);
    scene->addDebugVisualModel(visualModel);

    return debugTriangles;
}

Vec3d
getRandomPositions(const double radius)
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
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Create a scene
    imstkNew<Scene> scene("Debug rendering example");

    // Setup a viewer to render in its own thread
    imstkNew<VTKViewer> viewer("Viewer");
    viewer->setActiveScene(scene);
    viewer->setWindowTitle("Debug Rendering");
    viewer->setSize(1920, 1080);

    auto statusManager = viewer->getTextStatusManager();
    statusManager->setStatusFontSize(VTKTextStatusManager::StatusType::Custom, 30);
    statusManager->setStatusFontColor(VTKTextStatusManager::StatusType::Custom, Color::Orange);

    // Get VTK Renderer
    auto renderer = std::dynamic_pointer_cast<VTKRenderer>(viewer->getActiveRenderer());
    LOG_IF(FATAL, (!renderer)) << "Invalid renderer: Only VTKRenderer is supported for debug rendering";

    auto debugPoints    = addPointsDebugRendering(scene);
    auto debugLines     = addLinesDebugRendering(scene);
    auto debugTriangles = addTrianglesDebugRendering(scene);

    int mode  = 0; // 0: add point, 1: add line, 2: add triangle
    int count = 0; // The number of times cycling between modes

    auto updateFunc =
        [&](Event*)
        {
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

    // Set Camera configuration
    scene->getActiveCamera()->setPosition(Vec3d(0.0, 0.0, 50.0));

    // Light
    imstkNew<DirectionalLight> light1("light1");
    light1->setFocalPoint(Vec3d(-1.0, -1.0, -1.0));
    light1->setIntensity(1.0);
    scene->addLight(light1);

    imstkNew<DirectionalLight> light2("light2");
    light2->setFocalPoint(Vec3d(1.0, -1.0, -1.0));
    light2->setIntensity(1.0);
    scene->addLight(light2);

    // Run the simulation
    {
        // Setup a scene manager to advance the scene in its own thread
        imstkNew<SceneManager> sceneManager("Scene Manager");
        sceneManager->setActiveScene(scene);
        viewer->addChildThread(sceneManager); // SceneManager will start/stop with viewer
        connect<Event>(sceneManager, EventType::PostUpdate, updateFunc);

        // Add mouse and keyboard controls to the viewer
        {
            imstkNew<MouseSceneControl> mouseControl(viewer->getMouseDevice());
            mouseControl->setSceneManager(sceneManager);
            viewer->addControl(mouseControl);

            imstkNew<KeyboardSceneControl> keyControl(viewer->getKeyboardDevice());
            keyControl->setSceneManager(sceneManager);
            keyControl->setViewer(viewer);
            viewer->addControl(keyControl);
        }

        viewer->start();
    }

    return 0;
}
