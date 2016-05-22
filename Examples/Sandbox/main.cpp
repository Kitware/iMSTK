#include <cstring>
#include <iostream>
#include <memory>
#include <thread>
#include <iomanip>

#include "imstkMath.h"
#include "imstkTimer.h"
#include "imstkSimulationManager.h"

// Objects
#include "imstkSceneObject.h"
#include "imstkVirtualToolObject.h"
#include "imstkLight.h"
#include "imstkCamera.h"

// Geometry
#include "imstkPlane.h"
#include "imstkSphere.h"
#include "imstkCube.h"
#include "imstkTetrahedralMesh.h"
#include "imstkSurfaceMesh.h"
#include "imstkMeshReader.h"

// Maps
#include "imstkTetraTriangleMap.h"
#include "imstkIsometricMap.h"
#include "imstkOneToOneMap.h"

// Devices
#include "imstkHDAPIDeviceClient.h"
#include "imstkVRPNDeviceClient.h"
#include "imstkVRPNDeviceServer.h"
#include "imstkCameraController.h"

// Collisions
#include "imstkInteractionPair.h"

#include "g3log/g3log.hpp"

using namespace imstk;

void testMultiTextures();
void testMeshCCD();
void testPenaltyRigidCollision();
void testTwoFalcons();
void testObjectController();
void testCameraController();
void testReadMesh();
void testViewer();
void testAnalyticalGeometry();
void testScenesManagement();
void testIsometricMap();
void testTetraTriangleMap();
void testOneToOneNodalMap();
void testExtractSurfaceMesh();
void testSurfaceMeshOptimizer();

int main()
{
    std::cout << "****************\n"
              << "Starting Sandbox\n"
              << "****************\n";

    testMultiTextures();
    //testMeshCCD();
    //testPenaltyRigidCollision();
    //testTwoFalcons();
    //testObjectController();
    //testCameraController();
    //testViewer();
    //testReadMesh();
    //testAnalyticalGeometry();
    //testScenesManagement();
    //testIsometricMap();
    //testTetraTriangleMap();
    //testExtractSurfaceMesh();
    //testOneToOneNodalMap();
    //testSurfaceMeshOptimizer();

    return 0;
}

void testMultiTextures()
{
    // SDK and Scene
    auto sdk = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("multitexturestest");

    // Read surface mesh
    auto objMesh = imstk::MeshReader::read("/home/virtualfls/Projects/IMSTK/resources/textures/Fox skull OBJ/fox_skull.obj");
    auto surfaceMesh = std::dynamic_pointer_cast<imstk::SurfaceMesh>(objMesh);
    surfaceMesh->addTexture("/home/virtualfls/Projects/IMSTK/resources/textures/Fox skull OBJ/fox_skull_0.jpg",
                        "material_0");
    surfaceMesh->addTexture("/home/virtualfls/Projects/IMSTK/resources/textures/Fox skull OBJ/fox_skull_1.jpg",
                        "material_1");

    // Create object and add to scene
    auto object = std::make_shared<imstk::VisualObject>("meshObject");
    object->setVisualGeometry(surfaceMesh); // change to any mesh created above
    scene->addSceneObject(object);

    // Run
    sdk->setCurrentScene("multitexturestest");
    sdk->startSimulation(true);
}

void testMeshCCD()
{
    // SDK and Scene
    auto sdk = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("MeshCCDTest");

    auto mesh1 = imstk::MeshReader::read("/home/virtualfls/Projects/IMSTK/resources/Spheres/big.vtk");
    auto mesh2 = imstk::MeshReader::read("/home/virtualfls/Projects/IMSTK/resources/Spheres/small_0.vtk");

    // Obj1
    auto obj1 = std::make_shared<CollidingObject>("obj1");
    obj1->setVisualGeometry(mesh1);
    obj1->setCollidingGeometry(mesh1);
    scene->addSceneObject(obj1);

    // Obj2
    auto obj2 = std::make_shared<CollidingObject>("obj2");
    obj2->setVisualGeometry(mesh2);
    obj2->setCollidingGeometry(mesh2);
    scene->addSceneObject(obj2);

    // Collisions
    auto colGraph = scene->getCollisionGraph();
    colGraph->addInteractionPair(obj1, obj2,
                                 CollisionDetection::Type::MeshToMesh,
                                 CollisionHandling::Type::None,
                                 CollisionHandling::Type::None);

    auto t = std::thread([mesh2]
    {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        auto mesh2_1 = imstk::MeshReader::read("/home/virtualfls/Projects/IMSTK/resources/Spheres/small_1.vtk");
        mesh2->setVerticesPositions(mesh2_1->getVerticesPositions());
        std::this_thread::sleep_for(std::chrono::seconds(5));
        auto mesh2_2 = imstk::MeshReader::read("/home/virtualfls/Projects/IMSTK/resources/Spheres/small_2.vtk");
        mesh2->setVerticesPositions(mesh2_2->getVerticesPositions());
        std::this_thread::sleep_for(std::chrono::seconds(5));
        auto mesh2_3 = imstk::MeshReader::read("/home/virtualfls/Projects/IMSTK/resources/Spheres/small_3.vtk");
        mesh2->setVerticesPositions(mesh2_3->getVerticesPositions());
    });

    // Run
    sdk->setCurrentScene("MeshCCDTest");
    sdk->startSimulation(true);
    t.join();
}

