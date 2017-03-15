#include <cstring>
#include <iostream>
#include <memory>
#include <thread>
#include <iomanip>

#include "imstkMath.h"
#include "imstkTimer.h"
#include "imstkSimulationManager.h"

// Objects
#include "imstkForceModelConfig.h"
#include "imstkFEMDeformableBodyModel.h"
#include "imstkDeformableObject.h"
#include "imstkSceneObject.h"
#include "imstkLight.h"
#include "imstkCamera.h"

// Time Integrators
#include "imstkBackwardEuler.h"

// Solvers
#include "imstkNonlinearSystem.h"
#include "imstkNewtonSolver.h"
#include "imstkConjugateGradient.h"
#include "imstkPbdSolver.h"

// Geometry
#include "imstkPlane.h"
#include "imstkSphere.h"
#include "imstkCube.h"
#include "imstkTetrahedralMesh.h"
#include "imstkSurfaceMesh.h"
#include "imstkMeshIO.h"
#include "imstkLineMesh.h"

// Maps
#include "imstkTetraTriangleMap.h"
#include "imstkIsometricMap.h"
#include "imstkOneToOneMap.h"

// Devices
#include "imstkHDAPIDeviceClient.h"
#include "imstkHDAPIDeviceServer.h"
#include "imstkVRPNDeviceClient.h"
#include "imstkVRPNDeviceServer.h"
#include "imstkCameraController.h"
#include "imstkSceneObjectController.h"
#include "imstkLaparoscopicToolController.h"

// Collisions
#include "imstkInteractionPair.h"
#include "imstkMeshToPlaneCD.h"
#include "imstkMeshToSphereCD.h"

// logger
#include "g3log/g3log.hpp"

// imstk utilities
#include "imstkPlotterUtils.h"
#include "imstkAPIUtilities.h"

#include "imstkVirtualCouplingPBDObject.h"
#include "imstkPbdObject.h"

// testVTKTexture
#include <vtkOBJReader.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <string>
#include <vtkJPEGReader.h>

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
void testDeformableBody();
void testVTKTexture();
void testMultiObjectWithTextures();
void testTwoOmnis();
void testVectorPlotters();
void testPbdVolume();
void testPbdCloth();
void testPbdCollision();
void testLineMesh();
void testMshAndVegaIO();
void testLapToolController();
void testScreenShotUtility();
void testDeformableBodyCollision();
void liverToolInteraction();

int main()
{
    std::cout << "****************\n"
        << "Starting Sandbox\n"
        << "****************\n";

    /*------------------
    Test rendering
    ------------------*/
    //testMultiTextures();
    //testVTKTexture();
    //testMultiObjectWithTextures();
    //testViewer();
    //testScreenShotUtility();


    /*------------------
    Test CD and CR
    ------------------*/
    //testMeshCCD();
    //testPenaltyRigidCollision();


    /*------------------
    Test geometry, maps
    ------------------*/
    //testIsometricMap();
    //testTetraTriangleMap();
    //testExtractSurfaceMesh();
    //testOneToOneNodalMap();
    //testSurfaceMeshOptimizer();
    //testAnalyticalGeometry();


    /*------------------
    Test physics
    ------------------*/
    //testPbdVolume();
    //testPbdCloth();
    //testPbdCollision();
    //testDeformableBody();
    //testDeformableBodyCollision();
    liverToolInteraction();


    /*------------------
    Test mesh I/O
    ------------------*/
    //testLineMesh();
    //testMshAndVegaIO();
    //testReadMesh();


    /*------------------
    Test devices, controllers
    ------------------*/
    //testObjectController();
    //testTwoFalcons();
    //testCameraController();
    //testTwoOmnis();
    //testLapToolController();


    /*------------------
    Test Misc.
    ------------------*/
    //testScenesManagement();
    //testVectorPlotters();


    return 0;
}

void testLapToolController()
{
#ifdef iMSTK_USE_OPENHAPTICS
    // SDK and Scene
    auto sdk = std::make_shared<imstk::SimulationManager>();
    auto scene = sdk->createNewScene("TestLapToolController");

    // Device clients
    auto client0 = std::make_shared<imstk::HDAPIDeviceClient>("PHANToM 1");

    // Device Server
    auto server = std::make_shared<imstk::HDAPIDeviceServer>();
    server->addDeviceClient(client0);
    sdk->addModule(server);

    // Plane
    auto planeObj = apiutils::createVisualAnalyticalSceneObject(
        imstk::Geometry::Type::Plane, scene, "VisualPlane", 100., imstk::Vec3d(0., -20., 0.));

    // laparoscopic tool
    auto pivot = apiutils::createAndAddVisualSceneObject(scene, iMSTK_DATA_ROOT"/laptool/pivot.obj", "pivot");
    auto upperJaw = apiutils::createAndAddVisualSceneObject(scene, iMSTK_DATA_ROOT"/laptool/upper.obj", "upperJaw");
    auto lowerJaw = apiutils::createAndAddVisualSceneObject(scene, iMSTK_DATA_ROOT"/laptool/lower.obj", "lowerJaw");

    auto trackingCtrl = std::make_shared<imstk::DeviceTracker>(client0);
    trackingCtrl->setTranslationScaling(0.5);
    auto lapToolController = std::make_shared<imstk::LaparoscopicToolController>(pivot, upperJaw, lowerJaw, trackingCtrl);
    lapToolController->setJawRotationAxis(imstk::Vec3d(1.0, 0, 0));
    lapToolController->setJawAngleChange(0.1);
    scene->addObjectController(lapToolController);

    // Set Camera
    auto cam = scene->getCamera();
    cam->setPosition(imstk::Vec3d(0, 30, 60));
    cam->setFocalPoint(imstk::Vec3d(0, 0, 0));

    // Run
    sdk->setCurrentScene(scene);
    sdk->startSimulation(true);
#endif
}

void testMshAndVegaIO()
{
    // SDK and Scene
    auto sdk = std::make_shared<imstk::SimulationManager>();
    auto scene = sdk->createNewScene("SceneTestMesh");

    // Load a volumetric mesh (from .msh file)
    std::string ifile = iMSTK_DATA_ROOT"/liver/liver.msh";
    auto volMeshA = imstk::MeshIO::read(ifile);
    if (!volMeshA)
    {
        LOG(WARNING) << "Failed to read msh file : " << ifile;
        return;
    }

    // Extract surface mesh
    auto volumeMeshA = std::dynamic_pointer_cast<imstk::VolumetricMesh>(volMeshA); // change to any volumetric mesh above
    volumeMeshA->computeAttachedSurfaceMesh();
    auto surfaceMeshA = volumeMeshA->getAttachedSurfaceMesh();

    // Create object A
    auto objectA = std::make_shared<imstk::VisualObject>("meshObjectMSH");
    objectA->setVisualGeometry(surfaceMeshA);

    // Write a .veg file
    std::string ofile = iMSTK_DATA_ROOT"/liver/liver.veg";
    auto writeStatus = imstk::MeshIO::write(volMeshA, ofile);
    std::cout << "------------------------------Summary----------------------------------------------------\n";
    std::cout << "Following file conversion: " << ((writeStatus) ? "Success \n" : "Failure \n");
    std::cout << "\n Input mesh file : \n" << ifile << std::endl;
    std::cout << "\n Output mesh file: \n" << ofile << std::endl;

    // Read the above written veg file
    auto volMeshB = imstk::MeshIO::read(ofile);
    if (!volMeshB)
    {
        LOG(WARNING) << "Failed to extract topology/geometry from the veg file : " << ofile;
        return;
    }

    // Extract surface mesh
    auto volumeMeshB = std::dynamic_pointer_cast<imstk::VolumetricMesh>(volMeshB); // change to any volumetric mesh above
    volumeMeshB->computeAttachedSurfaceMesh();
    auto surfaceMeshB = volumeMeshB->getAttachedSurfaceMesh();

    // Create object B
    auto objectB = std::make_shared<imstk::VisualObject>("meshObjectVEGA");
    surfaceMeshB->translate(Vec3d(3, 0, 0));
    objectB->setVisualGeometry(surfaceMeshB);

    // Add objects to the scene
    scene->addSceneObject(objectA);
    scene->addSceneObject(objectB);

    // Run
    sdk->setCurrentScene(scene);
    sdk->startSimulation(true);
}

void testVTKTexture()
{
    // Parse command line arguments

    std::string inputFilename = iMSTK_DATA_ROOT"/ETI/resources/OperatingRoom/cloth.obj";
    std::string texturename = iMSTK_DATA_ROOT"/ETI/resources/TextureOR/cloth.jpg";

    std::string inputFilename1 = iMSTK_DATA_ROOT"/ETI/resources/OperatingRoom/bed1.obj";
    std::string texturename1 = iMSTK_DATA_ROOT"/ETI/resources/TextureOR/bed-1.jpg";

    vtkSmartPointer<vtkOBJReader> reader =
        vtkSmartPointer<vtkOBJReader>::New();
    reader->SetFileName(inputFilename.c_str());
    reader->Update();


    vtkSmartPointer<vtkOBJReader> reader1 =
        vtkSmartPointer<vtkOBJReader>::New();
    reader1->SetFileName(inputFilename1.c_str());
    reader1->Update();

    // Visualize
    vtkSmartPointer<vtkPolyDataMapper> mapper =
        vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(reader->GetOutputPort());

    vtkSmartPointer<vtkPolyDataMapper> mapper1 =
        vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper1->SetInputConnection(reader1->GetOutputPort());

    vtkSmartPointer<vtkActor> actor =
        vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);

    vtkSmartPointer<vtkActor> actor1 =
        vtkSmartPointer<vtkActor>::New();
    actor1->SetMapper(mapper1);

    vtkSmartPointer<vtkJPEGReader> jpgReader =
        vtkSmartPointer<vtkJPEGReader>::New();
    jpgReader->SetFileName(texturename.c_str());
    jpgReader->Update();
    vtkSmartPointer<vtkTexture> texture = vtkSmartPointer<vtkTexture>::New();
    texture->SetInputConnection(jpgReader->GetOutputPort());
    texture->InterpolateOn();
    actor->SetTexture(texture);

    vtkSmartPointer<vtkJPEGReader> jpgReader1 =
        vtkSmartPointer<vtkJPEGReader>::New();
    jpgReader1->SetFileName(texturename1.c_str());
    jpgReader1->Update();
    vtkSmartPointer<vtkTexture> texture1 = vtkSmartPointer<vtkTexture>::New();
    texture1->SetInputConnection(jpgReader1->GetOutputPort());
    texture1->InterpolateOn();
    actor1->SetTexture(texture1);

    vtkSmartPointer<vtkRenderer> renderer =
        vtkSmartPointer<vtkRenderer>::New();
    renderer->AddActor(actor);
    renderer->AddActor(actor1);
    renderer->SetBackground(.3, .6, .3); // Background color green

    vtkSmartPointer<vtkRenderWindow> renderWindow =
        vtkSmartPointer<vtkRenderWindow>::New();
    renderWindow->AddRenderer(renderer);

    vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
        vtkSmartPointer<vtkRenderWindowInteractor>::New();
    renderWindowInteractor->SetRenderWindow(renderWindow);

    renderWindowInteractor->Start();

}


