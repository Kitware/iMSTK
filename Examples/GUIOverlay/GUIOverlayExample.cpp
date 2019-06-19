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
#include "imstkAPIUtilities.h"
#include "imstkGUICanvas.h"
#include "imstkGUIWindow.h"
#include "imstkGUIShapes.h"
#include "imstkGUIText.h"

using namespace imstk;

///
/// \brief This example demonstrates the GUI feature.
/// NOTE: Requires enabling Vulkan rendering backend
///
int main()
{
    // SDK and Scene
    auto sdk = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("GraphicalUserInterface");
    auto canvas = sdk->getViewer()->getCanvas();

    // Cube
    auto cube = apiutils::createVisualAnalyticalSceneObject(Geometry::Type::Cube, scene, "cube", 0.5);
    auto sphere = std::make_shared<Sphere>();
    cube->getVisualModel(0)->getGeometry()->translate(0, 0, 0);
    cube->getVisualModel(0)->getGeometry()->rotate(UP_VECTOR, PI_4);

    // Plane
    auto plane = apiutils::createVisualAnalyticalSceneObject(Geometry::Type::Plane, scene, "plane", 10);

    // Window logic
    auto window = std::make_shared<GUIOverlay::Window>("Test Window", "Test Window", 200.0f, 100.0f, 200.0f, 0.0f);
    auto windowText = std::make_shared<GUIOverlay::Text>("Window Text", "Window Text", 0.0f, 0.0f);
    window->addWidget(windowText);

    auto circle = std::make_shared<GUIOverlay::Circle>("Circle", 100.0f, 100.0f, 25.0f, Color::Red, true);
    auto circleOutline = std::make_shared<GUIOverlay::Circle>("CircleOutline", 100.0f, 100.0f, 25.0f, Color::Black, false);

    // These two rectangles show that the ordering matters
    auto underRectangle = std::make_shared<GUIOverlay::Rectangle>("UnderRectangle", 100.0f, 100.0f, 100.0f, 100.0f, Color(0.0f, 0.0f, 1.0f, 0.7f), true);
    auto overRectangle = std::make_shared<GUIOverlay::Rectangle>("OverRectangle", 0.0f, 0.0f, 100.0f, 100.0f, Color(0.7f, 0.0f, 1.0f, 0.7f), true);

    canvas->addWidget(underRectangle);
    canvas->addWidget(circle);
    canvas->addWidget(circleOutline);
    canvas->addWidget(overRectangle);
    canvas->addWidget(window);

    StopWatch watch;
    watch.start();
    auto startTime = watch.getTimeElapsed();
    auto lastTime = startTime;

    // Update function
    auto GUIUpdateFunction =
        [&](InteractorStyle* style) -> bool
        {
            auto time = watch.getTimeElapsed();
            float angle = time / 1000.0f;
            circle->setPosition(std::sin(angle) * 50.0f + 100.0f, std::cos(angle) * 50.0f + 100.0f);
            circle->setRadius(std::abs(std::sin(angle)) * 25.0f);
            circleOutline->setPosition(std::sin(angle) * 50.0f + 100.0f, std::cos(angle) * 50.0f + 100.0f);

            if (time - lastTime > 1000)
            {
                auto seconds = (int)((lastTime - startTime) / 1000);
                windowText->setText("Elapsed Time: " + std::to_string(seconds) + "s");
                lastTime = time;
            }

            return false;
        };
    sdk->getViewer()->setOnTimerFunction(GUIUpdateFunction);

    bool canvasVisible = true;
    auto hideShowCanvasFunction =
        [&](InteractorStyle* style) -> bool
        {
            if (canvasVisible)
            {
                canvas->hide();
            }
            else
            {
                canvas->show();
            }

            canvasVisible = !canvasVisible;
            return true;
        };
    sdk->getViewer()->setOnCharFunction('c', hideShowCanvasFunction);

    // Position camera
    auto cam = scene->getCamera();
    cam->setPosition(0, 3, 6);
    cam->setFocalPoint(0, 0, 0);

    // Light
    auto light = std::make_shared<DirectionalLight>("Light");
    light->setIntensity(7);
    light->setColor(Color(1.0, 0.95, 0.8));
    light->setFocalPoint(Vec3d(-1, -1, 0));
    scene->addLight(light);

    // Run
    sdk->setActiveScene(scene);
    sdk->startSimulation(SimulationStatus::PAUSED);

    return 0;
}