void testPenaltyRigidCollision()
{
    // SDK and Scene
    auto sdk = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("InteractionPairTest");

    // Device server
    auto server = std::make_shared<imstk::VRPNDeviceServer>();
    server->addDevice("device0", imstk::DeviceType::NOVINT_FALCON, 0);
    server->addDevice("device1", imstk::DeviceType::NOVINT_FALCON, 1);
    sdk->addDeviceServer(server);

    // Falcon clients
    auto client0 = std::make_shared<imstk::VRPNDeviceClient>("device0", "localhost");
    auto client1 = std::make_shared<imstk::VRPNDeviceClient>("device1", "localhost");
    client0->setForceEnabled(true);
    client1->setForceEnabled(true);
    sdk->addDeviceClient(client0);
    sdk->addDeviceClient(client1);

    // Plane
    auto planeGeom = std::make_shared<Plane>();
    planeGeom->scale(10);
    auto planeObj = std::make_shared<CollidingObject>("Plane");
    planeObj->setVisualGeometry(planeGeom);
    planeObj->setCollidingGeometry(planeGeom);
    scene->addSceneObject(planeObj);

    // Sphere0
    auto sphere0Geom = std::make_shared<Sphere>();
    sphere0Geom->scale(0.5);
    sphere0Geom->translate(Vec3d(1,0.5,0));
    auto sphere0Obj = std::make_shared<imstk::VirtualCouplingObject>("Sphere0", client0, 40);
    sphere0Obj->setVisualGeometry(sphere0Geom);
    sphere0Obj->setCollidingGeometry(sphere0Geom);
    scene->addSceneObject(sphere0Obj);

    // Sphere1
    auto sphere1Geom = std::make_shared<Sphere>();
    sphere1Geom->scale(0.5);
    sphere1Geom->translate(Vec3d(-1,0.5,0));
    auto sphere1Obj = std::make_shared<imstk::VirtualCouplingObject>("Sphere1", client1, 40);
    sphere1Obj->setVisualGeometry(sphere1Geom);
    sphere1Obj->setCollidingGeometry(sphere1Geom);
    scene->addSceneObject(sphere1Obj);

    // Collisions
    auto colGraph = scene->getCollisionGraph();
    colGraph->addInteractionPair(planeObj, sphere0Obj,
                                 CollisionDetection::Type::PlaneToSphere,
                                 CollisionHandling::Type::None,
                                 CollisionHandling::Type::Penalty);
    colGraph->addInteractionPair(planeObj, sphere1Obj,
                                 CollisionDetection::Type::PlaneToSphere,
                                 CollisionHandling::Type::None,
                                 CollisionHandling::Type::Penalty);
    colGraph->addInteractionPair(sphere0Obj, sphere1Obj,
                                 CollisionDetection::Type::SphereToSphere,
                                 CollisionHandling::Type::Penalty,
                                 CollisionHandling::Type::Penalty);

    // Run
    sdk->setCurrentScene("InteractionPairTest");
    sdk->startSimulation(true);
}

