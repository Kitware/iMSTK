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
#include "imstkIsometricMap.h"

#include "g3log/g3log.hpp"

void testViewer();
void testAnalyticalGeometry();
void testScenesManagement();
void testGeometryMaps();

int main()
{
    std::cout << "****************\n"
              << "Starting Sandbox\n"
              << "****************\n";

    testViewer();
    //testAnalyticalGeometry();
    //testScenesManagement();
    //testIsometricMaps();

    return 0;
}

void testViewer()
{
    // SDK and Scene
    auto sdk = std::make_shared<imstk::SimulationManager>();
    auto sceneTest = sdk->createNewScene("SceneTest");
    sceneTest->setLoopDelay(1000);

    // Plane
    auto planeGeom = std::make_shared<imstk::Plane>();
    planeGeom->scale(10);
    auto planeObj = std::make_shared<imstk::VisualObject>("VisualPlane");
    planeObj->setVisualGeometry(planeGeom);

    // Cube
    auto cubeGeom = std::make_shared<imstk::Cube>();
    cubeGeom->scale(0.5);
    cubeGeom->rotate(imstk::UP_VECTOR, imstk::PI_4);
    cubeGeom->rotate(imstk::RIGHT_VECTOR, imstk::PI_4);
    cubeGeom->translate(1.0, - 1.0, 0.5);
    auto cubeObj = std::make_shared<imstk::VisualObject>("VisualCube");
    cubeObj->setVisualGeometry(cubeGeom);

    // Sphere
    auto sphereGeom = std::make_shared<imstk::Sphere>();
    sphereGeom->scale(0.3);
    sphereGeom->translate(0, 2, 0);
    auto sphereObj = std::make_shared<imstk::VisualObject>("VisualSphere");
    sphereObj->setVisualGeometry(sphereGeom);

    // Light (white)
    auto whiteLight = std::make_shared<imstk::Light>("whiteLight");
    whiteLight->setPosition(imstk::Vec3d(5, 8, 5));
    whiteLight->setPositional();

    // Light (red)
    auto colorLight = std::make_shared<imstk::Light>("colorLight");
    colorLight->setPosition(imstk::Vec3d(4, -3, 1));
    colorLight->setFocalPoint(imstk::Vec3d(0, 0, 0));
    colorLight->setColor(imstk::Color::Red);
    colorLight->setPositional();
    colorLight->setSpotAngle(15);

    // Add in scene
    sceneTest->addSceneObject(planeObj);
    sceneTest->addSceneObject(cubeObj);
    sceneTest->addSceneObject(sphereObj);
    sceneTest->addLight(whiteLight);
    sceneTest->addLight(colorLight);

    // Update Camera
    auto cam1 = sceneTest->getCamera();
    cam1->setPosition(imstk::Vec3d(-5.5, 2.5, 32));
    cam1->setFocalPoint(imstk::Vec3d(1, 1, 0));

    // Run
    sdk->setCurrentScene("SceneTest");
    sdk->startSimulation(true);
}

void testAnalyticalGeometry()
{
    auto sdk = std::make_shared<imstk::SimulationManager>();

    // Plane
    LOG(INFO) << "-- Plane : Init";
    auto pos   = imstk::Vec3d(5, 2, 5);
    auto norm  = imstk::Vec3d(0, 1, 1);
    auto width = 10;
    LOG(INFO) << "p = " << pos;
    LOG(INFO) << "n = " << norm;
    LOG(INFO) << "w = " << width;

    LOG(INFO) << "-- Plane : Create";
    auto plane = std::make_shared<imstk::Plane>(pos, norm, width);
    LOG(INFO) << "p = " << plane->getPosition();
    LOG(INFO) << "n = " << plane->getNormal();
    LOG(INFO) << "w = " << plane->getWidth();

    LOG(INFO) << "-- Plane : Set Position";
    plane->setPosition(imstk::Vec3d(1, 1, 1));
    LOG(INFO) << "p = " << plane->getPosition();

    LOG(INFO) << "-- Plane : Translate";
    plane->translate(imstk::Vec3d(2, 1, -3));
    LOG(INFO) << "p = " << plane->getPosition();

    LOG(INFO) << "-- Plane : Set Normal";
    plane->setNormal(imstk::FORWARD_VECTOR);
    LOG(INFO) << "n = " << plane->getNormal();

    LOG(INFO) << "-- Plane : Rotate";
    plane->rotate(imstk::UP_VECTOR, imstk::PI_2);
    LOG(INFO) << "n = " << plane->getNormal();
}