void testMultiObjectWithTextures()
{
    // SDK and Scene
    auto sdk = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("multiObjectWithTexturesTest");

    // Read surface mesh
    auto objMesh = imstk::MeshIO::read(iMSTK_DATA_ROOT"/ETI/resources/OperatingRoom/cloth.obj");
    auto surfaceMesh = std::dynamic_pointer_cast<imstk::SurfaceMesh>(objMesh);
    surfaceMesh->addTexture(iMSTK_DATA_ROOT"/ETI/resources/TextureOR/cloth.jpg");

    // Create object and add to scene
    auto object = std::make_shared<imstk::VisualObject>("meshObject");
    object->setVisualGeometry(surfaceMesh); // change to any mesh created above
    scene->addSceneObject(object);

    bool secondObject = true;
    bool secondObjectTexture = true;

    if (secondObject){
        // Read surface mesh1
        auto objMesh1 = imstk::MeshIO::read(iMSTK_DATA_ROOT"/ETI/resources/OperatingRoom/bed1.obj");
        auto surfaceMesh1 = std::dynamic_pointer_cast<imstk::SurfaceMesh>(objMesh1);
        if (secondObjectTexture)
            surfaceMesh1->addTexture(iMSTK_DATA_ROOT"/ETI/resources/TextureOR/bed-1.jpg");

        // Create object and add to scene
        auto object1 = std::make_shared<imstk::VisualObject>("meshObject1");
        object1->setVisualGeometry(surfaceMesh1); // change to any mesh created above
        scene->addSceneObject(object1);
    }
    // Run
    sdk->setCurrentScene(scene);
    sdk->startSimulation(true);
}


void testMultiTextures()
{
    // SDK and Scene
    auto sdk = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("multitexturestest");

    // Read surface mesh
    auto objMesh = imstk::MeshIO::read(iMSTK_DATA_ROOT"/textures/Fox skull OBJ/fox_skull.obj");
    auto surfaceMesh = std::dynamic_pointer_cast<imstk::SurfaceMesh>(objMesh);
    surfaceMesh->addTexture(iMSTK_DATA_ROOT"/textures/Fox skull OBJ/fox_skull_0.jpg",
        "material_0");
    surfaceMesh->addTexture(iMSTK_DATA_ROOT"/textures/Fox skull OBJ/fox_skull_1.jpg",
        "material_1");

    // Create object and add to scene
    auto object = std::make_shared<imstk::VisualObject>("meshObject");
    object->setVisualGeometry(surfaceMesh); // change to any mesh created above
    scene->addSceneObject(object);

    // Run
    sdk->setCurrentScene(scene);
    sdk->startSimulation(true);
}

void testMeshCCD()
{
    // SDK and Scene
    auto sdk = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("MeshCCDTest");

    auto mesh1 = imstk::MeshIO::read(iMSTK_DATA_ROOT"/spheres/big.vtk");
    auto mesh2 = imstk::MeshIO::read(iMSTK_DATA_ROOT"/spheres/small_0.vtk");

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
        auto mesh2_1 = imstk::MeshIO::read(iMSTK_DATA_ROOT"/spheres/small_1.vtk");
        mesh2->setVerticesPositions(mesh2_1->getVertexPositions());
        std::this_thread::sleep_for(std::chrono::seconds(5));
        auto mesh2_2 = imstk::MeshIO::read(iMSTK_DATA_ROOT"/spheres/small_2.vtk");
        mesh2->setVerticesPositions(mesh2_2->getVertexPositions());
        std::this_thread::sleep_for(std::chrono::seconds(5));
        auto mesh2_3 = imstk::MeshIO::read(iMSTK_DATA_ROOT"/spheres/small_3.vtk");
        mesh2->setVerticesPositions(mesh2_3->getVertexPositions());
    });

    // Run
    sdk->setCurrentScene(scene);
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
    sdk->addModule(server);

    // Falcon clients
    auto client0 = std::make_shared<imstk::VRPNDeviceClient>("device0", "localhost");
    auto client1 = std::make_shared<imstk::VRPNDeviceClient>("device1", "localhost");
    client0->setForceEnabled(true);
    client1->setForceEnabled(true);
    sdk->addModule(client0);
    sdk->addModule(client1);

    // Plane
    auto planeObj = apiutils::createCollidingAnalyticalSceneObject(imstk::Geometry::Type::Plane, scene, "plane", 10);

    // Sphere0
    auto sphere0Obj = apiutils::createCollidingAnalyticalSceneObject(
        imstk::Geometry::Type::Sphere, scene, "Sphere0", 0.5, Vec3d(1, 0.5, 0));

    auto trackCtrl0 = std::make_shared<imstk::DeviceTracker>(client0);
    trackCtrl0->setTranslationScaling(40);
    auto sphere0Controller = std::make_shared<imstk::SceneObjectController>(sphere0Obj, trackCtrl0);
    scene->addObjectController(sphere0Controller);

    // Sphere1
    auto sphere1Obj = apiutils::createCollidingAnalyticalSceneObject(
        imstk::Geometry::Type::Sphere, scene, "Sphere1", 0.5, Vec3d(-1., 0.5, 0.));

    auto trackCtrl1 = std::make_shared<imstk::DeviceTracker>(client1);
    trackCtrl1->setTranslationScaling(40);
    auto sphere1Controller = std::make_shared<imstk::SceneObjectController>(sphere1Obj, trackCtrl1);
    scene->addObjectController(sphere1Controller);

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
    sdk->setCurrentScene(scene);
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
    sdk->addModule(server);

    // Falcon clients
    auto falcon0 = std::make_shared<imstk::VRPNDeviceClient>("falcon0", "localhost");
    sdk->addModule(falcon0);
    auto falcon1 = std::make_shared<imstk::VRPNDeviceClient>("falcon1", "localhost");
    sdk->addModule(falcon1);

    // Cam Client
    auto hdk = std::make_shared<imstk::VRPNDeviceClient>("hdk", "localhost");
    sdk->addModule(hdk);

    // Plane
    auto planeObj = apiutils::createVisualAnalyticalSceneObject(
        imstk::Geometry::Type::Plane, scene, "VisualPlane", 50, imstk::FORWARD_VECTOR * 15);

    // Sphere0
    auto sphere0Obj = apiutils::createCollidingAnalyticalSceneObject(
        imstk::Geometry::Type::Sphere, scene, "Sphere0", 1., Vec3d(16, 4.5, 0));

    auto trackCtrl0 = std::make_shared<imstk::DeviceTracker>(falcon0);
    trackCtrl0->setTranslationScaling(30);
    auto controller0 = std::make_shared<imstk::SceneObjectController>(sphere0Obj, trackCtrl0);
    scene->addObjectController(controller0);

    // Sphere1
    auto sphere1Obj = apiutils::createCollidingAnalyticalSceneObject(
        imstk::Geometry::Type::Sphere, scene, "Sphere1", 1., Vec3d(-16, 4.5, 0));

    auto trackCtrl1 = std::make_shared<imstk::DeviceTracker>(falcon1);
    trackCtrl1->setTranslationScaling(30);
    auto controller1 = std::make_shared<imstk::SceneObjectController>(sphere1Obj, trackCtrl1);
    scene->addObjectController(controller1);

    // Camera
    auto cam = scene->getCamera();
    cam->setPosition(imstk::Vec3d(0, 18, 20));
    cam->setFocalPoint(imstk::UP_VECTOR * 18);
    auto camController = cam->setupController(hdk);
    camController->setInversionFlags(imstk::CameraController::InvertFlag::rotY |
                                     imstk::CameraController::InvertFlag::rotZ);

    // Run
    sdk->setCurrentScene(scene);
    sdk->startSimulation(true);
}