void testTwoFalcons()
{
    // SDK and Scene
    auto sdk = std::make_shared<imstk::SimulationManager>();
    auto scene = sdk->createNewScene("FalconsTestScene");

    // Device server
    auto server = std::make_shared<imstk::VRPNDeviceServer>();
    server->addDevice("falcon0", imstk::DeviceType::NOVINT_FALCON, 0);
    server->addDevice("falcon1", imstk::DeviceType::NOVINT_FALCON, 1);
    server->addDevice("hdk", imstk::DeviceType::OSVR_HDK);
    sdk->addDeviceServer(server);

    // Falcon clients
    auto falcon0 = std::make_shared<imstk::VRPNDeviceClient>("falcon0", "localhost");
    sdk->addDeviceClient(falcon0);
    auto falcon1 = std::make_shared<imstk::VRPNDeviceClient>("falcon1", "localhost");
    sdk->addDeviceClient(falcon1);

    // Cam Client
    auto hdk = std::make_shared<imstk::VRPNDeviceClient>("hdk", "localhost");
    sdk->addDeviceClient(hdk);

    // Plane
    auto planeGeom = std::make_shared<imstk::Plane>();
    planeGeom->scale(50);
    planeGeom->translate(imstk::FORWARD_VECTOR * 15);
    auto planeObj = std::make_shared<imstk::VisualObject>("VisualPlane");
    planeObj->setVisualGeometry(planeGeom);
    scene->addSceneObject(planeObj);

    // Sphere0
    auto sphere0Geom = std::make_shared<imstk::Sphere>();
    sphere0Geom->setPosition(imstk::Vec3d(16,4.5,0));
    sphere0Geom->scale(1);
    auto sphere0Obj = std::make_shared<imstk::VirtualToolObject>("Sphere0", falcon0, 30);
    sphere0Obj->setVisualGeometry(sphere0Geom);
    sphere0Obj->setCollidingGeometry(sphere0Geom);
    scene->addSceneObject(sphere0Obj);

    // Sphere1
    auto sphere1Geom = std::make_shared<imstk::Sphere>();
    sphere1Geom->setPosition(imstk::Vec3d(-16,4.5,0));
    sphere1Geom->scale(1);
    auto sphere1Obj = std::make_shared<imstk::VirtualToolObject>("Sphere1", falcon1, 30);
    sphere1Obj->setVisualGeometry(sphere1Geom);
    sphere1Obj->setCollidingGeometry(sphere1Geom);
    scene->addSceneObject(sphere1Obj);

    // Camera
    auto cam = scene->getCamera();
    cam->setPosition(imstk::Vec3d(0,18,20));
    cam->setFocalPoint(imstk::UP_VECTOR*18);
    cam->setupController(hdk);
    cam->getController()->setInversionFlags(imstk::CameraController::InvertFlag::rotY |
                                            imstk::CameraController::InvertFlag::rotZ );

    // Run
    sdk->setCurrentScene("FalconsTestScene");
    sdk->startSimulation(true);
}

void testObjectController()
{
#ifdef iMSTK_USE_OPENHAPTICS
    // SDK and Scene
    auto sdk = std::make_shared<imstk::SimulationManager>();
    auto scene = sdk->createNewScene("SceneTestDevice");

    // Device Client
    auto client = std::make_shared<imstk::HDAPIDeviceClient>("Default PHANToM"); // localhost = 127.0.0.1
    sdk->addDeviceClient(client);

    // Object
    auto geom = std::make_shared<imstk::Cube>();
    geom->setPosition(imstk::UP_VECTOR);
    geom->scale(2);
    auto object = std::make_shared<imstk::VirtualCouplingObject>("VirtualObject", client, 0.1);
    object->setVisualGeometry(geom);
    object->setCollidingGeometry(geom);
    scene->addSceneObject(object);

    // Update Camera position
    auto cam = scene->getCamera();
    cam->setPosition(imstk::Vec3d(0,0,10));
    cam->setFocalPoint(geom->getPosition());

    // Run
    sdk->setCurrentScene("SceneTestDevice");
    sdk->startSimulation(true);
#endif
}

void testCameraController()
{
    // SDK and Scene
    auto sdk = std::make_shared<imstk::SimulationManager>();
    auto scene = sdk->createNewScene("SceneTestDevice");

    // Device server
    auto server = std::make_shared<imstk::VRPNDeviceServer>("127.0.0.1");
    server->addDevice("device0", imstk::DeviceType::OSVR_HDK);
    sdk->addDeviceServer(server);

    // Device Client
    auto client = std::make_shared<imstk::VRPNDeviceClient>("device0", "localhost"); // localhost = 127.0.0.1
    //client->setLoopDelay(1000);
    sdk->addDeviceClient(client);

    // Mesh
    auto mesh = imstk::MeshReader::read("/home/virtualfls/Projects/IMSTK/resources/asianDragon/asianDragon.obj");
    auto meshObject = std::make_shared<imstk::VisualObject>("meshObject");
    meshObject->setVisualGeometry(mesh);
    scene->addSceneObject(meshObject);

    // Update Camera position
    auto cam = scene->getCamera();
    cam->setPosition(imstk::Vec3d(0,0,10));

    // Set camera controller
    cam->setupController(client, 100);
    //LOG(INFO) << cam->getController()->getTranslationOffset(); // should be the same than initial cam position
    cam->getController()->setInversionFlags(imstk::CameraController::InvertFlag::rotY |
                                            imstk::CameraController::InvertFlag::rotZ );

    // Run
    sdk->setCurrentScene("SceneTestDevice");
    sdk->startSimulation(true);
}