void testScenesManagement()
{
    // THIS TESTS NEEDS TO DISABLE STANDALONE VIEWER RENDERING

    auto sdk = std::make_shared<imstk::SimulationManager>();

    // Scenes
    LOG(INFO) << "-- Test add scenes";
    auto scene1 = std::make_shared<imstk::Scene>("scene1");
    scene1->setLoopDelay(500);
    sdk->addScene(scene1);

    sdk->createNewScene("scene2");
    auto scene2 = sdk->getScene("scene2");
    scene2->setLoopDelay(500);

    auto scene3 = sdk->createNewScene();
    sdk->removeScene("Scene_3");

    // switch
    LOG(INFO) << "-- Test scene switch";
    int delay = 5;
    sdk->setCurrentScene("scene1");
    sdk->startSimulation();
    std::this_thread::sleep_for(std::chrono::seconds(delay));
    sdk->setCurrentScene("scene2", false);
    std::this_thread::sleep_for(std::chrono::seconds(delay));
    sdk->setCurrentScene("scene1", true);
    std::this_thread::sleep_for(std::chrono::seconds(delay));
    sdk->endSimulation();

    // pause/run
    LOG(INFO) << "-- Test simulation pause/run";
    sdk->setCurrentScene("scene2");
    sdk->startSimulation();
    std::this_thread::sleep_for(std::chrono::seconds(delay));
    sdk->pauseSimulation();
    std::this_thread::sleep_for(std::chrono::seconds(delay));
    sdk->runSimulation();
    std::this_thread::sleep_for(std::chrono::seconds(delay));
    sdk->pauseSimulation();
    std::this_thread::sleep_for(std::chrono::seconds(delay));
    sdk->endSimulation();

    // Quit
    while (sdk->getStatus() != imstk::SimulationStatus::INACTIVE) {}
}

void testTetraTriangleMap()
{
    // SDK and Scene
    auto sdk = std::make_shared<imstk::SimulationManager>();
    auto geometryMapTest = sdk->createNewScene("geometryMapTest");
    geometryMapTest->setLoopDelay(1000);
}

void testIsometricMaps()
{
    // SDK and Scene
    auto sdk = std::make_shared<imstk::SimulationManager>();
    auto geometryMapTest = sdk->createNewScene("geometryMapTest");
    geometryMapTest->setLoopDelay(1000);

    // Cube
    auto cubeGeom = std::make_shared<imstk::Cube>();
    cubeGeom->scale(0.5);
    auto cubeObj = std::make_shared<imstk::VisualObject>("VisualCube");
    cubeObj->setVisualGeometry(cubeGeom);

    // Sphere
    auto sphereGeom = std::make_shared<imstk::Sphere>();
    sphereGeom->scale(0.3);
    auto sphereObj = std::make_shared<imstk::VisualObject>("VisualSphere");
    sphereObj->setVisualGeometry(sphereGeom);

    // Add objects in Scene
    geometryMapTest->addSceneObject(cubeObj);
    geometryMapTest->addSceneObject(sphereObj);

    // Isometric Map
    auto transform = imstk::RigidTransform3d::Identity();
    transform.translate(imstk::Vec3d(0.0, 1.0, 0.0));
    transform.rotate(Eigen::AngleAxisd(imstk::PI_4, imstk::Vec3d(0, 1.0, 0)));
    auto rigidMap = std::make_shared<imstk::IsometricMap>();
    rigidMap->setMaster(sphereObj->getVisualGeometry());
    rigidMap->setSlave(cubeObj->getVisualGeometry());
    rigidMap->setTransform(transform);

    // Test map
    LOG(INFO) << cubeGeom->getPosition();

    rigidMap->applyMap();
    LOG(INFO) << cubeGeom->getPosition();

    sphereGeom->setPosition(1.0, 0.0, 1.0);
    rigidMap->applyMap();
    LOG(INFO) << cubeGeom->getPosition();

    // Start simulation
    sdk->setCurrentScene("geometryMapTest");
    sdk->startSimulation(imstk::Renderer::Mode::DEBUG);
}
