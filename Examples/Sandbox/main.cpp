#include <cstring>
#include <iostream>
#include <memory>
#include <thread>
#include <iomanip>

#include "imstkMath.h"
#include "imstkSimulationManager.h"
#include "imstkSceneObject.h"
#include "imstkPlane.h"
#include "imstkSphere.h"
#include "imstkCube.h"
#include "imstkLight.h"
#include "imstkCamera.h"

// Maps
#include "imstkIsometricMap.h"

#include "g3log/g3log.hpp"

void testGeometryMaps();

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

    planeGeom->scale(8);
    auto planeObj = std::make_shared<imstk::VisualObject>("VisualPlane");
    planeObj->setVisualGeometry(planeGeom);

    auto cubeGeom = std::make_shared<imstk::Cube>();
    cubeGeom->scale(0.5);
    cubeGeom->rotate(imstk::UP_VECTOR, imstk::PI_4);
    cubeGeom->rotate(imstk::RIGHT_VECTOR, imstk::PI_4);
    cubeGeom->translate(1.0, - 1.0, 0.5);
    auto cubeObj = std::make_shared<imstk::VisualObject>("VisualCube");
    cubeObj->setVisualGeometry(cubeGeom);

    auto sphereGeom = std::make_shared<imstk::Sphere>();
    sphereGeom->scale(0.3);
    sphereGeom->translate(0, 2, 0);
    auto sphereObj = std::make_shared<imstk::VisualObject>("VisualSphere");
    sphereObj->setVisualGeometry(sphereGeom);

    auto whiteLight = std::make_shared<imstk::Light>("whiteLight");
    whiteLight->setPosition(imstk::Vec3d(5, 8, 5));
    whiteLight->setPositional();

    auto colorLight = std::make_shared<imstk::Light>("colorLight");
    colorLight->setPosition(imstk::Vec3d(4, -3, 1));
    colorLight->setFocalPoint(imstk::Vec3d(0, 0, 0));
    colorLight->setColor(imstk::Color::Red);
    colorLight->setPositional();
    colorLight->setSpotAngle(15);

    auto sceneTest = sdk->createNewScene("SceneTest");
    sceneTest->setLoopDelay(1000);
    sceneTest->addSceneObject(planeObj);
    sceneTest->addSceneObject(cubeObj);
    sceneTest->addSceneObject(sphereObj);
    sceneTest->addLight(whiteLight);
    sceneTest->addLight(colorLight);

    auto cam1 = sceneTest->getCamera();
    cam1->setPosition(imstk::Vec3d(-5.5, 2.5, 32));
    cam1->setFocalPoint(imstk::Vec3d(1, 1, 0));

    sdk->startSimulation("SceneTest");


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
        plane->setNormal(imstk::FORWARD_VECTOR);
        std::cout << "n = " << plane->getNormal() << std::endl;

        std::cout << "-- Plane : Rotate" << std::endl;
        plane->rotate(imstk::UP_VECTOR, imstk::PI_2);
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

void testGeometryMaps()
{
    std::cout << "****************" << std::endl
        << "Test: Geometric maps" << std::endl
        << "****************" << std::endl;

    auto sdk = std::make_shared<imstk::SimulationManager>();

    // Scene object 1
    auto cubeGeom = std::make_shared<imstk::Cube>();
    cubeGeom->scale(0.5);
    auto cubeObj = std::make_shared<imstk::VisualObject>("VisualCube");
    cubeObj->setVisualGeometry(cubeGeom);

    // Scene object 2
    auto sphereGeom = std::make_shared<imstk::Sphere>();
    sphereGeom->scale(0.3);
    auto sphereObj = std::make_shared<imstk::VisualObject>("VisualSphere");
    sphereObj->setVisualGeometry(sphereGeom);

    // Isometric Map
    auto rigidMap = std::make_shared<imstk::IsometricMap>();

    rigidMap->setMaster(sphereObj->getVisualGeometry());
    rigidMap->setSlave(cubeObj->getVisualGeometry());

    auto trans = imstk::RigidTransform3d::Identity();
    trans.translate(imstk::Vec3d(1.5, 1.5, 1.5));

    auto rz = imstk::RigidTransform3d(Eigen::AngleAxisd(imstk::PI_4, imstk::Vec3d(0, 1.0, 0)));
    trans.rotate(rz.rotation());

    rigidMap->setTransform(trans);

    // Apply map
    rigidMap->applyMap();

    // Move master and Apply map again
    sphereObj->getVisualGeometry()->setPosition(-1.0, 0, 0);
    rigidMap->applyMap();

    auto geometryMapTest = sdk->createNewScene("geometryMapTest");
    geometryMapTest->setLoopDelay(1000);
    geometryMapTest->addSceneObject(cubeObj);
    geometryMapTest->addSceneObject(sphereObj);

    sdk->startSimulation("geometryMapTest");
}
