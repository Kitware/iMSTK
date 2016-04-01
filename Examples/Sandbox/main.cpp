#include <cstring>
#include <iostream>
#include <memory>
#include <thread>
#include <iomanip>

#include "imstkSceneObject.h"
#include "imstkPlane.h"
#include "imstkSphere.h"
#include "imstkCube.h"
#include "imstkSimulationManager.h"

int main()
{
    std::cout << "****************" << std::endl
              << "Starting Sandbox" << std::endl
              << "****************" << std::endl;

    // --------------------------------------------
    // Rendering
    // --------------------------------------------
    auto sdk = std::make_shared<imstk::SimulationManager>();

    auto planeGeom = std::make_shared<imstk::Plane>();

    planeGeom->scale(5);
    auto planeObj = std::make_shared<imstk::VisualObject>("VisualPlane");
    planeObj->setVisualGeometry(planeGeom);

    auto cubeGeom = std::make_shared<imstk::Cube>();
    cubeGeom->scale(0.5);
    cubeGeom->rotate(imstk::UP,    M_PI/4);
    cubeGeom->rotate(imstk::RIGHT, M_PI/4);
    cubeGeom->translate(2, 1, 0.5);
    auto cubeObj = std::make_shared<imstk::VisualObject>("VisualCube");
    cubeObj->setVisualGeometry(cubeGeom);

    auto sphereGeom = std::make_shared<imstk::Sphere>();
    sphereGeom->scale(0.3);
    sphereGeom->translate(-0.75, 1.5, -0.5);
    auto sphereObj = std::make_shared<imstk::VisualObject>("VisualSphere");
    sphereObj->setVisualGeometry(sphereGeom);

    auto sceneTest = sdk->createNewScene("SceneTest");
    sceneTest->setLoopDelay(1000);
    sceneTest->addSceneObject(planeObj);
    sceneTest->addSceneObject(cubeObj);
    sceneTest->addSceneObject(sphereObj);

    sdk->startSimulation("SceneTest");
    std::this_thread::sleep_for(std::chrono::seconds(10));
    sdk->pauseSimulation();
    std::this_thread::sleep_for(std::chrono::seconds(5));
    sdk->runSimulation();
    std::this_thread::sleep_for(std::chrono::seconds(10));
    sdk->endSimulation();

    /*
        //--------------------------------------------
        // Geometry
        //--------------------------------------------
        std::cout << std::fixed << std::setprecision(2);

        // Plane
        std::cout << "-- Plane : Init" << std::endl;
        auto pos   = imstk::Vec3d(5, 2, 5);
        auto norm  = imstk::Vec3d(0, 1, 1);
        auto width = 10;
        std::cout << "p = " << pos << std::endl;
        std::cout << "n = " << norm << std::endl;
        std::cout << "w = " << width << std::endl;

        std::cout << "-- Plane : Create" << std::endl;
        auto plane = std::make_shared<imstk::Plane>(pos, norm, width);
        std::cout << "p = " << plane->getPosition() << std::endl;
        std::cout << "n = " << plane->getNormal() << std::endl;
        std::cout << "w = " << plane->getWidth() << std::endl;

        std::cout << "-- Plane : Set Position" << std::endl;
        plane->setPosition(imstk::Vec3d(1, 1, 1));
        std::cout << "p = " << plane->getPosition() << std::endl;

        std::cout << "-- Plane : Translate" << std::endl;
        plane->translate(imstk::Vec3d(2, 1, -3));
        std::cout << "p = " << plane->getPosition() << std::endl;

        std::cout << "-- Plane : Set Normal" << std::endl;
        plane->setNormal(imstk::FORWARD);
        std::cout << "n = " << plane->getNormal() << std::endl;

        std::cout << "-- Plane : Rotate" << std::endl;
        plane->rotate(imstk::UP, M_PI / 2);
        std::cout << "n = " << plane->getNormal() << std::endl;
     */

    /*
        //--------------------------------------------
        // SimulationManager
        //--------------------------------------------
        std::shared_ptr<imstk::SimulationManager> sdk =
                std::make_shared<imstk::SimulationManager>();

        // Scenes
        LOG(INFO) << "-- Test add scenes";

        std::shared_ptr<imstk::Scene> scene1 =
        std::make_shared<imstk::Scene>("scene1");
        scene1->setLoopDelay(500);
        sdk->addScene(scene1);

        sdk->createNewScene("scene2");
        std::shared_ptr<imstk::Scene> scene2 = sdk->getScene("scene2");
        scene2->setLoopDelay(500);

        std::shared_ptr<imstk::Scene> scene3 = sdk->createNewScene();
        sdk->removeScene("Scene_3");

        // switch
        LOG(INFO) << "-- Test scene switch";
        sdk->startSimulation("scene1");
        std::this_thread::sleep_for(std::chrono::seconds(2));
        sdk->switchScene("scene2", false);
        std::this_thread::sleep_for(std::chrono::seconds(2));
        sdk->switchScene("scene1", true);
        std::this_thread::sleep_for(std::chrono::seconds(2));
        sdk->endSimulation();

        // pause/run
        LOG(INFO) << "-- Test simulation pause/run";
        sdk->startSimulation("scene2");
        std::this_thread::sleep_for(std::chrono::seconds(2));
        sdk->pauseSimulation();
        std::this_thread::sleep_for(std::chrono::seconds(2));
        sdk->runSimulation();
        std::this_thread::sleep_for(std::chrono::seconds(2));
        sdk->pauseSimulation();
        std::this_thread::sleep_for(std::chrono::seconds(2));
        sdk->endSimulation();

        // Quit
        while (sdk->getStatus() != imstk::SimulationStatus::INACTIVE) {}
     */

    return 0;
}