void testTwoOmnis()
{
#ifdef iMSTK_USE_OPENHAPTICS
    // SDK and Scene
    auto sdk = std::make_shared<imstk::SimulationManager>();
    auto scene = sdk->createNewScene("OmnisTestScene");

    // Device clients
    auto client0 = std::make_shared<imstk::HDAPIDeviceClient>("PHANToM 1");
    auto client1 = std::make_shared<imstk::HDAPIDeviceClient>("PHANToM 2");

    // Device Server
    auto server = std::make_shared<imstk::HDAPIDeviceServer>();
    server->addDeviceClient(client0);
    server->addDeviceClient(client1);
    sdk->addModule(server);

    // Plane
    auto planeObj = apiutils::createVisualAnalyticalSceneObject(
        imstk::Geometry::Type::Plane, scene, "VisualPlane", 50., imstk::FORWARD_VECTOR * 15);

    // Sphere0
    auto sphere0Obj = apiutils::createCollidingAnalyticalSceneObject(
        imstk::Geometry::Type::Sphere, scene, "Sphere0", 1., Vec3d(2, 2.5, 0));

    auto trackCtrl0 = std::make_shared<imstk::DeviceTracker>(client0);
    trackCtrl0->setTranslationScaling(0.05);
    auto controller0 = std::make_shared<imstk::SceneObjectController>(sphere0Obj, trackCtrl0);
    scene->addObjectController(controller0);

    // Sphere1
    auto sphere1Obj = apiutils::createCollidingAnalyticalSceneObject(
        imstk::Geometry::Type::Sphere, scene, "Sphere1", 1., Vec3d(-2, 2.5, 0));

    auto trackCtrl1 = std::make_shared<imstk::DeviceTracker>(client1);
    trackCtrl1->setTranslationScaling(0.05);
    auto controller1 = std::make_shared<imstk::SceneObjectController>(sphere1Obj, trackCtrl1);
    scene->addObjectController(controller1);

    // Update Camera position
    auto cam = scene->getCamera();
    cam->setPosition(imstk::Vec3d(0, 0, 10));
    auto sphere0Geom = sphere0Obj->getVisualGeometry();
    cam->setFocalPoint(sphere0Geom->getPosition());

    // Run
    sdk->setCurrentScene(scene);
    sdk->startSimulation(false);
#endif
}

void testObjectController()
{
#ifdef iMSTK_USE_OPENHAPTICS
    // SDK and Scene
    auto sdk = std::make_shared<imstk::SimulationManager>();
    auto scene = sdk->createNewScene("SceneTestDevice");

    // Device Client
    auto client = std::make_shared<imstk::HDAPIDeviceClient>("Default PHANToM");

    // Device Server
    auto server = std::make_shared<imstk::HDAPIDeviceServer>();
    server->addDeviceClient(client);
    sdk->addModule(server);

    // Object
    auto geom = std::make_shared<imstk::Cube>();
    geom->setPosition(imstk::UP_VECTOR);
    geom->scale(2);

    auto object = std::make_shared<imstk::CollidingObject>("VirtualObject");
    object->setVisualGeometry(geom);
    object->setCollidingGeometry(geom);
    scene->addSceneObject(object);

    auto trackCtrl = std::make_shared<imstk::DeviceTracker>(client);
    trackCtrl->setTranslationScaling(0.1);
    auto controller = std::make_shared<imstk::SceneObjectController>(object, trackCtrl);
    scene->addObjectController(controller);

    // Update Camera position
    auto cam = scene->getCamera();
    cam->setPosition(imstk::Vec3d(0, 0, 10));
    cam->setFocalPoint(geom->getPosition());

    // Run
    sdk->setCurrentScene(scene);
    sdk->startSimulation(false);
#endif
}

void testCameraController()
{
    // SDK and Scene
    auto sdk = std::make_shared<imstk::SimulationManager>();
    auto scene = sdk->createNewScene("SceneTestDevice");

#ifdef iMSTK_USE_OPENHAPTICS

    auto client = std::make_shared<imstk::HDAPIDeviceClient>("PHANToM 1");

    // Device Server
    auto server = std::make_shared<imstk::HDAPIDeviceServer>();
    server->addDeviceClient(client);
    sdk->addModule(server);
#else
    LOG(WARNING) << "Phantom device option not enabled during build!";
#endif


    // Mesh
    auto mesh = imstk::MeshIO::read(iMSTK_DATA_ROOT"/asianDragon/asianDragon.obj");
    auto meshObject = std::make_shared<imstk::VisualObject>("meshObject");
    meshObject->setVisualGeometry(mesh);
    scene->addSceneObject(meshObject);

    // Update Camera position
    auto cam = scene->getCamera();
    cam->setPosition(imstk::Vec3d(0, 0, 10));

#ifdef iMSTK_USE_OPENHAPTICS

    // Set camera controller
    auto camController = cam->setupController(client);
    //camController->setTranslationScaling(100);
    //LOG(INFO) << camController->getTranslationOffset(); // should be the same than initial cam position
    camController->setInversionFlags(imstk::CameraController::InvertFlag::rotY |
                                     imstk::CameraController::InvertFlag::rotZ);
#endif

    // Run
    sdk->setCurrentScene(scene);
    sdk->startSimulation(true);
}

void testReadMesh()
{
    // SDK and Scene
    auto sdk = std::make_shared<imstk::SimulationManager>();
    auto scene = sdk->createNewScene("SceneTestMesh");

    // Read surface mesh
    /*auto objMesh = imstk::MeshIO::read(iMSTK_DATA_ROOT"/asianDragon/asianDragon.obj");
    auto plyMesh = imstk::MeshIO::read(iMSTK_DATA_ROOT"/cube/cube.ply");
    auto stlMesh = imstk::MeshIO::read(iMSTK_DATA_ROOT"/cube/cube.stl");
    auto vtkMesh = imstk::MeshIO::read(iMSTK_DATA_ROOT"/cube/cube.vtk");
    auto vtpMesh = imstk::MeshIO::read(iMSTK_DATA_ROOT"/cube/cube.vtp");*/

    // Read volumetricMesh
    //auto vtkMesh2 = imstk::MeshIO::read(iMSTK_DATA_ROOT"/nidus/nidus.vtk");
    auto vegaMesh = imstk::MeshIO::read(iMSTK_DATA_ROOT"/asianDragon/asianDragon.veg");

    // Extract surface mesh
    auto volumeMesh = std::dynamic_pointer_cast<imstk::VolumetricMesh>(vegaMesh); // change to any volumetric mesh above
    volumeMesh->computeAttachedSurfaceMesh();
    auto surfaceMesh = volumeMesh->getAttachedSurfaceMesh();

    // Create object and add to scene
    auto object = std::make_shared<imstk::VisualObject>("meshObject");
    object->setVisualGeometry(surfaceMesh); // change to any mesh created above
    scene->addSceneObject(object);

    // Run
    sdk->setCurrentScene(scene);
    sdk->startSimulation(true);
}

void testViewer()
{
    // SDK and Scene
    auto sdk = std::make_shared<imstk::SimulationManager>();
    auto sceneTest = sdk->createNewScene("SceneTest");

    // Plane
    auto planeObj = apiutils::createVisualAnalyticalSceneObject(imstk::Geometry::Type::Plane, sceneTest, "VisualPlane", 10);

    // Cube
    auto cubeObj = apiutils::createVisualAnalyticalSceneObject(
        imstk::Geometry::Type::Cube, sceneTest, "VisualCube", 0.5, Vec3d(1.0, -1.0, 0.5));
    auto cubeGeom = cubeObj->getVisualGeometry();
    cubeGeom->rotate(imstk::UP_VECTOR, imstk::PI_4);
    cubeGeom->rotate(imstk::RIGHT_VECTOR, imstk::PI_4);

    // Sphere
    auto sphereObj = apiutils::createVisualAnalyticalSceneObject(
        imstk::Geometry::Type::Sphere, sceneTest, "VisualSphere", 0.3, Vec3d(0, 2., 0));

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
    sdk->setCurrentScene(sceneTest);
    sdk->startSimulation(true);
}

void testAnalyticalGeometry()
{
    auto sdk = std::make_shared<imstk::SimulationManager>();

    // Plane
    LOG(INFO) << "-- Plane : Init";
    auto pos = imstk::Vec3d(5, 2, 5);
    auto norm = imstk::Vec3d(0, 1, 1);
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
    sdk->setCurrentScene(scene1);
    sdk->startSimulation();
    std::this_thread::sleep_for(std::chrono::seconds(delay));
    sdk->setCurrentScene(scene2, false);
    std::this_thread::sleep_for(std::chrono::seconds(delay));
    sdk->setCurrentScene(scene1, true);
    std::this_thread::sleep_for(std::chrono::seconds(delay));
    sdk->endSimulation();

    // pause/run
    LOG(INFO) << "-- Test simulation pause/run";
    sdk->setCurrentScene(scene2);
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
    sdk->setCurrentScene(geometryMapTest);
    sdk->startSimulation(imstk::VTKRenderer::Mode::DEBUG);
}