void testReadMesh()
{
    // SDK and Scene
    auto sdk = std::make_shared<imstk::SimulationManager>();
    auto scene = sdk->createNewScene("SceneTestMesh");

    // Read surface mesh
    auto objMesh = imstk::MeshReader::read("/home/virtualfls/Projects/IMSTK/resources/asianDragon/asianDragon.obj");
    auto plyMesh = imstk::MeshReader::read("/home/virtualfls/Projects/IMSTK/resources/Cube/models/cube.ply");
    auto stlMesh = imstk::MeshReader::read("/home/virtualfls/Projects/IMSTK/resources/Cube/models/cube.stl");
    auto vtkMesh = imstk::MeshReader::read("/home/virtualfls/Projects/IMSTK/resources/Cube/models/cube.vtk");
    auto vtpMesh = imstk::MeshReader::read("/home/virtualfls/Projects/IMSTK/resources/Cube/models/cube.vtp");

    // Read volumetricMesh
    auto vtkMesh2 = imstk::MeshReader::read("/home/virtualfls/Projects/IMSTK/resources/AVM/nidus-model/nidus10KTet.vtk");
    auto vegaMesh = imstk::MeshReader::read("/home/virtualfls/Projects/IMSTK/resources/asianDragon/asianDragon.veg");

    // Extract surface mesh
    auto volumeMesh = std::dynamic_pointer_cast<imstk::VolumetricMesh>(vegaMesh); // change to any volumetric mesh above
    volumeMesh->computeAttachedSurfaceMesh();
    auto surfaceMesh = volumeMesh->getAttachedSurfaceMesh();

    // Create object and add to scene
    auto object = std::make_shared<imstk::VisualObject>("meshObject");
    object->setVisualGeometry(surfaceMesh); // change to any mesh created above
    scene->addSceneObject(object);

    // Run
    sdk->setCurrentScene("SceneTestMesh");
    sdk->startSimulation(true);
}