void testTetraTriangleMap()
{
    auto sdk = std::make_shared<imstk::SimulationManager>();

    // Tetrahedral mesh
    auto tetMesh = std::make_shared<imstk::TetrahedralMesh>();
    imstk::StdVectorOfVec3d vertList;
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
    imstk::StdVectorOfVec3d SurfVertList;
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
    imstk::StdVectorOfVec3d vertList;
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
    imstk::StdVectorOfVec3d vertList;
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
    imstk::StdVectorOfVec3d SurfVertList;
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
    imstk::StdVectorOfVec3d vertList;
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

    for (int i = 0; i < 8; ++i)
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

void testDeformableBody()
{
    // a. SDK and Scene
    auto sdk = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("DeformableBodyTest");
    scene->getCamera()->setPosition(0, 2.0, 40.0);

    // b. Load a tetrahedral mesh
    auto tetMesh = imstk::MeshIO::read(iMSTK_DATA_ROOT"/oneTet/oneTet.veg");
    //auto tetMesh = imstk::MeshIO::read(iMSTK_DATA_ROOT"/asianDragon/asianDragon.veg");
    //auto tetMesh = imstk::MeshIO::read(iMSTK_DATA_ROOT"/liver/liver.veg");
    //auto tetMesh = imstk::MeshIO::read(iMSTK_DATA_ROOT"/oneTet/oneTet.veg");
    if (!tetMesh)
    {
        LOG(WARNING) << "Could not read mesh from file.";
        return;
    }

    // c. Extract the surface mesh
    auto surfMesh = std::make_shared<imstk::SurfaceMesh>();
    auto volTetMesh = std::dynamic_pointer_cast<imstk::TetrahedralMesh>(tetMesh);
    if (!volTetMesh)
    {
        LOG(WARNING) << "Dynamic pointer cast from imstk::Mesh to imstk::TetrahedralMesh failed!";
        return;
    }
    volTetMesh->extractSurfaceMesh(surfMesh);

    imstk::StopWatch wct;
    imstk::CpuTimer cput;

    wct.start();
    cput.start();

    // d. Construct a map

    // d.1 Construct one to one nodal map based on the above meshes
    auto oneToOneNodalMap = std::make_shared<imstk::OneToOneMap>();
    oneToOneNodalMap->setMaster(tetMesh);
    oneToOneNodalMap->setSlave(surfMesh);

    // d.2 Compute the map
    oneToOneNodalMap->compute();

    LOG(INFO) << "wall clock time: " << wct.getTimeElapsed() << " ms.";
    LOG(INFO) << "CPU time: " << cput.getTimeElapsed() << " ms.";

    // e. Scene object 1: Dragon

    // Configure dynamic model
    auto dynaModel = std::make_shared<FEMDeformableBodyModel>();
    dynaModel->configure(iMSTK_DATA_ROOT"/oneTet/oneTet.config");
    //dynaModel->configure(iMSTK_DATA_ROOT"/asianDragon/asianDragon.config");
    //dynaModel->configure(iMSTK_DATA_ROOT"/liver/liver.config");
    dynaModel->initialize(volTetMesh);
    auto timeIntegrator = std::make_shared<BackwardEuler>(0.001);// Create and add Backward Euler time integrator
    dynaModel->setTimeIntegrator(timeIntegrator);

    // Scene Object
    auto deformableObj = std::make_shared<DeformableObject>("Dragon");
    deformableObj->setVisualGeometry(surfMesh);
    //deformableObj->setCollidingGeometry(surfMesh);
    deformableObj->setPhysicsGeometry(volTetMesh);
    deformableObj->setPhysicsToVisualMap(oneToOneNodalMap); //assign the computed map
    deformableObj->setDynamicalModel(dynaModel);
    deformableObj->initialize();
    scene->addSceneObject(deformableObj);

    // f. Scene object 2: Plane
    auto planeGeom = std::make_shared<Plane>();
    planeGeom->scale(40);
    planeGeom->translate(0, -6, 0);
    auto planeObj = std::make_shared<CollidingObject>("Plane");
    planeObj->setVisualGeometry(planeGeom);
    planeObj->setCollidingGeometry(planeGeom);
    scene->addSceneObject(planeObj);

    // g. Add collision detection
    //auto collisioDet = std::make_shared<CollisionDetection>();

    // h. Add collision handling

    // create a nonlinear system
    auto nlSystem = std::make_shared<NonLinearSystem>(
        dynaModel->getFunction(),
        dynaModel->getFunctionGradient());

    std::vector<LinearProjectionConstraint> projList;
    for (auto i : dynaModel->getFixNodeIds())
    {
        auto s = LinearProjectionConstraint(i, false);
        s.setProjectorToDirichlet(i);
        s.setValue(Vec3d(0.001, 0, 0));
        projList.push_back(s);
    }
    nlSystem->setLinearProjectors(projList);

    nlSystem->setUnknownVector(dynaModel->getUnknownVec());
    nlSystem->setUpdateFunction(dynaModel->getUpdateFunction());
    nlSystem->setUpdatePreviousStatesFunction(dynaModel->getUpdatePrevStateFunction());

    // create a linear solver
    auto cgLinSolver = std::make_shared<ConjugateGradient>();

    // create a non-linear solver and add to the scene
    auto nlSolver = std::make_shared<NewtonSolver>();
    nlSolver->setLinearSolver(cgLinSolver);
    nlSolver->setSystem(nlSystem);
    //nlSolver->setToFullyImplicit();
    scene->addNonlinearSolver(nlSolver);

    // Run the simulation
    sdk->setCurrentScene(scene);
    sdk->startSimulation(true);
}

void testVectorPlotters()
{
    Vectord a;
    a.resize(100);
    a.setConstant(1.0001);

    Vectord b;
    b.resize(100);
    b.setConstant(2.0);

    plotterutils::writePlotterVectorMatlab(a, "plotX.m");
    plotterutils::writePlotterVecVsVecMatlab(a, b, "plotXvsY.m");

    plotterutils::writePlotterVectorMatPlotlib(a, "plotX.py");
    plotterutils::writePlotterVecVsVecMatPlotlib(a, b, "plotXvsY.py");

    getchar();
}

void testPbdVolume()
{
    auto sdk = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("PositionBasedDynamicsTest");
    scene->getCamera()->setPosition(0, 2.0, 15.0);

    // b. Load a tetrahedral mesh
    auto tetMesh = imstk::MeshIO::read(iMSTK_DATA_ROOT"/asianDragon/asianDragon.veg");
    if (!tetMesh)
    {
        LOG(WARNING) << "Could not read mesh from file.";
        return;
    }

    // c. Extract the surface mesh
    auto surfMesh = std::make_shared<imstk::SurfaceMesh>();
    auto volTetMesh = std::dynamic_pointer_cast<imstk::TetrahedralMesh>(tetMesh);
    if (!volTetMesh)
    {
        LOG(WARNING) << "Dynamic pointer cast from imstk::Mesh to imstk::TetrahedralMesh failed!";
        return;
    }
    volTetMesh->extractSurfaceMesh(surfMesh);

    // d. Construct a map

    // d.1 Construct one to one nodal map based on the above meshes
    auto oneToOneNodalMap = std::make_shared<imstk::OneToOneMap>();
    oneToOneNodalMap->setMaster(tetMesh);
    oneToOneNodalMap->setSlave(surfMesh);

    // d.2 Compute the map
    oneToOneNodalMap->compute();

    auto deformableObj = std::make_shared<PbdObject>("Beam");
    auto pbdModel = std::make_shared<PbdModel>();

    deformableObj->setDynamicalModel(pbdModel);
    deformableObj->setVisualGeometry(surfMesh);
    deformableObj->setPhysicsGeometry(volTetMesh);
    deformableObj->setPhysicsToVisualMap(oneToOneNodalMap); //assign the computed map

    deformableObj->initialize(/*Number of Constraints*/1,
        /*Constraint configuration*/"FEM NeoHookean 100.0 0.3",
        /*Mass*/1.0,
        /*Gravity*/"0 -9.8 0",
        /*TimeStep*/0.01,
        /*FixedPoint*/"51 127 178",
        /*NumberOfIterationInConstraintSolver*/5
        );

    auto pbdSolver = std::make_shared<PbdSolver>();
    pbdSolver->setPbdObject(deformableObj);
    scene->addNonlinearSolver(pbdSolver);

    scene->addSceneObject(deformableObj);


    auto planeGeom = std::make_shared<Plane>();
    planeGeom->scale(40);
    planeGeom->translate(0, -6, 0);
    auto planeObj = std::make_shared<CollidingObject>("Plane");
    planeObj->setVisualGeometry(planeGeom);
    planeObj->setCollidingGeometry(planeGeom);
    scene->addSceneObject(planeObj);

    sdk->setCurrentScene(scene);
    sdk->startSimulation(true);

}

void testPbdCloth()
{
    auto sdk = std::make_shared<imstk::SimulationManager>();
    auto scene = sdk->createNewScene("PositionBasedDynamicsTest");
    scene->getCamera()->setPosition(6.0, 2.0, 20.0);
    scene->getCamera()->setFocalPoint(0, -5, 5);

    // a. Construct a sample triangular mesh

    // b. Add nodal data
    auto surfMesh = std::make_shared<imstk::SurfaceMesh>();
    imstk::StdVectorOfVec3d vertList;
    const double width = 10.0;
    const double height = 10.0;
    const int nRows = 11;
    const int nCols = 11;
    vertList.resize(nRows*nCols);
    const double dy = width / (double)(nCols - 1);
    const double dx = height / (double)(nRows - 1);
    for (int i = 0; i < nRows; ++i)
    {
        for (int j = 0; j < nCols; j++)
        {
            vertList[i*nCols + j] = Vec3d((double)dx*i, 1.0, (double)dy*j);
        }
    }
    surfMesh->setInitialVerticesPositions(vertList);
    surfMesh->setVerticesPositions(vertList);

    // c. Add connectivity data
    std::vector<imstk::SurfaceMesh::TriangleArray> triangles;
    for (std::size_t i = 0; i < nRows - 1; ++i)
    {
        for (std::size_t j = 0; j < nCols - 1; j++)
        {
            imstk::SurfaceMesh::TriangleArray tri[2];
            tri[0] = { { i*nCols + j, (i + 1)*nCols + j, i*nCols + j + 1 } };
            tri[1] = { { (i + 1)*nCols + j + 1, i*nCols + j + 1, (i + 1)*nCols + j } };
            triangles.push_back(tri[0]);
            triangles.push_back(tri[1]);
        }
    }

    surfMesh->setTrianglesVertices(triangles);

    // Object & Model
    auto deformableObj = std::make_shared<PbdObject>("Cloth");
    auto pbdModel = std::make_shared<PbdModel>();
    deformableObj->setDynamicalModel(pbdModel);
    deformableObj->setVisualGeometry(surfMesh);
    deformableObj->setPhysicsGeometry(surfMesh);
    deformableObj->initialize(/*Number of constraints*/2,
        /*Constraint configuration*/"Distance 0.1",
        /*Constraint configuration*/"Dihedral 0.001",
        /*Mass*/1.0,
        /*Gravity*/"0 -9.8 0",
        /*TimeStep*/0.01,
        /*FixedPoint*/"1 2 3 4 5 6 7 8 9 10 11",
        /*NumberOfIterationInConstraintSolver*/5
        );

    // Solver
    auto pbdSolver = std::make_shared<PbdSolver>();
    pbdSolver->setPbdObject(deformableObj);
    scene->addNonlinearSolver(pbdSolver);

    // Light (white)
    auto whiteLight = std::make_shared<imstk::Light>("whiteLight");
    whiteLight->setPosition(imstk::Vec3d(10, 2, 10));
    whiteLight->setFocalPoint(imstk::Vec3d(0, -2, 0));
    whiteLight->setPositional();

    // Light (red)
    auto colorLight = std::make_shared<imstk::Light>("colorLight");
    colorLight->setPosition(imstk::Vec3d(5, -3, 5));
    colorLight->setFocalPoint(imstk::Vec3d(-5, -5, 0));
    colorLight->setColor(imstk::Color::Red);
    colorLight->setPositional();
    colorLight->setSpotAngle(15);

    // Add in scene
    scene->addLight(whiteLight);
    scene->addLight(colorLight);
    scene->addSceneObject(deformableObj);

    sdk->setCurrentScene(scene);
    sdk->startSimulation(true);
}

void testPbdCollision()
{
    auto sdk = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("PbdCollisionTest");

    scene->getCamera()->setPosition(0, 10.0, 25.0);

    // dragon
    auto tetMesh = imstk::MeshIO::read(iMSTK_DATA_ROOT"/asianDragon/asianDragon.veg");
    if (!tetMesh)
    {
        LOG(WARNING) << "Could not read mesh from file.";
        return;
    }

    auto surfMesh = std::make_shared<imstk::SurfaceMesh>();
    auto surfMeshVisual = std::make_shared<imstk::SurfaceMesh>();
    auto volTetMesh = std::dynamic_pointer_cast<imstk::TetrahedralMesh>(tetMesh);
    if (!volTetMesh)
    {
        LOG(WARNING) << "Dynamic pointer cast from imstk::Mesh to imstk::TetrahedralMesh failed!";
        return;
    }
    volTetMesh->extractSurfaceMesh(surfMesh);
    volTetMesh->extractSurfaceMesh(surfMeshVisual);

    auto deformMapP2V = std::make_shared<imstk::OneToOneMap>();
    deformMapP2V->setMaster(tetMesh);
    deformMapP2V->setSlave(surfMeshVisual);
    deformMapP2V->compute();

    auto deformMapC2V = std::make_shared<imstk::OneToOneMap>();
    deformMapC2V->setMaster(surfMesh);
    deformMapC2V->setSlave(surfMeshVisual);
    deformMapC2V->compute();

    auto deformMapP2C = std::make_shared<imstk::OneToOneMap>();
    deformMapP2C->setMaster(tetMesh);
    deformMapP2C->setSlave(surfMesh);
    deformMapP2C->compute();

    auto deformableObj = std::make_shared<PbdObject>("Dragon");
    deformableObj->setVisualGeometry(surfMeshVisual);
    deformableObj->setCollidingGeometry(surfMesh);
    deformableObj->setPhysicsGeometry(volTetMesh);
    deformableObj->setPhysicsToCollidingMap(deformMapP2C);
    deformableObj->setPhysicsToVisualMap(deformMapP2V);
    deformableObj->setCollidingToVisualMap(deformMapC2V);

    auto pbdModel = std::make_shared<PbdModel>();
    deformableObj->setDynamicalModel(pbdModel);

    deformableObj->initialize(/*Number of Constraints*/1,
        /*Constraint configuration*/"FEM NeoHookean 1.0 0.3",
        /*Mass*/1.0,
        /*Gravity*/"0 -9.8 0",
        /*TimeStep*/0.001,
        /*FixedPoint*/"",
        /*NumberOfIterationInConstraintSolver*/2,
        /*Proximity*/0.1,
        /*Contact stiffness*/0.01);

    auto pbdSolver = std::make_shared<PbdSolver>();
    pbdSolver->setPbdObject(deformableObj);
    scene->addNonlinearSolver(pbdSolver);

    scene->addSceneObject(deformableObj);

    bool clothTest = 0;
    bool volumetric = !clothTest;
    if (clothTest){
        auto clothMesh = std::make_shared<imstk::SurfaceMesh>();
        imstk::StdVectorOfVec3d vertList;
        double width = 60.0;
        double height = 60.0;
        int nRows = 10;
        int nCols = 10;
        int corner[4] = { 1, nRows, nRows*nCols - nCols + 1, nRows*nCols };
        char intStr[33];
        std::string fixed_corner;
        for (unsigned int i = 0; i < 4; ++i)
        {
            std::sprintf(intStr, "%d", corner[i]);
            fixed_corner += std::string(intStr) + ' ';
        }
        vertList.resize(nRows*nCols);
        const double dy = width / (double)(nCols - 1);
        const double dx = height / (double)(nRows - 1);
        for (int i = 0; i < nRows; ++i)
        {
            for (int j = 0; j < nCols; j++)
            {
                const double y = (double)dy*j;
                const double x = (double)dx*i;
                vertList[i*nCols + j] = Vec3d(x - 30, -10, y - 30);

            }
        }
        clothMesh->setInitialVerticesPositions(vertList);
        clothMesh->setVerticesPositions(vertList);

        // c. Add connectivity data
        std::vector<imstk::SurfaceMesh::TriangleArray> triangles;
        for (std::size_t i = 0; i < nRows - 1; ++i)
        {
            for (std::size_t j = 0; j < nCols - 1; j++)
            {
                imstk::SurfaceMesh::TriangleArray tri[2];
                tri[0] = { { i*nCols + j, i*nCols + j + 1, (i + 1)*nCols + j } };
                tri[1] = { { (i + 1)*nCols + j + 1, (i + 1)*nCols + j, i*nCols + j + 1 } };
                triangles.push_back(tri[0]);
                triangles.push_back(tri[1]);
            }
        }
        clothMesh->setTrianglesVertices(triangles);

        auto oneToOneFloor = std::make_shared<imstk::OneToOneMap>();
        oneToOneFloor->setMaster(clothMesh);
        oneToOneFloor->setSlave(clothMesh);
        oneToOneFloor->compute();

        auto floor = std::make_shared<PbdObject>("Floor");
        floor->setCollidingGeometry(clothMesh);
        floor->setVisualGeometry(clothMesh);
        floor->setPhysicsGeometry(clothMesh);
        floor->setPhysicsToCollidingMap(oneToOneFloor);
        floor->setPhysicsToVisualMap(oneToOneFloor);
        //floor->setCollidingToVisualMap(oneToOneFloor);
        floor->initialize(/*Number of constraints*/2,
            /*Constraint configuration*/"Distance 0.1",
            /*Constraint configuration*/"Dihedral 0.001",
            /*Mass*/0.1,
            /*Gravity*/"0 9.8 0",
            /*TimeStep*/0.002,
            /*FixedPoint*/fixed_corner.c_str(),
            /*NumberOfIterationInConstraintSolver*/5,
            /*Proximity*/0.1,
            /*Contact stiffness*/0.95);
        scene->addSceneObject(floor);

        std::cout << "nbr of vertices in cloth mesh" << clothMesh->getNumVertices() << std::endl;

        // Collisions
        auto clothTestcolGraph = scene->getCollisionGraph();
        auto pair1 = std::make_shared<PbdInteractionPair>(PbdInteractionPair(deformableObj, floor));
        pair1->setNumberOfInterations(5);

        clothTestcolGraph->addInteractionPair(pair1);

        scene->getCamera()->setPosition(0, 0, 50);
    }
    else if (0){
        auto tetMesh1 = imstk::MeshIO::read(iMSTK_DATA_ROOT"/asianDragon/asianDragon.veg");
        if (!tetMesh1)
        {
            LOG(WARNING) << "Could not read mesh from file.";
            return;
        }

        auto surfMesh1 = std::make_shared<imstk::SurfaceMesh>();
        auto surfMeshVisual1 = std::make_shared<imstk::SurfaceMesh>();
        auto volTetMesh1 = std::dynamic_pointer_cast<imstk::TetrahedralMesh>(tetMesh1);
        if (!volTetMesh1)
        {
            LOG(WARNING) << "Dynamic pointer cast from imstk::Mesh to imstk::TetrahedralMesh failed!";
            return;
        }

        auto vs = volTetMesh1->getInitialVerticesPositions();
        Vec3d tmpPos;
        for (int i = 0; i < volTetMesh1->getNumVertices(); ++i){
            tmpPos = volTetMesh1->getVertexPosition(i);
            tmpPos[1] -= 6;
            volTetMesh1->setVerticePosition(i, tmpPos);
        }
        volTetMesh1->setInitialVerticesPositions(volTetMesh1->getVertexPositions());

        volTetMesh1->extractSurfaceMesh(surfMesh1);
        volTetMesh1->extractSurfaceMesh(surfMeshVisual1);


        auto deformMapP2V1 = std::make_shared<imstk::OneToOneMap>();
        deformMapP2V1->setMaster(volTetMesh1);
        deformMapP2V1->setSlave(surfMeshVisual1);
        deformMapP2V1->compute();

        auto deformMapC2V1 = std::make_shared<imstk::OneToOneMap>();
        deformMapC2V1->setMaster(surfMesh1);
        deformMapC2V1->setSlave(surfMeshVisual1);
        deformMapC2V1->compute();

        auto deformMapP2C1 = std::make_shared<imstk::OneToOneMap>();
        deformMapP2C1->setMaster(volTetMesh1);
        deformMapP2C1->setSlave(surfMesh1);
        deformMapP2C1->compute();

        auto deformableObj1 = std::make_shared<PbdObject>("Dragon2");
        deformableObj1->setVisualGeometry(surfMeshVisual1);
        deformableObj1->setCollidingGeometry(surfMesh1);
        deformableObj1->setPhysicsGeometry(volTetMesh1);
        deformableObj1->setPhysicsToCollidingMap(deformMapP2C1);
        deformableObj1->setPhysicsToVisualMap(deformMapP2V1);
        deformableObj1->setCollidingToVisualMap(deformMapC2V1);
        deformableObj1->initialize(/*Number of Constraints*/1,
            /*Constraint configuration*/"FEM NeoHookean 10.0 0.5",
            /*Mass*/0.0,
            /*Gravity*/"0 -9.8 0",
            /*TimeStep*/0.002,
            /*FixedPoint*/"",
            /*NumberOfIterationInConstraintSolver*/2,
            /*Proximity*/0.1,
            /*Contact stiffness*/0.01);

        scene->addSceneObject(deformableObj1);

        // Collisions
        auto colGraph = scene->getCollisionGraph();
        auto pair = std::make_shared<PbdInteractionPair>(PbdInteractionPair(deformableObj, deformableObj1));
        pair->setNumberOfInterations(2);

        colGraph->addInteractionPair(pair);
    }
    else{
        // floor

        imstk::StdVectorOfVec3d vertList;
        double width = 100.0;
        double height = 100.0;
        int nRows = 2;
        int nCols = 2;
        vertList.resize(nRows*nCols);
        const double dy = width / (double)(nCols - 1);
        const double dx = height / (double)(nRows - 1);
        for (int i = 0; i < nRows; ++i)
        {
            for (int j = 0; j < nCols; j++)
            {
                const double y = (double)dy*j;
                const double x = (double)dx*i;
                vertList[i*nCols + j] = Vec3d(x - 50, -10.0, y - 50);

            }
        }

        // c. Add connectivity data
        std::vector<imstk::SurfaceMesh::TriangleArray> triangles;
        for (std::size_t i = 0; i < nRows - 1; ++i)
        {
            for (std::size_t j = 0; j < nCols - 1; j++)
            {
                imstk::SurfaceMesh::TriangleArray tri[2];
                tri[0] = { { i*nCols + j, i*nCols + j + 1, (i + 1)*nCols + j } };
                tri[1] = { { (i + 1)*nCols + j + 1, (i + 1)*nCols + j, i*nCols + j + 1 } };
                triangles.push_back(tri[0]);
                triangles.push_back(tri[1]);
            }
        }
        auto floorMeshColliding = std::make_shared<imstk::SurfaceMesh>();
        floorMeshColliding->initialize(vertList, triangles);
        auto floorMeshVisual = std::make_shared<imstk::SurfaceMesh>();
        floorMeshVisual->initialize(vertList, triangles);
        auto floorMeshPhysics = std::make_shared<imstk::SurfaceMesh>();
        floorMeshPhysics->initialize(vertList, triangles);


        auto floorMapP2V = std::make_shared<imstk::OneToOneMap>();
        floorMapP2V->setMaster(floorMeshPhysics);
        floorMapP2V->setSlave(floorMeshVisual);
        floorMapP2V->compute();


        auto floorMapP2C = std::make_shared<imstk::OneToOneMap>();
        floorMapP2C->setMaster(floorMeshPhysics);
        floorMapP2C->setSlave(floorMeshColliding);
        floorMapP2C->compute();

        auto floorMapC2V = std::make_shared<imstk::OneToOneMap>();
        floorMapC2V->setMaster(floorMeshColliding);
        floorMapC2V->setSlave(floorMeshVisual);
        floorMapC2V->compute();

        auto floor = std::make_shared<PbdObject>("Floor");
        floor->setCollidingGeometry(floorMeshColliding);
        floor->setVisualGeometry(floorMeshVisual);
        floor->setPhysicsGeometry(floorMeshPhysics);
        floor->setPhysicsToCollidingMap(floorMapP2C);
        floor->setPhysicsToVisualMap(floorMapP2V);
        floor->setCollidingToVisualMap(floorMapC2V);

        auto pbdModel2 = std::make_shared<PbdModel>();
        floor->setDynamicalModel(pbdModel2);

        floor->initialize(/*Number of Constraints*/0,
            /*Mass*/0.0,
            /*Proximity*/0.1,
            /*Contact stiffness*/1.0);

        auto pbdSolverfloor = std::make_shared<PbdSolver>();
        pbdSolverfloor->setPbdObject(floor);
        scene->addNonlinearSolver(pbdSolverfloor);

        scene->addSceneObject(floor);

        // Collisions
        auto colGraph = scene->getCollisionGraph();
        auto pair = std::make_shared<PbdInteractionPair>(PbdInteractionPair(deformableObj, floor));
        pair->setNumberOfInterations(2);

        colGraph->addInteractionPair(pair);
    }
    sdk->setCurrentScene(scene);
    sdk->startSimulation(true);
}

void testLineMesh()
{
#ifdef iMSTK_USE_OPENHAPTICS
    // SDK and Scene
    auto sdk = std::make_shared<imstk::SimulationManager>();
    auto scene = sdk->createNewScene("TestLineMesh");

    // Device clients
    auto client0 = std::make_shared<imstk::HDAPIDeviceClient>("PHANToM 1");

    // Device Server
    auto server = std::make_shared<imstk::HDAPIDeviceServer>();
    server->addDeviceClient(client0);
    sdk->addModule(server);

    auto blade = std::make_shared<imstk::VirtualCouplingPBDObject>("blade", client0);
    auto linesTool = std::make_shared<imstk::VirtualCouplingPBDObject>("linesTool", client0);
    auto tool = std::make_shared<imstk::VirtualCouplingPBDObject>("tool", client0);

    bool line;
    bool clothTest;

    std::cout << "Select tool: 0 for blade, 1 for lines..." << std::endl;
    std::cin >> line;
    std::cout << "Select deformable: 0 for volumetric mesh, 1 for surface mesh..." << std::endl;
    std::cin >> clothTest;

    if (line){
        // Read LineMesh
        auto lineMeshColliding = std::make_shared<imstk::LineMesh>();
        auto lineMeshVisual = std::make_shared<imstk::LineMesh>();
        auto lineMeshPhysics = std::make_shared<imstk::LineMesh>();

        imstk::StdVectorOfVec3d vertList;
        vertList.resize(3);
        vertList[0] = Vec3d(0.0, -10.0, -10.0);
        vertList[1] = Vec3d(0.0, 0.0, -10.0);
        vertList[2] = Vec3d(0.0, 0.0, -30.0);
        std::vector<std::vector<int> > connectivity;
        for (int i = 0; i < 2;){
            std::vector<int> line;
            line.push_back(i);
            i++;
            line.push_back(i);

            connectivity.push_back(line);
        }

        lineMeshColliding->setInitialVerticesPositions(vertList);
        lineMeshColliding->setVerticesPositions(vertList);
        lineMeshColliding->setConnectivity(connectivity);

        lineMeshPhysics->setInitialVerticesPositions(vertList);
        lineMeshPhysics->setVerticesPositions(vertList);
        lineMeshPhysics->setConnectivity(connectivity);

        lineMeshVisual->setInitialVerticesPositions(vertList);
        lineMeshVisual->setVerticesPositions(vertList);
        lineMeshVisual->setConnectivity(connectivity);


        auto mapC2P = std::make_shared<imstk::OneToOneMap>();
        mapC2P->setMaster(lineMeshColliding);
        mapC2P->setSlave(lineMeshPhysics);
        mapC2P->compute();

        auto mapC2V = std::make_shared<imstk::OneToOneMap>();
        mapC2V->setMaster(lineMeshColliding);
        mapC2V->setSlave(lineMeshVisual);
        mapC2V->compute();

        auto mapP2C = std::make_shared<imstk::OneToOneMap>();
        mapP2C->setMaster(lineMeshPhysics);
        mapP2C->setSlave(lineMeshColliding);
        mapP2C->compute();

        auto mapP2V = std::make_shared<imstk::OneToOneMap>();
        mapP2V->setMaster(lineMeshPhysics);
        mapP2V->setSlave(lineMeshVisual);
        mapP2V->compute();

        linesTool->setCollidingGeometry(lineMeshColliding);
        linesTool->setVisualGeometry(lineMeshVisual);
        linesTool->setPhysicsGeometry(lineMeshPhysics);
        linesTool->setPhysicsToCollidingMap(mapP2C);
        linesTool->setCollidingToVisualMap(mapC2V);
        linesTool->setPhysicsToVisualMap(mapP2V);
        linesTool->setColldingToPhysicsMap(mapC2P);
        linesTool->initialize(/*Number of constraints*/1,
            /*Constraint configuration*/"Distance 100",
            /*Mass*/0.0,
            /*Gravity*/"0 -9.8 0",
            /*TimeStep*/0.002,
            /*FixedPoint*/"0 1 2",
            /*NumberOfIterationInConstraintSolver*/5,
            /*Proximity*/0.1,
            /*Contact stiffness*/0.1);
        scene->addSceneObject(linesTool);
    }
    else{
        std::string path2obj = iMSTK_DATA_ROOT"/ETI/resources/Tools/blade2.obj";

        auto collidingMesh = imstk::MeshIO::read(path2obj);
        auto viusalMesh = imstk::MeshIO::read(path2obj);
        auto physicsMesh = imstk::MeshIO::read(path2obj);

        auto bladeMapP2V = std::make_shared<imstk::OneToOneMap>();
        bladeMapP2V->setMaster(physicsMesh);
        bladeMapP2V->setSlave(viusalMesh);
        bladeMapP2V->compute();

        auto bladeMapP2C = std::make_shared<imstk::OneToOneMap>();
        bladeMapP2C->setMaster(physicsMesh);
        bladeMapP2C->setSlave(collidingMesh);
        bladeMapP2C->compute();

        auto bladeMapC2V = std::make_shared<imstk::OneToOneMap>();
        bladeMapC2V->setMaster(collidingMesh);
        bladeMapC2V->setSlave(viusalMesh);
        bladeMapC2V->compute();

        auto bladeMapC2P = std::make_shared<imstk::OneToOneMap>();
        bladeMapC2P->setMaster(collidingMesh);
        bladeMapC2P->setSlave(physicsMesh);
        bladeMapC2P->compute();

        blade->setCollidingGeometry(collidingMesh);
        blade->setVisualGeometry(viusalMesh);
        blade->setPhysicsGeometry(physicsMesh);
        blade->setPhysicsToCollidingMap(bladeMapP2C);
        blade->setCollidingToVisualMap(bladeMapC2V);
        blade->setPhysicsToVisualMap(bladeMapP2V);
        blade->setColldingToPhysicsMap(bladeMapC2P);
        blade->initialize(/*Number of constraints*/1,
            /*Constraint configuration*/"Distance 0.1",
            /*Mass*/0.0,
            /*Gravity*/"0 0 0",
            /*TimeStep*/0.001,
            /*FixedPoint*/"",
            /*NumberOfIterationInConstraintSolver*/5,
            /*Proximity*/0.1,
            /*Contact stiffness*/0.01);
        scene->addSceneObject(blade);
    }



    if (clothTest){

        imstk::StdVectorOfVec3d vertList;
        double width = 60.0;
        double height = 60.0;
        int nRows = 20;
        int nCols = 20;
        int corner[4] = { 1, nRows, nRows*nCols - nCols + 1, nRows*nCols };
        char intStr[33];
        std::string fixed_corner;
        for (unsigned int i = 0; i < 4; ++i)
        {
            std::sprintf(intStr, "%d", corner[i]);
            fixed_corner += std::string(intStr) + ' ';
        }
        vertList.resize(nRows*nCols);
        const double dy = width / (double)(nCols - 1);
        const double dx = height / (double)(nRows - 1);
        for (int i = 0; i < nRows; ++i)
        {
            for (int j = 0; j < nCols; j++)
            {
                const double y = (double)dy*j;
                const double x = (double)dx*i;
                vertList[i*nCols + j] = Vec3d(x - 30, -25, y - 60);

            }
        }

        // c. Add connectivity data
        std::vector<imstk::SurfaceMesh::TriangleArray> triangles;
        for (size_t i = 0; i < nRows - 1; ++i)
        {
            for (size_t j = 0; j < nCols - 1; j++)
            {
                imstk::SurfaceMesh::TriangleArray tri[2];
                tri[0] = { { i*nCols + j, i*nCols + j + 1, (i + 1)*nCols + j } };
                tri[1] = { { (i + 1)*nCols + j + 1, (i + 1)*nCols + j, i*nCols + j + 1 } };
                triangles.push_back(tri[0]);
                triangles.push_back(tri[1]);
            }
        }

        auto clothMeshVisual = std::make_shared<imstk::SurfaceMesh>();
        clothMeshVisual->initialize(vertList, triangles);
        auto clothMeshColliding = std::make_shared<imstk::SurfaceMesh>();
        clothMeshColliding->initialize(vertList, triangles);
        auto clothMeshPhysics = std::make_shared<imstk::SurfaceMesh>();
        clothMeshPhysics->initialize(vertList, triangles);

        auto clothMapP2V = std::make_shared<imstk::OneToOneMap>();
        clothMapP2V->setMaster(clothMeshPhysics);
        clothMapP2V->setSlave(clothMeshVisual);
        clothMapP2V->compute();

        auto clothMapC2V = std::make_shared<imstk::OneToOneMap>();
        clothMapC2V->setMaster(clothMeshColliding);
        clothMapC2V->setSlave(clothMeshVisual);
        clothMapC2V->compute();

        auto clothMapP2C = std::make_shared<imstk::OneToOneMap>();
        clothMapP2C->setMaster(clothMeshPhysics);
        clothMapP2C->setSlave(clothMeshColliding);
        clothMapP2C->compute();


        auto floor = std::make_shared<PbdObject>("cloth");
        floor->setCollidingGeometry(clothMeshColliding);
        floor->setVisualGeometry(clothMeshVisual);
        floor->setPhysicsGeometry(clothMeshPhysics);
        floor->setPhysicsToCollidingMap(clothMapP2C);
        floor->setPhysicsToVisualMap(clothMapP2V);
        floor->setCollidingToVisualMap(clothMapC2V);
        floor->initialize(/*Number of constraints*/2,
            /*Constraint configuration*/"Distance 0.1",
            /*Constraint configuration*/"Dihedral 0.001",
            /*Mass*/0.1,
            /*Gravity*/"0 -9.8 0",
            /*TimeStep*/0.001,
            /*FixedPoint*/fixed_corner.c_str(),
            /*NumberOfIterationInConstraintSolver*/5,
            /*Proximity*/0.1,
            /*Contact stiffness*/0.1);
        scene->addSceneObject(floor);

        std::cout << "nbr of vertices in cloth mesh" << clothMeshVisual->getNumVertices() << std::endl;

        // Collisions
        auto clothTestcolGraph = scene->getCollisionGraph();
        if (line)
            tool = linesTool;
        else
            tool = blade;

        auto pair1 = std::make_shared<PbdInteractionPair>(PbdInteractionPair(tool, floor));
        pair1->setNumberOfInterations(5);

        clothTestcolGraph->addInteractionPair(pair1);

        scene->getCamera()->setPosition(0, 0, 50);
    }
    else{
        //auto tetMesh = imstk::MeshIO::read(iMSTK_DATA_ROOT"/ETI/resources/Human/tongue.veg");
        auto tetMesh = imstk::MeshIO::read(iMSTK_DATA_ROOT"/asianDragon/asianDragon.veg");
        if (!tetMesh)
        {
            LOG(WARNING) << "Could not read mesh from file.";
            return;
        }

        auto volTetMesh = std::dynamic_pointer_cast<imstk::TetrahedralMesh>(tetMesh);
        if (!volTetMesh)
        {
            LOG(WARNING) << "Dynamic pointer cast from imstk::Mesh to imstk::TetrahedralMesh failed!";
            return;
        }

        auto vs = volTetMesh->getInitialVerticesPositions();
        Vec3d tmpPos;
        for (int i = 0; i < volTetMesh->getNumVertices(); ++i)
        {
            tmpPos = volTetMesh->getVertexPosition(i);
            tmpPos[1] -= 15;
            volTetMesh->setVerticePosition(i, tmpPos);
        }
        volTetMesh->setInitialVerticesPositions(volTetMesh->getVertexPositions());

        auto surfMesh = std::make_shared<imstk::SurfaceMesh>();
        volTetMesh->extractSurfaceMesh(surfMesh);

        auto surfMeshVisual = std::make_shared<imstk::SurfaceMesh>();
        volTetMesh->extractSurfaceMesh(surfMeshVisual);

        auto dragonMapP2V = std::make_shared<imstk::OneToOneMap>();
        dragonMapP2V->setMaster(volTetMesh);
        dragonMapP2V->setSlave(surfMeshVisual);
        dragonMapP2V->compute();

        auto dragonMapC2V = std::make_shared<imstk::OneToOneMap>();
        dragonMapC2V->setMaster(surfMesh);
        dragonMapC2V->setSlave(surfMeshVisual);
        dragonMapC2V->compute();

        auto dragonMapP2C = std::make_shared<imstk::OneToOneMap>();
        dragonMapP2C->setMaster(volTetMesh);
        dragonMapP2C->setSlave(surfMesh);
        dragonMapP2C->compute();


        auto deformableObj = std::make_shared<PbdObject>("Dragon");
        deformableObj->setVisualGeometry(surfMeshVisual);
        deformableObj->setCollidingGeometry(surfMesh);
        deformableObj->setPhysicsGeometry(volTetMesh);
        deformableObj->setPhysicsToCollidingMap(dragonMapP2C);
        deformableObj->setPhysicsToVisualMap(dragonMapP2V);
        deformableObj->setCollidingToVisualMap(dragonMapC2V);
        deformableObj->initialize(/*Number of Constraints*/1,
            /*Constraint configuration*/"FEM NeoHookean 10.0 0.3",
            /*Mass*/0.1,
            /*Gravity*/"0 0 0",
            /*TimeStep*/0.001,
            /*FixedPoint*/"",
            /*NumberOfIterationInConstraintSolver*/5,
            /*Proximity*/0.1,
            /*Contact stiffness*/0.01);

        scene->addSceneObject(deformableObj);
        std::cout << "nbr of vertices in tongue mesh = " << surfMesh->getNumVertices() << std::endl;

        // Collisions
        auto deformableColGraph = scene->getCollisionGraph();
        tool = line ? linesTool : blade;

        auto pair1 = std::make_shared<PbdInteractionPair>(PbdInteractionPair(tool, deformableObj));
        pair1->setNumberOfInterations(10);
        deformableColGraph->addInteractionPair(pair1);

        scene->getCamera()->setPosition(0, 5, 5);
        scene->getCamera()->setFocalPoint(surfMesh.get()->getInitialVertexPosition(20));
    }
    // Run
    sdk->setCurrentScene(scene);
    sdk->startSimulation(true);
#endif
}

void testScreenShotUtility()
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
    cubeGeom->translate(1.0, -1.0, 0.5);
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

    // Set up for screen shot
    sdk->getViewer()->getScreenCaptureUtility()->setScreenShotPrefix("screenShot_");
    // Create a call back on key press of 'b' to take the screen shot
    sdk->getViewer()->setOnCharFunction('b', [&](VTKInteractorStyle* c) -> bool
    {
        sdk->getViewer()->getScreenCaptureUtility()->saveScreenShot();
        return false;
    });

    // Run
    sdk->setCurrentScene(sceneTest);
    sdk->startSimulation(true);
}

void testDeformableBodyCollision()
{
    // SDK and Scene
    auto sdk = std::make_shared<imstk::SimulationManager>();
    auto scene = sdk->createNewScene("OneTetraCH");

    auto geom = std::make_shared<imstk::Plane>(WORLD_ORIGIN, -UP_VECTOR, 1.);

    geom->scale(100);
    geom->translate(Vec3d(0., -20., 0.));

    auto planeObj = std::make_shared<imstk::CollidingObject>("VisualPlane");
    planeObj->setVisualGeometry(geom);
    planeObj->setCollidingGeometry(geom);
    scene->addSceneObject(planeObj);

    // Load a tetrahedral mesh
    auto tetMesh = imstk::MeshIO::read(iMSTK_DATA_ROOT"/oneTet/oneTet.veg");
    if (!tetMesh)
    {
        LOG(WARNING) << "Could not read mesh from file.";
        return;
    }

    // Extract the surface mesh
    auto surfMesh = std::make_shared<imstk::SurfaceMesh>();
    auto volTetMesh = std::dynamic_pointer_cast<imstk::TetrahedralMesh>(tetMesh);
    if (!volTetMesh)
    {
        LOG(WARNING) << "Dynamic pointer cast from imstk::Mesh to imstk::TetrahedralMesh failed!";
        return;
    }
    volTetMesh->extractSurfaceMesh(surfMesh);

    // Construct one to one nodal map based on the above meshes
    auto oneToOneNodalMap = std::make_shared<imstk::OneToOneMap>();
    oneToOneNodalMap->setMaster(tetMesh);
    oneToOneNodalMap->setSlave(surfMesh);
    oneToOneNodalMap->compute();

    // Configure the dynamic model
    auto dynaModel = std::make_shared<FEMDeformableBodyModel>();
    dynaModel->configure(iMSTK_DATA_ROOT"/oneTet/oneTet.config");
    dynaModel->initialize(volTetMesh);

    // Create and add Backward Euler time integrator
    auto timeIntegrator = std::make_shared<BackwardEuler>(0.001);
    dynaModel->setTimeIntegrator(timeIntegrator);

    // Configure Scene Object
    auto deformableObj = std::make_shared<DeformableObject>("Liver");
    deformableObj->setVisualGeometry(surfMesh);
    deformableObj->setCollidingGeometry(volTetMesh);
    deformableObj->setPhysicsGeometry(volTetMesh);
    deformableObj->setPhysicsToVisualMap(oneToOneNodalMap);
    deformableObj->setDynamicalModel(dynaModel);
    deformableObj->initialize();
    scene->addSceneObject(deformableObj);

    // Create a nonlinear system and solver
    auto nlSystem = std::make_shared<NonLinearSystem>(dynaModel->getFunction(), dynaModel->getFunctionGradient());
    std::vector<LinearProjectionConstraint> linProj;
    for (auto id : dynaModel->getFixNodeIds())
    {
        linProj.push_back(LinearProjectionConstraint(id, true));
    }
    nlSystem->setLinearProjectors(linProj);
    nlSystem->setUnknownVector(dynaModel->getUnknownVec());
    nlSystem->setUpdateFunction(dynaModel->getUpdateFunction());
    nlSystem->setUpdatePreviousStatesFunction(dynaModel->getUpdatePrevStateFunction());

    // create a non-linear solver and add to the scene
    auto nlSolver = std::make_shared<NewtonSolver>();
    auto cgLinSolver = std::make_shared<ConjugateGradient>();// create a linear solver to be used in the NL solver
    nlSolver->setLinearSolver(cgLinSolver);
    nlSolver->setSystem(nlSystem);
    scene->addNonlinearSolver(nlSolver);

    // Create collision detection and handling
    scene->getCollisionGraph()->addInteractionPair(deformableObj,
        planeObj,
        CollisionDetection::Type::MeshToPlane,
        CollisionHandling::Type::Penalty,
        CollisionHandling::Type::None);

    // Set Camera configuration
    auto cam = scene->getCamera();
    cam->setPosition(imstk::Vec3d(0, 20, 20));
    cam->setFocalPoint(imstk::Vec3d(0, 0, 0));

    // Run
    sdk->setCurrentScene(scene);
    sdk->startSimulation(true);
}