void testViewer()
{
    // SDK and Scene
    auto sdk = std::make_shared<imstk::SimulationManager>();
    auto sceneTest = sdk->createNewScene("SceneTest");

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
    sdk->addScene(scene1);

    sdk->createNewScene("scene2");
    auto scene2 = sdk->getScene("scene2");

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

void testIsometricMap()
{
    // SDK and Scene
    auto sdk = std::make_shared<imstk::SimulationManager>();
    auto geometryMapTest = sdk->createNewScene("geometryMapTest");

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
    transform.rotate(imstk::Rotd(imstk::PI_4, imstk::Vec3d(0, 1.0, 0)));

    auto rigidMap = std::make_shared<imstk::IsometricMap>();
    rigidMap->setMaster(sphereObj->getVisualGeometry());
    rigidMap->setSlave(cubeObj->getVisualGeometry());
    rigidMap->setTransform(transform);

    // Test map
    LOG(INFO) << cubeGeom->getPosition();

    rigidMap->apply();
    LOG(INFO) << cubeGeom->getPosition();

    sphereGeom->setPosition(1.0, 0.0, 1.0);
    rigidMap->apply();
    LOG(INFO) << cubeGeom->getPosition();

    // Start simulation
    sdk->setCurrentScene("geometryMapTest");
    sdk->startSimulation(imstk::Renderer::Mode::DEBUG);
}

void testTetraTriangleMap()
{
    auto sdk = std::make_shared<imstk::SimulationManager>();

    // Tetrahedral mesh
    auto tetMesh = std::make_shared<imstk::TetrahedralMesh>();
    std::vector<imstk::Vec3d> vertList;
    vertList.push_back(imstk::Vec3d(0, 0, 0));
    vertList.push_back(imstk::Vec3d(1.0, 0, 0));
    vertList.push_back(imstk::Vec3d(0, 1.0, 0));
    vertList.push_back(imstk::Vec3d(0, 0, 1.0));
    tetMesh->setInitialVerticesPositions(vertList);
    tetMesh->setVerticesPositions(vertList);

    std::vector<imstk::TetrahedralMesh::TetraArray> tetConnectivity;
    imstk::TetrahedralMesh::TetraArray tet1 = { 0, 1, 2, 3 };
    tetConnectivity.push_back(tet1);
    tetMesh->setTetrahedraVertices(tetConnectivity);

    // Triangular mesh
    auto triMesh = std::make_shared<imstk::SurfaceMesh>();
    std::vector<imstk::Vec3d> SurfVertList;
    SurfVertList.push_back(imstk::Vec3d(0, 0, 1));// coincides with one vertex
    SurfVertList.push_back(imstk::Vec3d(0.25, 0.25, 0.25));// centroid
    SurfVertList.push_back(imstk::Vec3d(1.05, 0, 0));
    triMesh->setInitialVerticesPositions(SurfVertList);
    triMesh->setVerticesPositions(SurfVertList);

    // Construct a map
    auto tetTriMap = std::make_shared<imstk::TetraTriangleMap>();
    tetTriMap->setMaster(tetMesh);
    tetTriMap->setSlave(triMesh);
    tetTriMap->compute();

    tetTriMap->print();

    getchar();
}

void testExtractSurfaceMesh()
{
    auto sdk = std::make_shared<imstk::SimulationManager>();

    // a. Construct a sample tetrahedral mesh

    // a.1 add vertex positions
    auto tetMesh = std::make_shared<imstk::TetrahedralMesh>();
    std::vector<imstk::Vec3d> vertList;
    vertList.push_back(imstk::Vec3d(0, 0, 0));
    vertList.push_back(imstk::Vec3d(1.0, 0, 0));
    vertList.push_back(imstk::Vec3d(0, 1.0, 0));
    vertList.push_back(imstk::Vec3d(0, 0, 1.0));
    vertList.push_back(imstk::Vec3d(1.0, 1.0, 1.0));
    tetMesh->setInitialVerticesPositions(vertList);
    tetMesh->setVerticesPositions(vertList);

    // a.2 add connectivity
    std::vector<imstk::TetrahedralMesh::TetraArray> tetConnectivity;
    imstk::TetrahedralMesh::TetraArray tet1 = { 0, 1, 2, 3 };
    imstk::TetrahedralMesh::TetraArray tet2 = { 1, 2, 3, 4 };
    tetConnectivity.push_back(tet1);
    tetConnectivity.push_back(tet2);
    tetMesh->setTetrahedraVertices(tetConnectivity);

    // b. Print tetrahedral mesh
    tetMesh->print();

    // c. Extract the surface mesh
    auto surfMesh = std::make_shared<imstk::SurfaceMesh>();
    if (tetMesh->extractSurfaceMesh(surfMesh))
    {
        // c.1. Print the resulting mesh
        surfMesh->print();
    }
    else
    {
        LOG(WARNING) << "Surface mesh was not extracted!";
    }

    getchar();
}

void testOneToOneNodalMap()
{
    auto sdk = std::make_shared<imstk::SimulationManager>();

    // a. Construct a sample tetrahedral mesh

    // a.1 add vertex positions
    auto tetMesh = std::make_shared<imstk::TetrahedralMesh>();
    std::vector<imstk::Vec3d> vertList;
    vertList.push_back(imstk::Vec3d(0, 0, 0));
    vertList.push_back(imstk::Vec3d(1.0, 0, 0));
    vertList.push_back(imstk::Vec3d(0, 1.0, 0));
    vertList.push_back(imstk::Vec3d(0, 0, 1.0));
    vertList.push_back(imstk::Vec3d(1.0, 1.0, 1.0));
    tetMesh->setInitialVerticesPositions(vertList);
    tetMesh->setVerticesPositions(vertList);

    tetMesh->print();

    // b. Construct a surface mesh
    auto triMesh = std::make_shared<imstk::SurfaceMesh>();

    // b.1 Add vertex positions
    std::vector<imstk::Vec3d> SurfVertList;
    SurfVertList.push_back(imstk::Vec3d(0, 0, 0));
    SurfVertList.push_back(imstk::Vec3d(1.0, 0, 0));
    SurfVertList.push_back(imstk::Vec3d(0, 1.0, 0));
    SurfVertList.push_back(imstk::Vec3d(0, 0, 1.0));
    SurfVertList.push_back(imstk::Vec3d(1.0, 1.0, 1.0));
    triMesh->setInitialVerticesPositions(SurfVertList);
    triMesh->setVerticesPositions(SurfVertList);

    // b.2 Add vertex connectivity
    std::vector<imstk::SurfaceMesh::TriangleArray> triConnectivity;
    triConnectivity.push_back({ { 0, 1, 2 } });
    triConnectivity.push_back({ { 0, 1, 3 } });
    triConnectivity.push_back({ { 0, 2, 3 } });
    triConnectivity.push_back({ { 1, 2, 4 } });
    triConnectivity.push_back({ { 1, 3, 4 } });
    triConnectivity.push_back({ { 2, 3, 4 } });
    triMesh->setTrianglesVertices(triConnectivity);

    triMesh->print();

    // c. Construct the one to one nodal map based on the above meshes
    auto oneToOneNodalMap = std::make_shared<imstk::OneToOneMap>();
    oneToOneNodalMap->setMaster(tetMesh);
    oneToOneNodalMap->setSlave(triMesh);

    // d. Compute the map
    oneToOneNodalMap->compute();

    // e. Print the computed nodal map if valid
    if (oneToOneNodalMap->isValid())
    {
        oneToOneNodalMap->print();
    }

    getchar();
}

void testSurfaceMeshOptimizer()
{
    auto sdk = std::make_shared<imstk::SimulationManager>();

    // a. Construct a sample triangular mesh

    // b. Add nodal data
    auto surfMesh = std::make_shared<imstk::SurfaceMesh>();
    std::vector<imstk::Vec3d> vertList;
    vertList.push_back(imstk::Vec3d(0, 0, 0));
    vertList.push_back(imstk::Vec3d(0.5, 0.5, 0));
    vertList.push_back(imstk::Vec3d(1, 1, 0));
    vertList.push_back(imstk::Vec3d(1, 0, 0));
    vertList.push_back(imstk::Vec3d(0, 1, 0));
    vertList.push_back(imstk::Vec3d(0.5, 1, 0));
    vertList.push_back(imstk::Vec3d(0, 0.5, 0));
    vertList.push_back(imstk::Vec3d(1, 0.5, 0));
    vertList.push_back(imstk::Vec3d(0.5, 0, 0));
    surfMesh->setInitialVerticesPositions(vertList);
    surfMesh->setVerticesPositions(vertList);

    // c. Add connectivity data
    std::vector<imstk::SurfaceMesh::TriangleArray> triangles;
    imstk::SurfaceMesh::TriangleArray tri[8];
    tri[0] = { { 0, 8, 6 } };
    tri[1] = { { 7, 2, 5 } };
    tri[2] = { { 1, 5, 4 } };
    tri[3] = { { 3, 7, 1 } };
    tri[4] = { { 8, 1, 6 } };
    tri[5] = { { 1, 4, 6 } };
    tri[6] = { { 1, 7, 5 } };
    tri[7] = { { 3, 1, 8 } };

    for (int i = 0; i < 8; i++)
    {
        triangles.push_back(tri[i]);
    }

    surfMesh->setTrianglesVertices(triangles);

    imstk::StopWatch wwt;
    imstk::CpuTimer ct;

    wwt.start();
    ct.start();

    // d. Print the mesh
    surfMesh->print();

    // e. Rewire the mesh position and connectivity
    surfMesh->optimizeForDataLocality();

    // f. Print the resulting mesh
    surfMesh->print();

    /*wwt.storeLap("opDataLoc");
    wwt.printLapTimes();*/

    wwt.printTimeElapsed("opDataLoc");

    //std::cout << "wall clock time: " << wwt.getTimeElapsed() << " ms." << std::endl;
    LOG(INFO) << "CPU time: " << ct.getTimeElapsed() << " ms.";

    // Cross-check
    // Connectivity: 0:(0, 1, 2), 1:(1, 3, 2), 2:(3, 4, 2), 3:(5, 3, 1), 4:(3, 6, 4), 5:(5, 7, 3), 6:(3, 7, 6), 7:(7, 8, 6)
    // Nodal data: 0:(0, 0, 0), 1:(0.5, 0, 0), 2:(0, 0.5, 0), 3:(0.5, 0.5, 0), 4:(0, 1, 0), 5:(1, 0, 0), 6:(0.5, 1, 0), 7:(1, 0.5, 0), 8:(1, 1, 0)

    getchar();
}