void liverToolInteraction()
{
    // SDK and Scene
    auto sdk = std::make_shared<imstk::SimulationManager>();
    auto scene = sdk->createNewScene("LiverToolInteraction");

    //----------------------------------------------------------
    // Create plane visual scene object
    //----------------------------------------------------------
    auto planeObj = apiutils::createVisualAnalyticalSceneObject(
        imstk::Geometry::Type::Plane, scene, "VisualPlane", 100, Vec3d(0., -20., 0.));

    //----------------------------------------------------------
    // Create liver FE deformable scene object
    //----------------------------------------------------------

    // Load a tetrahedral mesh
    auto tetMesh = imstk::MeshIO::read(iMSTK_DATA_ROOT"/oneTet/oneTet.veg");
    //auto tetMesh = imstk::MeshIO::read(iMSTK_DATA_ROOT"/liver/liver.veg");

    if (!tetMesh)
    {
        LOG(WARNING) << "Could not read mesh from file.";
        return;
    }

    // Extract the surface mesh
    auto surfMesh = std::make_shared<imstk::SurfaceMesh>();
    auto volTetMesh = std::dynamic_pointer_cast<imstk::TetrahedralMesh>(tetMesh);
    if (!volTetMesh)
    {
        LOG(WARNING) << "Dynamic pointer cast from imstk::Mesh to imstk::TetrahedralMesh failed!";
        return;
    }
    volTetMesh->extractSurfaceMesh(surfMesh);

    // Construct one to one nodal map based on the above meshes
    auto oneToOneNodalMap = std::make_shared<imstk::OneToOneMap>();
    oneToOneNodalMap->setMaster(tetMesh);
    oneToOneNodalMap->setSlave(surfMesh);
    oneToOneNodalMap->compute();

    // Configure the dynamic model
    auto dynaModel = std::make_shared<FEMDeformableBodyModel>();
    //dynaModel->configure(iMSTK_DATA_ROOT"/liver/liver.config");
    dynaModel->configure(iMSTK_DATA_ROOT"/oneTet/oneTet.config");
    dynaModel->initialize(volTetMesh);

    // Create and add Backward Euler time integrator
    auto timeIntegrator = std::make_shared<BackwardEuler>(0.001);
    dynaModel->setTimeIntegrator(timeIntegrator);

    // Configure Scene Object
    auto deformableObj = std::make_shared<DeformableObject>("Liver");
    deformableObj->setVisualGeometry(surfMesh);
    deformableObj->setCollidingGeometry(volTetMesh);
    deformableObj->setPhysicsGeometry(volTetMesh);
    deformableObj->setPhysicsToVisualMap(oneToOneNodalMap);
    deformableObj->setDynamicalModel(dynaModel);
    deformableObj->initialize();
    scene->addSceneObject(deformableObj);

    //----------------------------------------------------------
    // Create a nonlinear system and its solver
    //----------------------------------------------------------
    auto nlSystem = std::make_shared<NonLinearSystem>( dynaModel->getFunction(), dynaModel->getFunctionGradient());
    std::vector<LinearProjectionConstraint> linProj;
    for (auto id : dynaModel->getFixNodeIds())
    {
        linProj.push_back(LinearProjectionConstraint(id, true));
    }
    nlSystem->setLinearProjectors(linProj);
    nlSystem->setUnknownVector(dynaModel->getUnknownVec());
    nlSystem->setUpdateFunction(dynaModel->getUpdateFunction());
    nlSystem->setUpdatePreviousStatesFunction(dynaModel->getUpdatePrevStateFunction());

    // create a non-linear solver and add to the scene
    auto nlSolver = std::make_shared<NewtonSolver>();
    auto cgLinSolver = std::make_shared<ConjugateGradient>();// create a linear solver to be used in the NL solver
    nlSolver->setLinearSolver(cgLinSolver);
    nlSolver->setSystem(nlSystem);
    //nlSolver->setToFullyImplicit();
    scene->addNonlinearSolver(nlSolver);

    //----------------------------------------------------------
    // Create collision detection and handling
    //----------------------------------------------------------
    //auto collData = std::make_shared<imstk::CollisionData>();
    /*auto collisioDet = std::make_shared<imstk::MeshToPlaneCD>(volTetMesh,
                                                              std::dynamic_pointer_cast<imstk::Plane>(planeObj->getCollidingGeometry()),
                                                              *collData.get());
    auto collHandling = std::make_shared<imstk::PenaltyMeshToRigidCH>(imstk::CollisionHandling::Side::A, *collData.get(), deformableObj);*/



    //----------------------------------------------------------
    // Create laparoscopic tool controller
    //----------------------------------------------------------
#ifdef iMSTK_USE_OPENHAPTICS

    // Device clients
    auto client = std::make_shared<imstk::HDAPIDeviceClient>("Default Device");

    // Device Server
    auto server = std::make_shared<imstk::HDAPIDeviceServer>();
    server->addDeviceClient(client);
    sdk->addModule(server);

    // Create laparoscopic tool related scene objects
    /*auto pivot = apiutils::createAndAddVisualSceneObject(scene, iMSTK_DATA_ROOT"/laptool/pivot.obj", "pivot");
    auto upperJaw = apiutils::createAndAddVisualSceneObject(scene, iMSTK_DATA_ROOT"/laptool/upper.obj", "upperJaw");
    auto lowerJaw = apiutils::createAndAddVisualSceneObject(scene, iMSTK_DATA_ROOT"/laptool/lower.obj", "lowerJaw");*/

    // Sphere0
    auto sphere0Obj = apiutils::createCollidingAnalyticalSceneObject(
        imstk::Geometry::Type::Sphere, scene, "Sphere0", 3, Vec3d(1, 0.5, 0));

    auto trackingCtrl = std::make_shared<imstk::DeviceTracker>(client);
    //trackingCtrl->setTranslationScaling(100);
    auto lapToolController = std::make_shared<imstk::SceneObjectController>(sphere0Obj, trackingCtrl);
    /*auto lapToolController = std::make_shared<imstk::LaparoscopicToolController>(pivot, upperJaw, lowerJaw, trackingCtrl);
    lapToolController->setJawRotationAxis(imstk::Vec3d(1.0, 0, 0));*/
    scene->addObjectController(lapToolController);

    scene->getCollisionGraph()->addInteractionPair(deformableObj,
                                                   sphere0Obj,
                                                   CollisionDetection::Type::MeshToSphere,
                                                   CollisionHandling::Type::Penalty,
                                                   CollisionHandling::Type::None);

#endif

    // Set Camera configuration
    auto cam = scene->getCamera();
    cam->setPosition(imstk::Vec3d(0, 20, 20));
    cam->setFocalPoint(imstk::Vec3d(0, 0, 0));

    // Run
    sdk->setCurrentScene(scene);
    sdk->startSimulation(true);
}
