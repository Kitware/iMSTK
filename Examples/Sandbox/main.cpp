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

#include "imstkMath.h"
#include "imstkTimer.h"
#include "imstkSimulationManager.h"

// Objects
#include "imstkForceModelConfig.h"
#include "imstkFEMDeformableBodyModel.h"
#include "imstkVirtualCouplingPBDObject.h"
#include "imstkDynamicObject.h"
#include "imstkDeformableObject.h"
#include "imstkPbdObject.h"
#include "imstkSceneObject.h"
#include "imstkLight.h"
#include "imstkCamera.h"
#include "imstkRigidObject.h"


#include "imstkGraph.h"

// Time Integrators
#include "imstkBackwardEuler.h"

// Solvers
#include "imstkNonlinearSystem.h"
#include "imstkNewtonSolver.h"
#include "imstkConjugateGradient.h"
#include "imstkPbdSolver.h"
#include "imstkGaussSeidel.h"
#include "imstkJacobi.h"
#include "imstkSOR.h"

// Geometry
#include "imstkPlane.h"
#include "imstkSphere.h"
#include "imstkCube.h"
#include "imstkCylinder.h"
#include "imstkTetrahedralMesh.h"
#include "imstkHexahedralMesh.h"
#include "imstkSurfaceMesh.h"
#include "imstkMeshIO.h"
#include "imstkLineMesh.h"

// Maps
#include "imstkTetraTriangleMap.h"
#include "imstkIsometricMap.h"
#include "imstkOneToOneMap.h"

// Devices and controllers
#include "imstkHDAPIDeviceClient.h"
#include "imstkHDAPIDeviceServer.h"
#include "imstkVRPNDeviceClient.h"
#include "imstkVRPNDeviceServer.h"
#include "imstkCameraController.h"
#include "imstkSceneObjectController.h"
#include "imstkLaparoscopicToolController.h"

// Collisions
#include "imstkInteractionPair.h"
#include "imstkPointSetToPlaneCD.h"
#include "imstkPointSetToSphereCD.h"
#include "imstkVirtualCouplingCH.h"
#include "imstkPointSetToSpherePickingCD.h"
#include "imstkPickingCH.h"
#include "imstkBoneDrillingCH.h"

// logger
#include "g3log/g3log.hpp"
#include "imstkLogger.h"

// imstk utilities
#include "imstkPlotterUtils.h"
#include "imstkAPIUtilities.h"

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

#ifdef iMSTK_AUDIO_ENABLED

// Audio
#include <SFML/Audio.hpp>

#endif

// global variables
const std::string phantomOmni1Name = "Phantom1";
const std::string phantomOmni2Name = "Phantom2";
const std::string novintFalcon1Name = "device0";
const std::string novintFalcon2Name = "device1";

using namespace imstk;

void testLapToolController()
{
#ifdef iMSTK_USE_OPENHAPTICS
    // SDK and Scene
    auto sdk = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("TestLapToolController");

    // Device clients
    auto client0 = std::make_shared<HDAPIDeviceClient>(phantomOmni1Name);

    // Device Server
    auto server = std::make_shared<HDAPIDeviceServer>();
    server->addDeviceClient(client0);
    sdk->addModule(server);

    // Plane
    auto planeObj = apiutils::createVisualAnalyticalSceneObject(
        Geometry::Type::Plane, scene, "VisualPlane", 100., Vec3d(0., -20., 0.));

    // laparoscopic tool
    auto pivot = apiutils::createAndAddVisualSceneObject(scene, iMSTK_DATA_ROOT "/laptool/pivot.obj", "pivot");
    auto upperJaw = apiutils::createAndAddVisualSceneObject(scene, iMSTK_DATA_ROOT "/laptool/upper.obj", "upperJaw");
    auto lowerJaw = apiutils::createAndAddVisualSceneObject(scene, iMSTK_DATA_ROOT "/laptool/lower.obj", "lowerJaw");

    auto trackingCtrl = std::make_shared<DeviceTracker>(client0);
    trackingCtrl->setTranslationScaling(0.5);
    auto lapToolController = std::make_shared<LaparoscopicToolController>(pivot, upperJaw, lowerJaw, trackingCtrl);
    lapToolController->setJawRotationAxis(Vec3d(1.0, 0, 0));
    lapToolController->setJawAngleChange(0.1);
    scene->addObjectController(lapToolController);

    // Set Camera
    auto cam = scene->getCamera();
    cam->setPosition(Vec3d(0, 30, 60));
    cam->setFocalPoint(Vec3d(0, 0, 0));

    // Light
    auto light = std::make_shared<DirectionalLight>("light");
    light->setFocalPoint(Vec3d(5, -8, -5));
    light->setIntensity(1);
    scene->addLight(light);

    // Run
    sdk->setActiveScene(scene);
    sdk->startSimulation(true);
#endif
}

void testMshAndVegaIO()
{
    // SDK and Scene
    auto sdk = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("testMshAndVegaIO");

    // Load a volumetric mesh (from .msh file)
    std::string ifile = iMSTK_DATA_ROOT "/liver/liver.msh";
    auto volMeshA = MeshIO::read(ifile);
    if (!volMeshA)
    {
        LOG(WARNING) << "Failed to read msh file : " << ifile;
        return;
    }

    // Extract surface mesh
    auto volumeMeshA = std::dynamic_pointer_cast<VolumetricMesh>(volMeshA); // change to any volumetric mesh above
    volumeMeshA->computeAttachedSurfaceMesh();
    auto surfaceMeshA = volumeMeshA->getAttachedSurfaceMesh();

    // Create object A
    auto objectA = std::make_shared<VisualObject>("meshObjectMSH");
    objectA->setVisualGeometry(surfaceMeshA);

    // Write a .veg file
    std::string ofile = iMSTK_DATA_ROOT "/liver/liver.veg";
    auto writeStatus = MeshIO::write(volMeshA, ofile);
    std::cout << "------------------------------Summary----------------------------------------------------\n";
    std::cout << "Following file conversion: " << ((writeStatus) ? "Success \n" : "Failure \n");
    std::cout << "\n Input mesh file : \n" << ifile << std::endl;
    std::cout << "\n Output mesh file: \n" << ofile << std::endl;

    // Read the above written veg file
    auto volMeshB = MeshIO::read(ofile);
    if (!volMeshB)
    {
        LOG(WARNING) << "Failed to extract topology/geometry from the veg file : " << ofile;
        return;
    }

    // Extract surface mesh
    auto volumeMeshB = std::dynamic_pointer_cast<VolumetricMesh>(volMeshB); // change to any volumetric mesh above
    volumeMeshB->computeAttachedSurfaceMesh();
    auto surfaceMeshB = volumeMeshB->getAttachedSurfaceMesh();

    // Create object B
    auto objectB = std::make_shared<VisualObject>("meshObjectVEGA");
    surfaceMeshB->translate(Vec3d(3, 0, 0), Geometry::TransformType::ApplyToData);
    objectB->setVisualGeometry(surfaceMeshB);

    // Add objects to the scene
    scene->addSceneObject(objectA);
    scene->addSceneObject(objectB);

    // Light
    auto light = std::make_shared<DirectionalLight>("light");
    light->setFocalPoint(Vec3d(5, -8, -5));
    light->setIntensity(1);
    scene->addLight(light);

    // Run
    sdk->setActiveScene(scene);
    sdk->startSimulation(true);
}

void testVTKTexture()
{
    // Parse command line arguments

    std::string inputFilename = iMSTK_DATA_ROOT "/ETI/resources/OperatingRoom/cloth.obj";
    std::string texturename = iMSTK_DATA_ROOT "/ETI/resources/TextureOR/cloth.jpg";

    std::string inputFilename1 = iMSTK_DATA_ROOT "/ETI/resources/OperatingRoom/bed1.obj";
    std::string texturename1 = iMSTK_DATA_ROOT "/ETI/resources/TextureOR/bed-1.jpg";

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
    auto objMesh = MeshIO::read(iMSTK_DATA_ROOT "/asianDragon/asianDragon.obj");
    auto surfaceMesh = std::dynamic_pointer_cast<SurfaceMesh>(objMesh);
    surfaceMesh->translate(-8, 0, 0, Geometry::TransformType::ApplyToData);

    // Read and setup texture/material
    auto diffuseTexture = std::make_shared<Texture>(iMSTK_DATA_ROOT "/textured_organs/texture_set_1/diffuse.png");
    auto cubemapTexture = std::make_shared<Texture>(iMSTK_DATA_ROOT "/textures/cubemaps/clouds1_.jpg", Texture::CUBEMAP);
    auto material = std::make_shared<RenderMaterial>();
    material->addTexture(diffuseTexture);
    material->addTexture(cubemapTexture);
    material->setRoughness(1.0);
    surfaceMesh->setRenderMaterial(material);

    // Create object and add to scene
    auto object = std::make_shared<VisualObject>("meshObject");
    object->setVisualGeometry(surfaceMesh);
    scene->addSceneObject(object);

    // Second object
    bool secondObject = true;
    bool secondObjectTexture = true;
    if (secondObject)
    {
        // Read surface mesh1
        auto objMesh1 = MeshIO::read(iMSTK_DATA_ROOT "/spheres/big.vtk");
        auto surfaceMesh1 = std::dynamic_pointer_cast<SurfaceMesh>(objMesh1);

        // Read and setup texture/material
        if (secondObjectTexture)
        {
            auto diffuseTexture1 = std::make_shared<Texture>(iMSTK_DATA_ROOT "/textured_organs/texture_set_2/diffuse.png");
            auto cubemapTexture1 = std::make_shared<Texture>(iMSTK_DATA_ROOT "/textures/cubemaps/clouds1_.jpg", Texture::CUBEMAP);
            auto material1 = std::make_shared<RenderMaterial>();
            material1->addTexture(diffuseTexture1);
            material1->addTexture(cubemapTexture1);
            material1->setMetalness(1.0);
            material1->setRoughness(0.0);
            material1->setDisplayMode(RenderMaterial::DisplayMode::WIREFRAME_SURFACE);
            surfaceMesh1->setRenderMaterial(material1);
        }

        // Create object and add to scene
        auto object1 = std::make_shared<VisualObject>("meshObject1");
        object1->setVisualGeometry(surfaceMesh1); // change to any mesh created above
        scene->addSceneObject(object1);
    }

    // Light (white)
    auto whiteLight = std::make_shared<DirectionalLight>("whiteLight");
    whiteLight->setFocalPoint(Vec3d(5, -8, -5));
    whiteLight->setIntensity(10);
    scene->addLight(whiteLight);

    // Run
    sdk->setActiveScene(scene);
    sdk->startSimulation(true);
}

void testMeshCCD()
{
    // SDK and Scene
    auto sdk = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("MeshCCDTest");

    auto mesh1 = MeshIO::read(iMSTK_DATA_ROOT "/asianDragon/asianDragon.obj");
    auto mesh2 = MeshIO::read(iMSTK_DATA_ROOT "/textured_organs/heart.obj");

    // Obj1
    auto obj1 = std::make_shared<CollidingObject>("obj1");
    obj1->setVisualGeometry(mesh1);
    obj1->setCollidingGeometry(mesh1);
    scene->addSceneObject(obj1);

    // set configure Obj1 rendering
    auto matObj1 = std::make_shared<RenderMaterial>();
    matObj1->setBackFaceCulling(false);
    matObj1->setDiffuseColor(Color::Green);
    matObj1->setDisplayMode(RenderMaterial::DisplayMode::WIREFRAME_SURFACE);
    mesh1->setRenderMaterial(matObj1);

    // Obj2
    auto obj2 = std::make_shared<CollidingObject>("obj2");
    obj2->setVisualGeometry(mesh2);
    obj2->setCollidingGeometry(mesh2);
    scene->addSceneObject(obj2);

    // set configure Obj2 rendering
    auto matObj2 = std::make_shared<RenderMaterial>();
    matObj2->setBackFaceCulling(false);
    matObj2->setDiffuseColor(Color::Green);
    matObj2->setDisplayMode(RenderMaterial::DisplayMode::WIREFRAME_SURFACE);
    mesh2->setRenderMaterial(matObj2);

    // Collisions
    auto colGraph = scene->getCollisionGraph();
    colGraph->addInteractionPair(obj1, obj2,
        CollisionDetection::Type::MeshToMesh,
        CollisionHandling::Type::None,
        CollisionHandling::Type::None);

    // Rotate the obj1 every frame
    auto rotateFunc = [&](Module* module)
                      {
                          mesh1->rotate(Vec3d(1., 0, 0), PI / 1000, Geometry::TransformType::ApplyToData);
                      };
    sdk->getSceneManager(scene)->setPostUpdateCallback(rotateFunc);

    // Light
    auto light = std::make_shared<DirectionalLight>("light");
    light->setFocalPoint(Vec3d(5, -8, -5));
    light->setIntensity(1);
    scene->addLight(light);

    // set the position of the camera
    scene->getCamera()->setPosition(0., 0., 10);

    // Run
    sdk->setActiveScene(scene);
    sdk->startSimulation(true);
}

void testPenaltyRigidCollision()
{
    // SDK and Scene
    auto sdk = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("testPenaltyRigidCollision");

    // Device server
    auto server = std::make_shared<VRPNDeviceServer>();
    server->addDevice(novintFalcon1Name, DeviceType::NOVINT_FALCON, 0);
    server->addDevice(novintFalcon2Name, DeviceType::NOVINT_FALCON, 1);
    sdk->addModule(server);

    // Falcon clients
    auto client0 = std::make_shared<VRPNDeviceClient>(novintFalcon1Name, "localhost");
    auto client1 = std::make_shared<VRPNDeviceClient>(novintFalcon2Name, "localhost");
    client0->setForceEnabled(true);
    client1->setForceEnabled(true);
    sdk->addModule(client0);
    sdk->addModule(client1);

    // Plane
    auto planeObj = apiutils::createCollidingAnalyticalSceneObject(Geometry::Type::Plane, scene, "plane", 10);

    // Sphere0
    auto sphere0Obj = apiutils::createCollidingAnalyticalSceneObject(
        Geometry::Type::Sphere, scene, "Sphere0", 0.5, Vec3d(1, 0.5, 0));

    auto trackCtrl0 = std::make_shared<DeviceTracker>(client0);
    trackCtrl0->setTranslationScaling(40);
    auto sphere0Controller = std::make_shared<SceneObjectController>(sphere0Obj, trackCtrl0);
    scene->addObjectController(sphere0Controller);

    // Sphere1
    auto sphere1Obj = apiutils::createCollidingAnalyticalSceneObject(
        Geometry::Type::Sphere, scene, "Sphere1", 0.5, Vec3d(-1., 0.5, 0.));

    auto trackCtrl1 = std::make_shared<DeviceTracker>(client1);
    trackCtrl1->setTranslationScaling(40);
    auto sphere1Controller = std::make_shared<SceneObjectController>(sphere1Obj, trackCtrl1);
    scene->addObjectController(sphere1Controller);

    // Collisions
    auto colGraph = scene->getCollisionGraph();
    colGraph->addInteractionPair(planeObj, sphere0Obj,
        CollisionDetection::Type::BidirectionalPlaneToSphere,
        CollisionHandling::Type::None,
        CollisionHandling::Type::Penalty);
    colGraph->addInteractionPair(planeObj, sphere1Obj,
        CollisionDetection::Type::BidirectionalPlaneToSphere,
        CollisionHandling::Type::None,
        CollisionHandling::Type::Penalty);
    colGraph->addInteractionPair(sphere0Obj, sphere1Obj,
        CollisionDetection::Type::SphereToSphere,
        CollisionHandling::Type::Penalty,
        CollisionHandling::Type::Penalty);

    // Light
    auto light = std::make_shared<DirectionalLight>("light");
    light->setFocalPoint(Vec3d(5, -8, -5));
    light->setIntensity(1);
    scene->addLight(light);

    // Run
    sdk->setActiveScene(scene);
    sdk->startSimulation(true);
}

void testTwoFalcons()
{
    // SDK and Scene
    auto sdk = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("FalconsTestScene");

    // Device server
    auto server = std::make_shared<VRPNDeviceServer>();
    server->addDevice("falcon0", DeviceType::NOVINT_FALCON, 0);
    server->addDevice("falcon1", DeviceType::NOVINT_FALCON, 1);
    sdk->addModule(server);

    // Falcon clients
    auto falcon0 = std::make_shared<VRPNDeviceClient>("falcon0", "localhost");
    sdk->addModule(falcon0);
    auto falcon1 = std::make_shared<VRPNDeviceClient>("falcon1", "localhost");
    sdk->addModule(falcon1);

    // Plane
    auto planeObj = apiutils::createVisualAnalyticalSceneObject(
        Geometry::Type::Plane, scene, "VisualPlane", 50, FORWARD_VECTOR * 15);

    // Sphere0
    auto sphere0Obj = apiutils::createCollidingAnalyticalSceneObject(
        Geometry::Type::Sphere, scene, "Sphere0", 1., Vec3d(-16, 4.5, 0));

    auto trackCtrl0 = std::make_shared<DeviceTracker>(falcon0);
    trackCtrl0->setTranslationScaling(100);
    auto controller0 = std::make_shared<SceneObjectController>(sphere0Obj, trackCtrl0);
    scene->addObjectController(controller0);

    // Sphere1
    auto sphere1Obj = apiutils::createCollidingAnalyticalSceneObject(
        Geometry::Type::Sphere, scene, "Sphere1", 1., Vec3d(16, 4.5, 0));

    auto trackCtrl1 = std::make_shared<DeviceTracker>(falcon1);
    trackCtrl1->setTranslationScaling(100);
    auto controller1 = std::make_shared<SceneObjectController>(sphere1Obj, trackCtrl1);
    scene->addObjectController(controller1);

    // Camera
    auto cam = scene->getCamera();
    cam->setPosition(Vec3d(0, 18, 40));

    // Print device tracking info (callback)
    Logger* logger;
    unsigned int displayCpt = 0;
    auto postInitFoo =
        [&logger](Module* module)
        {
            logger = new Logger(module->getName());
            logger->setFrequency(5);
        };
    auto postUpdateFoo =
        [&displayCpt, &logger](Module* module)
        {
            // Print position & velocity
            auto client = static_cast<VRPNDeviceClient*>(module);
            Vec3d p = client->getPosition();
            Vec3d v = client->getVelocity();
            std::string message =
                " pos = (" +  std::to_string(p[0])
                + ", " +  std::to_string(p[1])
                + ", "  +  std::to_string(p[2]) + ") "
                + " vel = (" +  std::to_string(v[0])
                + ", " + std::to_string(v[1])
                + ", "  + std::to_string(v[2]) + ")";

            // Show every 1000 updates in standard
            if (++displayCpt > 1000)
            {
                std::cout << "\r-- " << module->getName() << message << std::flush;
                displayCpt = 0;
            }

            // Asynchronous log
            if (logger->readyForLoggingWithFrequency())
            {
                logger->log(message, true);
                logger->updateLogTime();
            }
        };
    auto postCleanUpFoo =
        [&logger](Module* module)
        {
            logger->shutdown();
            delete logger;
        };

    falcon0->setPostInitCallback(postInitFoo);
    falcon0->setPostUpdateCallback(postUpdateFoo);
    falcon0->setPostCleanUpCallback(postCleanUpFoo);

    // Light
    auto light = std::make_shared<DirectionalLight>("light");
    light->setFocalPoint(Vec3d(5, -8, -5));
    light->setIntensity(1);
    scene->addLight(light);

    // Run
    sdk->setActiveScene(scene);
    sdk->startSimulation(true);
}

void testTwoOmnis()
{
#ifdef iMSTK_USE_OPENHAPTICS
    // SDK and Scene
    auto sdk = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("TwoOmnisTestScene");

    // Device clients
    auto client0 = std::make_shared<HDAPIDeviceClient>(phantomOmni1Name);
    auto client1 = std::make_shared<HDAPIDeviceClient>(phantomOmni2Name);

    // Device Server
    auto server = std::make_shared<HDAPIDeviceServer>();
    server->addDeviceClient(client0);
    server->addDeviceClient(client1);
    sdk->addModule(server);

    // Plane
    auto planeObj = apiutils::createVisualAnalyticalSceneObject(
        Geometry::Type::Plane, scene, "VisualPlane", 50., FORWARD_VECTOR * 15);

    // Sphere0
    auto sphere0Obj = apiutils::createCollidingAnalyticalSceneObject(
        Geometry::Type::Sphere, scene, "Sphere0", 1., Vec3d(2, 2.5, 0));

    auto trackCtrl0 = std::make_shared<DeviceTracker>(client0);
    trackCtrl0->setTranslationScaling(0.05);
    auto controller0 = std::make_shared<SceneObjectController>(sphere0Obj, trackCtrl0);
    scene->addObjectController(controller0);

    // Sphere1
    auto sphere1Obj = apiutils::createCollidingAnalyticalSceneObject(
        Geometry::Type::Sphere, scene, "Sphere1", 1., Vec3d(-2, 2.5, 0));

    auto trackCtrl1 = std::make_shared<DeviceTracker>(client1);
    trackCtrl1->setTranslationScaling(0.05);
    auto controller1 = std::make_shared<SceneObjectController>(sphere1Obj, trackCtrl1);
    scene->addObjectController(controller1);

    // Update Camera position
    auto cam = scene->getCamera();
    cam->setPosition(Vec3d(0, 0, 10));
    auto sphere0Geom = sphere0Obj->getVisualGeometry();
    cam->setFocalPoint(Vec3d(-2, 2.5, 0));

    // Light
    auto light = std::make_shared<DirectionalLight>("light");
    light->setFocalPoint(Vec3d(5, -8, -5));
    light->setIntensity(1);
    scene->addLight(light);

    // Run
    sdk->setActiveScene(scene);
    sdk->startSimulation(false);
#endif
}

void testObjectController()
{
#ifdef iMSTK_USE_OPENHAPTICS
    // SDK and Scene
    auto sdk = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("ObjectControllerTest");

    // Device Client
    auto client = std::make_shared<HDAPIDeviceClient>(phantomOmni1Name);

    // Device Server
    auto server = std::make_shared<HDAPIDeviceServer>();
    server->addDeviceClient(client);
    sdk->addModule(server);

    // Object
    auto geom = std::make_shared<Cube>();
    geom->setPosition(0, 1, 0);
    geom->setWidth(2);

    auto object = std::make_shared<CollidingObject>("VirtualObject");
    object->setVisualGeometry(geom);
    object->setCollidingGeometry(geom);
    scene->addSceneObject(object);

    auto trackCtrl = std::make_shared<DeviceTracker>(client);
    trackCtrl->setTranslationScaling(0.1);
    auto controller = std::make_shared<SceneObjectController>(object, trackCtrl);
    scene->addObjectController(controller);

    // Update Camera position
    auto cam = scene->getCamera();
    cam->setPosition(Vec3d(0, 0, 10));
    cam->setFocalPoint(geom->getPosition());

    // Light
    auto light = std::make_shared<DirectionalLight>("light");
    light->setFocalPoint(Vec3d(5, -8, -5));
    light->setIntensity(1);
    scene->addLight(light);

    // Run
    sdk->setActiveScene(scene);
    sdk->startSimulation(false);
#endif
}

void testCameraController()
{
    // SDK and Scene
    auto sdk = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("CameraControllerTest");

#ifdef iMSTK_USE_OPENHAPTICS

    auto client = std::make_shared<HDAPIDeviceClient>(phantomOmni1Name);

    // Device Server
    auto server = std::make_shared<HDAPIDeviceServer>();
    server->addDeviceClient(client);
    sdk->addModule(server);
#else
    LOG(WARNING) << "Phantom device option not enabled during build!";
#endif


    // Mesh
    auto mesh = MeshIO::read(iMSTK_DATA_ROOT "/asianDragon/asianDragon.obj");
    auto meshObject = std::make_shared<VisualObject>("meshObject");
    meshObject->setVisualGeometry(mesh);
    scene->addSceneObject(meshObject);

    // Update Camera position
    auto cam = scene->getCamera();
    cam->setPosition(Vec3d(0, 0, 10));

#ifdef iMSTK_USE_OPENHAPTICS

    auto camControllerInput = std::make_shared<CameraController>(*cam, client);

    // Set camera controller
    auto camController = cam->setController(camControllerInput);
    //camController->setTranslationScaling(100);
    //LOG(INFO) << camController->getTranslationOffset(); // should be the same than initial cam position
    camController->setInversionFlags(CameraController::InvertFlag::rotY |
                                     CameraController::InvertFlag::rotZ);
#endif

    // Light
    auto light = std::make_shared<DirectionalLight>("light");
    light->setFocalPoint(Vec3d(5, -8, -5));
    light->setIntensity(1);
    scene->addLight(light);

    // Run
    sdk->setActiveScene(scene);
    sdk->startSimulation(true);
}

void testReadMesh()
{
    // SDK and Scene
    auto sdk = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("testReadMesh");

    // Read surface mesh
    /*auto objMesh = MeshIO::read(iMSTK_DATA_ROOT"/asianDragon/asianDragon.obj");
    auto plyMesh = MeshIO::read(iMSTK_DATA_ROOT"/cube/cube.ply");
    auto stlMesh = MeshIO::read(iMSTK_DATA_ROOT"/cube/cube.stl");
    auto vtkMesh = MeshIO::read(iMSTK_DATA_ROOT"/cube/cube.vtk");
    auto vtpMesh = MeshIO::read(iMSTK_DATA_ROOT"/cube/cube.vtp");*/

    // Read volumetricMesh
    //auto vtkMesh2 = MeshIO::read(iMSTK_DATA_ROOT"/nidus/nidus.vtk");
    auto vegaMesh = MeshIO::read(iMSTK_DATA_ROOT "/asianDragon/asianDragon.veg");

    // Extract surface mesh
    auto volumeMesh = std::dynamic_pointer_cast<VolumetricMesh>(vegaMesh); // change to any volumetric mesh above
    volumeMesh->computeAttachedSurfaceMesh();
    auto surfaceMesh = volumeMesh->getAttachedSurfaceMesh();

    // Create object and add to scene
    auto object = std::make_shared<VisualObject>("meshObject");
    object->setVisualGeometry(surfaceMesh); // change to any mesh created above
    scene->addSceneObject(object);

    // Light
    auto light = std::make_shared<DirectionalLight>("light");
    light->setFocalPoint(Vec3d(5, -8, -5));
    light->setIntensity(1);
    scene->addLight(light);

    // Run
    sdk->setActiveScene(scene);
    sdk->startSimulation(true);
}

void testViewer()
{
    // SDK and Scene
    auto sdk = std::make_shared<SimulationManager>();
    auto sceneTest = sdk->createNewScene("ViewerTest");

    // Plane
    auto planeObj = apiutils::createVisualAnalyticalSceneObject(Geometry::Type::Plane, sceneTest, "VisualPlane", 10);

    // Cube
    auto cubeObj = apiutils::createVisualAnalyticalSceneObject(
        Geometry::Type::Cube, sceneTest, "VisualCube", 0.5, Vec3d(1.0, -1.0, 0.5));
    auto cubeGeom = cubeObj->getVisualGeometry();
    // rotates could be replaced by cubeGeom->setOrientationAxis(1,1,1) (normalized inside)
    cubeGeom->rotate(UP_VECTOR, PI_4, Geometry::TransformType::ApplyToData);
    cubeGeom->rotate(RIGHT_VECTOR, PI_4, Geometry::TransformType::ApplyToData);

    // Sphere
    auto sphereObj = apiutils::createVisualAnalyticalSceneObject(
        Geometry::Type::Sphere, sceneTest, "VisualSphere", 0.3, Vec3d(0, 2., 0));

    // Light (white)
    auto whiteLight = std::make_shared<PointLight>("whiteLight");
    whiteLight->setPosition(Vec3d(5, 8, 5));
    whiteLight->setIntensity(100);
    sceneTest->addLight(whiteLight);

    // Light (red)
    auto colorLight = std::make_shared<SpotLight>("colorLight");
    colorLight->setPosition(Vec3d(4, -3, 1));
    colorLight->setFocalPoint(Vec3d(0, 0, 0));
    colorLight->setColor(Color::Red);
    colorLight->setIntensity(100);
    colorLight->setSpotAngle(1);
    sceneTest->addLight(colorLight);

    // Update Camera
    auto cam1 = sceneTest->getCamera();
    cam1->setPosition(Vec3d(-5.5, 2.5, 32));
    cam1->setFocalPoint(Vec3d(1, 1, 0));

    // Run
    sdk->setActiveScene(sceneTest);
    sdk->startSimulation(true);
}

void testCapsule()
{
    // SDK and Scene
    auto sdk = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("CapsuleTest");

    // Plane
    auto planeObj = apiutils::createVisualAnalyticalSceneObject(Geometry::Type::Plane, scene, "VisualPlane", 10);

    // Capsule
    auto capsuleObj = apiutils::createVisualAnalyticalSceneObject(
        Geometry::Type::Capsule, scene, "VisualCapsule", 2.0, Vec3d(0., 1., 0.));
    auto capsuleGeom = capsuleObj->getVisualGeometry();
    // rotates could be replaced by cubeGeom->setOrientationAxis(1,1,0) (normalized inside)
    capsuleGeom->rotate(RIGHT_VECTOR, PI_4, Geometry::TransformType::ApplyToData);

#ifdef iMSTK_USE_OPENHAPTICS
    // Device Client
    auto client = std::make_shared<HDAPIDeviceClient>(phantomOmni1Name);

    // Device Server
    auto server = std::make_shared<HDAPIDeviceServer>();
    server->addDeviceClient(client);
    sdk->addModule(server);

    auto trackCtrl = std::make_shared<DeviceTracker>(client);
    trackCtrl->setTranslationScaling(0.1);
    auto controller = std::make_shared<SceneObjectController>(capsuleObj, trackCtrl);
    scene->addObjectController(controller);
#endif

    // Add objects and light to scene
    scene->addSceneObject(planeObj);
    scene->addSceneObject(capsuleObj);

    // Update Camera
    auto cam1 = scene->getCamera();
    cam1->setPosition(Vec3d(5., 5., 5.));
    cam1->setFocalPoint(Vec3d(1, 1, 0));

    // Light
    auto light = std::make_shared<DirectionalLight>("light");
    light->setFocalPoint(Vec3d(5, -8, -5));
    light->setIntensity(1);
    scene->addLight(light);

    // Run
    sdk->setActiveScene(scene);
    sdk->startSimulation(true);
}

void testAnalyticalGeometry()
{
    auto sdk = std::make_shared<SimulationManager>();

    // Plane
    LOG(INFO) << "-- Plane : Init";
    auto pos = Vec3d(5, 2, 5);
    auto norm = Vec3d(0, 1, 1);
    auto width = 10;
    LOG(INFO) << "p = " << pos;
    LOG(INFO) << "n = " << norm;
    LOG(INFO) << "w = " << width;

    LOG(INFO) << "-- Plane : Create";
    auto plane = std::make_shared<Plane>();
    LOG(INFO) << "p = " << plane->getPosition();
    LOG(INFO) << "n = " << plane->getNormal();
    LOG(INFO) << "w = " << plane->getWidth();

    LOG(INFO) << "-- Plane : Set Position";
    plane->setPosition(Vec3d(1, 1, 1));
    LOG(INFO) << "p = " << plane->getPosition();

    LOG(INFO) << "-- Plane : Translate";
    plane->translate(Vec3d(2, 1, -3), Geometry::TransformType::ApplyToData);
    LOG(INFO) << "p = " << plane->getPosition();

    LOG(INFO) << "-- Plane : Set Normal";
    plane->setNormal(FORWARD_VECTOR);
    LOG(INFO) << "n = " << plane->getNormal();

    LOG(INFO) << "-- Plane : Rotate";
    plane->rotate(UP_VECTOR, PI_2, Geometry::TransformType::ApplyToData);
    LOG(INFO) << "n = " << plane->getNormal();
}

void testScenesManagement()
{
    // THIS TESTS NEEDS TO DISABLE STANDALONE VIEWER RENDERING

    auto sdk = std::make_shared<SimulationManager>();

    // Scenes
    LOG(INFO) << "-- Test add scenes";
    auto scene1 = std::make_shared<Scene>("scene1");
    sdk->addScene(scene1);

    sdk->createNewScene("scene2");
    auto scene2 = sdk->getScene("scene2");

    auto scene3 = sdk->createNewScene();
    sdk->removeScene("Scene_3");

    // switch
    LOG(INFO) << "-- Test scene switch";
    int delay = 5;
    sdk->setActiveScene(scene1);
    sdk->startSimulation();
    std::this_thread::sleep_for(std::chrono::seconds(delay));
    sdk->setActiveScene(scene2, false);
    std::this_thread::sleep_for(std::chrono::seconds(delay));
    sdk->setActiveScene(scene1, true);
    std::this_thread::sleep_for(std::chrono::seconds(delay));
    sdk->endSimulation();

    // pause/run
    LOG(INFO) << "-- Test simulation pause/run";
    sdk->setActiveScene(scene2);
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
    while (sdk->getStatus() != SimulationStatus::INACTIVE) {}
}

void testIsometricMap()
{
    // SDK and Scene
    auto sdk = std::make_shared<SimulationManager>();
    auto geometryMapTest = sdk->createNewScene("geometryMapTest");

    // Cube
    auto cubeGeom = std::make_shared<Cube>();
    cubeGeom->setWidth(0.5);
    auto cubeObj = std::make_shared<VisualObject>("VisualCube");
    cubeObj->setVisualGeometry(cubeGeom);

    // Sphere
    auto sphereGeom = std::make_shared<Sphere>();
    sphereGeom->setRadius(0.3);
    auto sphereObj = std::make_shared<VisualObject>("VisualSphere");
    sphereObj->setVisualGeometry(sphereGeom);

    // Add objects in Scene
    geometryMapTest->addSceneObject(cubeObj);
    geometryMapTest->addSceneObject(sphereObj);

    // Isometric Map
    auto transform = RigidTransform3d::Identity();
    transform.translate(Vec3d(0.0, 1.0, 0.0));
    transform.rotate(Rotd(PI_4, Vec3d(0, 1.0, 0)));

    auto rigidMap = std::make_shared<IsometricMap>();
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

    // Light
    auto light = std::make_shared<DirectionalLight>("light");
    light->setFocalPoint(Vec3d(5, -8, -5));
    light->setIntensity(1);
    geometryMapTest->addLight(light);

    // Start simulation
    sdk->setActiveScene(geometryMapTest);
    sdk->startSimulation(imstk::Renderer::Mode::DEBUG);
}

void testTetraTriangleMap()
{
    auto sdk = std::make_shared<SimulationManager>();

    // Tetrahedral mesh
    auto tetMesh = std::make_shared<TetrahedralMesh>();
    StdVectorOfVec3d vertList;
    vertList.push_back(Vec3d(0, 0, 0));
    vertList.push_back(Vec3d(1.0, 0, 0));
    vertList.push_back(Vec3d(0, 1.0, 0));
    vertList.push_back(Vec3d(0, 0, 1.0));
    tetMesh->setInitialVertexPositions(vertList);
    tetMesh->setVertexPositions(vertList);

    std::vector<TetrahedralMesh::TetraArray> tetConnectivity;
    TetrahedralMesh::TetraArray tet1 = { 0, 1, 2, 3 };
    tetConnectivity.push_back(tet1);
    tetMesh->setTetrahedraVertices(tetConnectivity);

    // Triangular mesh
    auto triMesh = std::make_shared<SurfaceMesh>();
    StdVectorOfVec3d SurfVertList;
    SurfVertList.push_back(Vec3d(0, 0, 1));// coincides with one vertex
    SurfVertList.push_back(Vec3d(0.25, 0.25, 0.25));// centroid
    SurfVertList.push_back(Vec3d(1.05, 0, 0));
    triMesh->setInitialVertexPositions(SurfVertList);
    triMesh->setVertexPositions(SurfVertList);

    // Construct a map
    auto tetTriMap = std::make_shared<TetraTriangleMap>();
    tetTriMap->setMaster(tetMesh);
    tetTriMap->setSlave(triMesh);
    tetTriMap->compute();

    tetTriMap->print();

    getchar();
}

void testExtractSurfaceMesh()
{
    auto sdk = std::make_shared<SimulationManager>();

    // a. Construct a sample tetrahedral mesh

    // a.1 add vertex positions
    auto tetMesh = std::make_shared<TetrahedralMesh>();
    StdVectorOfVec3d vertList;
    vertList.push_back(Vec3d(0, 0, 0));
    vertList.push_back(Vec3d(1.0, 0, 0));
    vertList.push_back(Vec3d(0, 1.0, 0));
    vertList.push_back(Vec3d(0, 0, 1.0));
    vertList.push_back(Vec3d(1.0, 1.0, 1.0));
    tetMesh->setInitialVertexPositions(vertList);
    tetMesh->setVertexPositions(vertList);

    // a.2 add connectivity
    std::vector<TetrahedralMesh::TetraArray> tetConnectivity;
    TetrahedralMesh::TetraArray tet1 = { 0, 1, 2, 3 };
    TetrahedralMesh::TetraArray tet2 = { 1, 2, 3, 4 };
    tetConnectivity.push_back(tet1);
    tetConnectivity.push_back(tet2);
    tetMesh->setTetrahedraVertices(tetConnectivity);

    // b. Print tetrahedral mesh
    tetMesh->print();

    // c. Extract the surface mesh
    auto surfMesh = std::make_shared<SurfaceMesh>();
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
    auto sdk = std::make_shared<SimulationManager>();

    // a. Construct a sample tetrahedral mesh

    // a.1 add vertex positions
    auto tetMesh = std::make_shared<TetrahedralMesh>();
    StdVectorOfVec3d vertList;
    vertList.push_back(Vec3d(0, 0, 0));
    vertList.push_back(Vec3d(1.0, 0, 0));
    vertList.push_back(Vec3d(0, 1.0, 0));
    vertList.push_back(Vec3d(0, 0, 1.0));
    vertList.push_back(Vec3d(1.0, 1.0, 1.0));
    tetMesh->setInitialVertexPositions(vertList);
    tetMesh->setVertexPositions(vertList);

    tetMesh->print();

    // b. Construct a surface mesh
    auto triMesh = std::make_shared<SurfaceMesh>();

    // b.1 Add vertex positions
    StdVectorOfVec3d SurfVertList;
    SurfVertList.push_back(Vec3d(0, 0, 0));
    SurfVertList.push_back(Vec3d(1.0, 0, 0));
    SurfVertList.push_back(Vec3d(0, 1.0, 0));
    SurfVertList.push_back(Vec3d(0, 0, 1.0));
    SurfVertList.push_back(Vec3d(1.0, 1.0, 1.0));
    triMesh->setInitialVertexPositions(SurfVertList);
    triMesh->setVertexPositions(SurfVertList);

    // b.2 Add vertex connectivity
    std::vector<SurfaceMesh::TriangleArray> triConnectivity;
    triConnectivity.push_back({ { 0, 1, 2 } });
    triConnectivity.push_back({ { 0, 1, 3 } });
    triConnectivity.push_back({ { 0, 2, 3 } });
    triConnectivity.push_back({ { 1, 2, 4 } });
    triConnectivity.push_back({ { 1, 3, 4 } });
    triConnectivity.push_back({ { 2, 3, 4 } });
    triMesh->setTrianglesVertices(triConnectivity);

    triMesh->print();

    // c. Construct the one to one nodal map based on the above meshes
    auto oneToOneNodalMap = std::make_shared<OneToOneMap>();
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
    auto sdk = std::make_shared<SimulationManager>();

    // a. Construct a sample triangular mesh

    // b. Add nodal data
    auto surfMesh = std::make_shared<SurfaceMesh>();
    StdVectorOfVec3d vertList;
    vertList.push_back(Vec3d(0, 0, 0));
    vertList.push_back(Vec3d(0.5, 0.5, 0));
    vertList.push_back(Vec3d(1, 1, 0));
    vertList.push_back(Vec3d(1, 0, 0));
    vertList.push_back(Vec3d(0, 1, 0));
    vertList.push_back(Vec3d(0.5, 1, 0));
    vertList.push_back(Vec3d(0, 0.5, 0));
    vertList.push_back(Vec3d(1, 0.5, 0));
    vertList.push_back(Vec3d(0.5, 0, 0));
    surfMesh->setInitialVertexPositions(vertList);
    surfMesh->setVertexPositions(vertList);

    // c. Add connectivity data
    std::vector<SurfaceMesh::TriangleArray> triangles;
    SurfaceMesh::TriangleArray tri[8];
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

    StopWatch wwt;
    CpuTimer ct;

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
    scene->getCamera()->setPosition(0, 2.0, 15.0);

    // b. Load a tetrahedral mesh
    //auto tetMesh = MeshIO::read(iMSTK_DATA_ROOT "/oneTet/oneTet.veg");
    auto tetMesh = MeshIO::read(iMSTK_DATA_ROOT "/asianDragon/asianDragon.veg");
    //auto tetMesh = MeshIO::read(iMSTK_DATA_ROOT"/liver/liver.veg");
    //auto tetMesh = MeshIO::read(iMSTK_DATA_ROOT"/oneTet/oneTet.veg");
    if (!tetMesh)
    {
        LOG(WARNING) << "Could not read mesh from file.";
        return;
    }

    // c. Extract the surface mesh
    auto surfMesh = std::make_shared<SurfaceMesh>();
    auto volTetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(tetMesh);
    if (!volTetMesh)
    {
        LOG(WARNING) << "Dynamic pointer cast from PointSet to TetrahedralMesh failed!";
        return;
    }
    volTetMesh->extractSurfaceMesh(surfMesh, true);

    StopWatch wct;
    CpuTimer cput;

    wct.start();
    cput.start();

    // d. Construct a map

    // d.1 Construct one to one nodal map based on the above meshes
    auto oneToOneNodalMap = std::make_shared<OneToOneMap>();
    oneToOneNodalMap->setMaster(tetMesh);
    oneToOneNodalMap->setSlave(surfMesh);

    // d.2 Compute the map
    oneToOneNodalMap->compute();

    LOG(INFO) << "wall clock time: " << wct.getTimeElapsed() << " ms.";
    LOG(INFO) << "CPU time: " << cput.getTimeElapsed() << " ms.";

    // e. Scene object 1: Dragon

    // Configure dynamic model
    auto dynaModel = std::make_shared<FEMDeformableBodyModel>();
    //dynaModel->configure(iMSTK_DATA_ROOT "/oneTet/oneTet.config");
    dynaModel->configure(iMSTK_DATA_ROOT "/asianDragon/asianDragon.config");
    dynaModel->setTimeStepSizeType(TimeSteppingType::realTime);
    //dynaModel->configure(iMSTK_DATA_ROOT"/liver/liver.config");
    dynaModel->setModelGeometry(volTetMesh);
    auto timeIntegrator = std::make_shared<BackwardEuler>(0.001);// Create and add Backward Euler time integrator
    dynaModel->setTimeIntegrator(timeIntegrator);

    auto material = std::make_shared<RenderMaterial>();
    material->setDisplayMode(RenderMaterial::DisplayMode::WIREFRAME_SURFACE);
    surfMesh->setRenderMaterial(material);

    // Scene Object
    auto deformableObj = std::make_shared<DeformableObject>("Dragon");
    deformableObj->setVisualGeometry(surfMesh);
    //deformableObj->setCollidingGeometry(surfMesh);
    deformableObj->setPhysicsGeometry(volTetMesh);
    deformableObj->setPhysicsToVisualMap(oneToOneNodalMap); //assign the computed map
    deformableObj->setDynamicalModel(dynaModel);
    scene->addSceneObject(deformableObj);

    // f. Scene object 2: Plane
    auto planeGeom = std::make_shared<Plane>();
    planeGeom->setWidth(40);
    planeGeom->setPosition(0, -6, 0);
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
        s.setProjectorToDirichlet(i, Vec3d(0.001, 0, 0));
        projList.push_back(s);
    }

    nlSystem->setUnknownVector(dynaModel->getUnknownVec());
    nlSystem->setUpdateFunction(dynaModel->getUpdateFunction());
    nlSystem->setUpdatePreviousStatesFunction(dynaModel->getUpdatePrevStateFunction());

    // create a linear solver
    auto linSolver = std::make_shared<ConjugateGradient>();
    //auto linSolver = std::make_shared<GaussSeidel>();
    //auto linSolver = std::make_shared<Jacobi>();
    //auto linSolver = std::make_shared<SOR>(0.4);

    // create a non-linear solver and add to the scene
    auto nlSolver = std::make_shared<NewtonSolver>();
    linSolver->setLinearProjectors(&projList);
    nlSolver->setLinearSolver(linSolver);
    nlSolver->setSystem(nlSystem);
    scene->addNonlinearSolver(nlSolver);

    // print UPS
    auto ups = std::make_shared<UPSCounter>();
    apiutils::printUPS(sdk->getSceneManager(scene), ups);

    // Light
    auto light = std::make_shared<DirectionalLight>("light");
    light->setFocalPoint(Vec3d(5, -8, -5));
    light->setIntensity(1);
    scene->addLight(light);

    // Run the simulation
    sdk->setActiveScene(scene);
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
    auto scene = sdk->createNewScene("PBDVolumeTest");
    scene->getCamera()->setPosition(0, 2.0, 15.0);

    // b. Load a tetrahedral mesh
    auto tetMesh = MeshIO::read(iMSTK_DATA_ROOT "/asianDragon/asianDragon.veg");
    if (!tetMesh)
    {
        LOG(WARNING) << "Could not read mesh from file.";
        return;
    }

    // c. Extract the surface mesh
    auto surfMesh = std::make_shared<SurfaceMesh>();
    auto volTetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(tetMesh);
    if (!volTetMesh)
    {
        LOG(WARNING) << "Dynamic pointer cast from PointSet to TetrahedralMesh failed!";
        return;
    }
    volTetMesh->extractSurfaceMesh(surfMesh, true);

    auto material = std::make_shared<RenderMaterial>();
    material->setDisplayMode(RenderMaterial::DisplayMode::WIREFRAME_SURFACE);
    surfMesh->setRenderMaterial(material);

    // d. Construct a map

    // d.1 Construct one to one nodal map based on the above meshes
    auto oneToOneNodalMap = std::make_shared<OneToOneMap>();
    oneToOneNodalMap->setMaster(tetMesh);
    oneToOneNodalMap->setSlave(surfMesh);

    // d.2 Compute the map
    oneToOneNodalMap->compute();

    auto deformableObj = std::make_shared<PbdObject>("Beam");
    auto pbdModel = std::make_shared<PbdModel>();
    pbdModel->setModelGeometry(volTetMesh);
    pbdModel->configure(/*Number of Constraints*/ 1,
        /*Constraint configuration*/ "FEM StVk 100.0 0.3",
        /*Mass*/ 1.0,
        /*Gravity*/ "0 -9.8 0",
        /*TimeStep*/ 0.01,
        /*FixedPoint*/ "51 127 178",
        /*NumberOfIterationInConstraintSolver*/ 5
        );

    deformableObj->setDynamicalModel(pbdModel);
    deformableObj->setVisualGeometry(surfMesh);
    deformableObj->setPhysicsGeometry(volTetMesh);
    deformableObj->setPhysicsToVisualMap(oneToOneNodalMap); //assign the computed map

    deformableObj->setPbdModel(pbdModel);
    auto pbdSolver = std::make_shared<PbdSolver>();
    pbdSolver->setPbdObject(deformableObj);
    scene->addNonlinearSolver(pbdSolver);

    scene->addSceneObject(deformableObj);

    auto planeGeom = std::make_shared<Plane>();
    planeGeom->setWidth(40);
    planeGeom->setTranslation(0, -6, 0);
    auto planeObj = std::make_shared<CollidingObject>("Plane");
    planeObj->setVisualGeometry(planeGeom);
    planeObj->setCollidingGeometry(planeGeom);
    scene->addSceneObject(planeObj);

    // Light
    auto light = std::make_shared<DirectionalLight>("light");
    light->setFocalPoint(Vec3d(5, -8, -5));
    light->setIntensity(1);
    scene->addLight(light);

    // print UPS
    auto ups = std::make_shared<UPSCounter>();
    apiutils::printUPS(sdk->getSceneManager(scene), ups);

    sdk->setActiveScene(scene);
    sdk->getViewer()->setBackgroundColors(Vec3d(0.3285, 0.3285, 0.6525), Vec3d(0.13836, 0.13836, 0.2748), true);
    sdk->startSimulation();
}

void testPbdCloth()
{
    auto sdk = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("PBDClothTest");

    // a. Construct a sample triangular mesh

    // b. Add nodal data
    auto surfMesh = std::make_shared<SurfaceMesh>();
    StdVectorOfVec3d vertList;
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
    surfMesh->setInitialVertexPositions(vertList);
    surfMesh->setVertexPositions(vertList);

    // c. Add connectivity data
    std::vector<SurfaceMesh::TriangleArray> triangles;
    for (std::size_t i = 0; i < nRows - 1; ++i)
    {
        for (std::size_t j = 0; j < nCols - 1; j++)
        {
            SurfaceMesh::TriangleArray tri[2];
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
    pbdModel->setModelGeometry(surfMesh);
    pbdModel->configure(/*Number of constraints*/ 2,
        /*Constraint configuration*/ "Distance 0.1",
        /*Constraint configuration*/ "Dihedral 0.001",
        /*Mass*/ 1.0,
        /*Gravity*/ "0 -9.8 0",
        /*TimeStep*/ 0.03,
        /*FixedPoint*/ "1 2 3 4 5 6 7 8 9 10 11",
        /*NumberOfIterationInConstraintSolver*/ 5);
    deformableObj->setDynamicalModel(pbdModel);
    deformableObj->setVisualGeometry(surfMesh);
    deformableObj->setPhysicsGeometry(surfMesh);

    auto material = std::make_shared<RenderMaterial>();
    material->setBackFaceCulling(false);
    material->setDiffuseColor(Color::LightGray);
    material->setDisplayMode(RenderMaterial::DisplayMode::WIREFRAME_SURFACE);
    surfMesh->setRenderMaterial(material);

    // Solver
    auto pbdSolver = std::make_shared<PbdSolver>();
    pbdSolver->setPbdObject(deformableObj);
    scene->addNonlinearSolver(pbdSolver);

    // Light (white)
    auto whiteLight = std::make_shared<DirectionalLight>("whiteLight");
    whiteLight->setFocalPoint(Vec3d(5, -8, -5));
    whiteLight->setIntensity(7);

    // Light (red)
    auto colorLight = std::make_shared<SpotLight>("colorLight");
    colorLight->setPosition(Vec3d(-5, -3, 5));
    colorLight->setFocalPoint(Vec3d(0, -5, 5));
    colorLight->setIntensity(100);
    colorLight->setColor(Color::Red);
    colorLight->setSpotAngle(30);

    // Add in scene
    scene->addLight(whiteLight);
    scene->addLight(colorLight);
    scene->addSceneObject(deformableObj);

    // print UPS
    auto ups = std::make_shared<UPSCounter>();
    apiutils::printUPS(sdk->getSceneManager(scene), ups);

    scene->getCamera()->setFocalPoint(0, -5, 5);
    scene->getCamera()->setPosition(-15., -5.0, 15.0);

    // Start
    sdk->setActiveScene(scene);
    sdk->startSimulation(true);
}

void testGraph()
{
    Graph g1(5);
    g1.addEdge(0, 1);
    g1.addEdge(0, 2);
    g1.addEdge(1, 2);
    g1.addEdge(1, 3);
    g1.addEdge(2, 3);
    g1.addEdge(3, 4);

    g1.print();
    auto colorsG1 = g1.doGreedyColoring(0);

    Graph g2(5);
    g2.addEdge(0, 1);
    g2.addEdge(0, 2);
    g2.addEdge(1, 2);
    g2.addEdge(1, 4);
    g2.addEdge(2, 4);
    g2.addEdge(4, 3);

    g2.print();
    auto colorsG2 = g2.doGreedyColoring(1);

    getchar();
}

void testPbdCollision()
{
    auto sdk = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("PbdCollisionTest");

    scene->getCamera()->setPosition(0, 10.0, 10.0);

    // dragon
    auto tetMesh = MeshIO::read(iMSTK_DATA_ROOT "/asianDragon/asianDragon.veg");
    if (!tetMesh)
    {
        LOG(WARNING) << "Could not read mesh from file.";
        return;
    }

    auto surfMesh = std::make_shared<SurfaceMesh>();
    auto surfMeshVisual = std::make_shared<SurfaceMesh>();
    auto volTetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(tetMesh);
    if (!volTetMesh)
    {
        LOG(WARNING) << "Dynamic pointer cast from PointSet to TetrahedralMesh failed!";
        return;
    }
    volTetMesh->extractSurfaceMesh(surfMesh, true);

    auto material = std::make_shared<RenderMaterial>();
    material->setDisplayMode(RenderMaterial::DisplayMode::WIREFRAME_SURFACE);
    surfMesh->setRenderMaterial(material);

    auto deformMapP2V = std::make_shared<OneToOneMap>();
    deformMapP2V->setMaster(tetMesh);
    deformMapP2V->setSlave(surfMesh);
    deformMapP2V->compute();

    auto deformMapC2V = std::make_shared<OneToOneMap>();
    deformMapC2V->setMaster(surfMesh);
    deformMapC2V->setSlave(surfMesh);
    deformMapC2V->compute();

    auto deformMapP2C = std::make_shared<OneToOneMap>();
    deformMapP2C->setMaster(tetMesh);
    deformMapP2C->setSlave(surfMesh);
    deformMapP2C->compute();

    auto deformableObj = std::make_shared<PbdObject>("Dragon");
    deformableObj->setVisualGeometry(surfMesh);
    deformableObj->setCollidingGeometry(surfMesh);
    deformableObj->setPhysicsGeometry(volTetMesh);
    deformableObj->setPhysicsToCollidingMap(deformMapP2C);
    deformableObj->setPhysicsToVisualMap(deformMapP2V);
    deformableObj->setCollidingToVisualMap(deformMapC2V);

    auto pbdModel = std::make_shared<PbdModel>();
    pbdModel->setModelGeometry(volTetMesh);
    pbdModel->configure(/*Number of Constraints*/ 1,
        /*Constraint configuration*/ "FEM NeoHookean 1.0 0.3",
        /*Mass*/ 1.0,
        /*Gravity*/ "0 -9.8 0",
        /*TimeStep*/ 0.001,
        /*FixedPoint*/ "",
        /*NumberOfIterationInConstraintSolver*/ 2,
        /*Proximity*/ 0.1,
        /*Contact stiffness*/ 0.01);
    deformableObj->setDynamicalModel(pbdModel);

    auto pbdSolver = std::make_shared<PbdSolver>();
    pbdSolver->setPbdObject(deformableObj);
    scene->addNonlinearSolver(pbdSolver);

    scene->addSceneObject(deformableObj);

    bool clothTest = 0;
    bool volumetric = !clothTest;
    if (clothTest)
    {
        auto clothMesh = std::make_shared<SurfaceMesh>();
        StdVectorOfVec3d vertList;
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
        clothMesh->setInitialVertexPositions(vertList);
        clothMesh->setVertexPositions(vertList);

        // c. Add connectivity data
        std::vector<SurfaceMesh::TriangleArray> triangles;
        for (std::size_t i = 0; i < nRows - 1; ++i)
        {
            for (std::size_t j = 0; j < nCols - 1; j++)
            {
                SurfaceMesh::TriangleArray tri[2];
                tri[0] = { { i*nCols + j, i*nCols + j + 1, (i + 1)*nCols + j } };
                tri[1] = { { (i + 1)*nCols + j + 1, (i + 1)*nCols + j, i*nCols + j + 1 } };
                triangles.push_back(tri[0]);
                triangles.push_back(tri[1]);
            }
        }
        clothMesh->setTrianglesVertices(triangles);

        auto oneToOneFloor = std::make_shared<OneToOneMap>();
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
        //floor->initialize(/*Number of constraints*/ 2,
        //                  /*Constraint configuration*/ "Distance 0.1",
        //                  /*Constraint configuration*/ "Dihedral 0.001",
        //                  /*Mass*/ 0.1,
        //                  /*Gravity*/ "0 9.8 0",
        //                  /*TimeStep*/ 0.002,
        //                  /*FixedPoint*/ fixed_corner.c_str(),
        //                  /*NumberOfIterationInConstraintSolver*/ 5,
        //                  /*Proximity*/ 0.1,
        //                  /*Contact stiffness*/ 0.95);
        scene->addSceneObject(floor);

        std::cout << "nbr of vertices in cloth mesh" << clothMesh->getNumVertices() << std::endl;

        // Collisions
        auto clothTestcolGraph = scene->getCollisionGraph();
        auto pair1 = std::make_shared<PbdInteractionPair>(PbdInteractionPair(deformableObj, floor));
        pair1->setNumberOfInterations(5);

        clothTestcolGraph->addInteractionPair(pair1);

        scene->getCamera()->setPosition(0, 0, 50);
    }
    else if (0)
    {
        auto tetMesh1 = MeshIO::read(iMSTK_DATA_ROOT "/asianDragon/asianDragon.veg");
        if (!tetMesh1)
        {
            LOG(WARNING) << "Could not read mesh from file.";
            return;
        }

        auto surfMesh1 = std::make_shared<SurfaceMesh>();
        auto surfMeshVisual1 = std::make_shared<SurfaceMesh>();
        auto volTetMesh1 = std::dynamic_pointer_cast<TetrahedralMesh>(tetMesh1);
        if (!volTetMesh1)
        {
            LOG(WARNING) << "Dynamic pointer cast from PointSet to TetrahedralMesh failed!";
            return;
        }

        auto vs = volTetMesh1->getInitialVertexPositions();
        Vec3d tmpPos;
        for (int i = 0; i < volTetMesh1->getNumVertices(); ++i)
        {
            tmpPos = volTetMesh1->getVertexPosition(i);
            tmpPos[1] -= 6;
            volTetMesh1->setVertexPosition(i, tmpPos);
        }
        volTetMesh1->setInitialVertexPositions(volTetMesh1->getVertexPositions());

        volTetMesh1->extractSurfaceMesh(surfMesh1);
        volTetMesh1->extractSurfaceMesh(surfMeshVisual1);


        auto deformMapP2V1 = std::make_shared<OneToOneMap>();
        deformMapP2V1->setMaster(volTetMesh1);
        deformMapP2V1->setSlave(surfMeshVisual1);
        deformMapP2V1->compute();

        auto deformMapC2V1 = std::make_shared<OneToOneMap>();
        deformMapC2V1->setMaster(surfMesh1);
        deformMapC2V1->setSlave(surfMeshVisual1);
        deformMapC2V1->compute();

        auto deformMapP2C1 = std::make_shared<OneToOneMap>();
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
        //deformableObj1->initialize(/*Number of Constraints*/ 1,
        //                           /*Constraint configuration*/ "FEM NeoHookean 10.0 0.5",
        //                           /*Mass*/ 0.0,
        //                           /*Gravity*/ "0 -9.8 0",
        //                           /*TimeStep*/ 0.002,
        //                           /*FixedPoint*/ "",
        //                           /*NumberOfIterationInConstraintSolver*/ 2,
        //                           /*Proximity*/ 0.1,
        //                           /*Contact stiffness*/ 0.01);

        scene->addSceneObject(deformableObj1);

        // Collisions
        auto colGraph = scene->getCollisionGraph();
        auto pair = std::make_shared<PbdInteractionPair>(PbdInteractionPair(deformableObj, deformableObj1));
        pair->setNumberOfInterations(2);

        colGraph->addInteractionPair(pair);
    }
    else
    {
        // Build floor geometry
        StdVectorOfVec3d vertList;
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
        std::vector<SurfaceMesh::TriangleArray> triangles;
        for (std::size_t i = 0; i < nRows - 1; ++i)
        {
            for (std::size_t j = 0; j < nCols - 1; j++)
            {
                SurfaceMesh::TriangleArray tri[2];
                tri[0] = { { i*nCols + j, i*nCols + j + 1, (i + 1)*nCols + j } };
                tri[1] = { { (i + 1)*nCols + j + 1, (i + 1)*nCols + j, i*nCols + j + 1 } };
                triangles.push_back(tri[0]);
                triangles.push_back(tri[1]);
            }
        }
        auto floorMesh = std::make_shared<SurfaceMesh>();
        floorMesh->initialize(vertList, triangles);

        auto materialFloor = std::make_shared<RenderMaterial>();
        materialFloor->setDisplayMode(RenderMaterial::DisplayMode::WIREFRAME_SURFACE);
        floorMesh->setRenderMaterial(materialFloor);

        auto floorMapP2V = std::make_shared<OneToOneMap>();
        floorMapP2V->setMaster(floorMesh);
        floorMapP2V->setSlave(floorMesh);
        floorMapP2V->compute();

        auto floorMapP2C = std::make_shared<OneToOneMap>();
        floorMapP2C->setMaster(floorMesh);
        floorMapP2C->setSlave(floorMesh);
        floorMapP2C->compute();

        auto floorMapC2V = std::make_shared<OneToOneMap>();
        floorMapC2V->setMaster(floorMesh);
        floorMapC2V->setSlave(floorMesh);
        floorMapC2V->compute();

        auto floor = std::make_shared<PbdObject>("Floor");
        floor->setCollidingGeometry(floorMesh);
        floor->setVisualGeometry(floorMesh);
        floor->setPhysicsGeometry(floorMesh);
        floor->setPhysicsToCollidingMap(floorMapP2C);
        floor->setPhysicsToVisualMap(floorMapP2V);
        floor->setCollidingToVisualMap(floorMapC2V);

        auto pbdModel2 = std::make_shared<PbdModel>();
        pbdModel2->setModelGeometry(floorMesh);
        pbdModel2->configure(/*Number of Constraints*/ 0,
            /*Mass*/ 0.0,
            /*Proximity*/ 0.1,
            /*Contact stiffness*/ 1.0);
        floor->setDynamicalModel(pbdModel2);

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

    // Light
    auto light = std::make_shared<DirectionalLight>("light");
    light->setFocalPoint(Vec3d(5, -8, -5));
    light->setIntensity(1);
    scene->addLight(light);

    sdk->setActiveScene(scene);
    sdk->startSimulation(true);
}

void testPbdFluidBenchmarking()
{
    std::vector<int> nPointsList = { 5, 10, 20 };
    std::vector<int> cubeSizeList = { 1, 1, 2 };

    int nPointsPerSide = 10;
    double cubeLength = 1;

    auto sdk = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("PBDFluidBenchmarking");

    scene->getCamera()->setPosition(0, 10.0, 25.0);

    //Create PointSet
    StdVectorOfVec3d vertList;
    int nPoints = pow(nPointsPerSide, 3);
    const double spacing = cubeLength / nPointsPerSide;

    vertList.resize(nPoints);
    for (int i = 0; i < nPointsPerSide; ++i)
    {
        for (int j = 0; j < nPointsPerSide; j++)
        {
            for (int k = 0; k < nPointsPerSide; k++)
            {
                vertList[i*nPointsPerSide*nPointsPerSide + j*nPointsPerSide + k] =
                    Vec3d((double)i * spacing, (double)j * spacing, (double)k * spacing);
            }
        }
    }

    auto cubeMesh = std::make_shared<PointSet>();
    cubeMesh->initialize(vertList);

    auto material = std::make_shared<RenderMaterial>();
    material->setDiffuseColor(Color::Blue);
    material->setSphereGlyphSize(.1);
    cubeMesh->setRenderMaterial(material);

    auto cube = std::make_shared<PbdObject>("Cube");
    cube->setCollidingGeometry(cubeMesh);
    cube->setVisualGeometry(cubeMesh);
    cube->setPhysicsGeometry(cubeMesh);

    auto pbdModel = std::make_shared<PbdModel>();
    pbdModel->setModelGeometry(cubeMesh);
    pbdModel->configure(/*Number of Constraints*/ 1,
        /*Constraint configuration*/ "ConstantDensity 1.0 0.3",
        /*Mass*/ 1.0,
        /*Gravity*/ "0 -9.8 0",
        /*TimeStep*/ 0.005,
        /*FixedPoint*/ "",
        /*NumberOfIterationInConstraintSolver*/ 2,
        /*Proximity*/ 0.2,
        /*Contact stiffness*/ 1.0);
    cube->setDynamicalModel(pbdModel);

    auto pbdSolver = std::make_shared<PbdSolver>();
    pbdSolver->setPbdObject(cube);
    scene->addNonlinearSolver(pbdSolver);

    scene->addSceneObject(cube);

    // plane
    double width = 40.0;
    double height = 40.0;
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
            vertList[i*nCols + j] = Vec3d(x - 20, -0.5, y - 20);
        }
    }

    // c. Add connectivity data
    std::vector<SurfaceMesh::TriangleArray> triangles;
    for (std::size_t i = 0; i < nRows - 1; ++i)
    {
        for (std::size_t j = 0; j < nCols - 1; j++)
        {
            SurfaceMesh::TriangleArray tri[2];
            tri[0] = { { i*nCols + j, i*nCols + j + 1, (i + 1)*nCols + j } };
            tri[1] = { { (i + 1)*nCols + j + 1, (i + 1)*nCols + j, i*nCols + j + 1 } };
            triangles.push_back(tri[0]);
            triangles.push_back(tri[1]);
        }
    }

    auto floorMeshColliding = std::make_shared<SurfaceMesh>();
    floorMeshColliding->initialize(vertList, triangles);
    auto floorMeshVisual = std::make_shared<SurfaceMesh>();
    floorMeshVisual->initialize(vertList, triangles);
    auto floorMeshPhysics = std::make_shared<SurfaceMesh>();
    floorMeshPhysics->initialize(vertList, triangles);

    auto floorMapP2V = std::make_shared<OneToOneMap>();
    floorMapP2V->setMaster(floorMeshPhysics);
    floorMapP2V->setSlave(floorMeshVisual);
    floorMapP2V->compute();

    auto floorMapP2C = std::make_shared<OneToOneMap>();
    floorMapP2C->setMaster(floorMeshPhysics);
    floorMapP2C->setSlave(floorMeshColliding);
    floorMapP2C->compute();

    auto floorMapC2V = std::make_shared<OneToOneMap>();
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
    pbdModel2->setModelGeometry(floorMeshPhysics);
    pbdModel2->configure(/*Number of Constraints*/ 0,
        /*Mass*/ 0.0,
        /*Proximity*/ 0.1,
        /*Contact stiffness*/ 1.0);
    floor->setDynamicalModel(pbdModel2);

    auto pbdSolverfloor = std::make_shared<PbdSolver>();
    pbdSolverfloor->setPbdObject(floor);
    scene->addNonlinearSolver(pbdSolverfloor);

    scene->addSceneObject(floor);

    // Collisions
    auto colGraph = scene->getCollisionGraph();
    auto pair = std::make_shared<PbdInteractionPair>(PbdInteractionPair(cube, floor));
    pair->setNumberOfInterations(2);

    auto dynaModel1 = std::static_pointer_cast<PbdModel>(cube->getDynamicalModel());

    colGraph->addInteractionPair(pair);

    // print UPS
    auto ups = std::make_shared<UPSCounter>();
    apiutils::printUPS(sdk->getSceneManager(scene), ups);

    // Light (white)
    auto whiteLight = std::make_shared<DirectionalLight>("whiteLight");
    whiteLight->setFocalPoint(Vec3d(5, -8, -5));
    whiteLight->setIntensity(7);
    scene->addLight(whiteLight);

    scene->getCamera()->setPosition(0, 10.0, 10.0);

    sdk->setActiveScene(scene);
    sdk->startSimulation(true);
}

void testPbdFluid()
{
    auto sdk = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("PBDFluidTest");

    scene->getCamera()->setPosition(0, 10.0, 15.0);

    // dragon
    auto tetMesh = MeshIO::read(iMSTK_DATA_ROOT "/asianDragon/asianDragon.veg");
    if (!tetMesh)
    {
        LOG(WARNING) << "Could not read mesh from file.";
        return;
    }

    auto fluidMesh = std::make_shared<PointSet>();
    fluidMesh->initialize(tetMesh->getInitialVertexPositions());

    auto material1 = std::make_shared<RenderMaterial>();
    material1->setDiffuseColor(Color::Blue);
    material1->setSphereGlyphSize(.15);
    fluidMesh->setRenderMaterial(material1);

    auto deformableObj = std::make_shared<PbdObject>("Dragon");
    deformableObj->setVisualGeometry(fluidMesh);
    deformableObj->setCollidingGeometry(fluidMesh);
    deformableObj->setPhysicsGeometry(fluidMesh);

    auto pbdModel = std::make_shared<PbdModel>();
    pbdModel->setModelGeometry(fluidMesh);
    pbdModel->configure(/*Number of Constraints*/ 1,
        /*Constraint configuration*/ "ConstantDensity 1.0 0.3",
        /*Mass*/ 1.0,
        /*Gravity*/ "0 -9.8 0",
        /*TimeStep*/ 0.005,
        /*FixedPoint*/ "",
        /*NumberOfIterationInConstraintSolver*/ 2,
        /*Proximity*/ 0.1,
        /*Contact stiffness*/ 1.0);
    deformableObj->setDynamicalModel(pbdModel);

    auto pbdSolver = std::make_shared<PbdSolver>();
    pbdSolver->setPbdObject(deformableObj);
    scene->addNonlinearSolver(pbdSolver);

    scene->addSceneObject(deformableObj);

    // box
    StdVectorOfVec3d vertList;
    int nSides = 5;
    double width = 40.0;
    double height = 40.0;
    int nRows = 2;
    int nCols = 2;
    vertList.resize(nRows*nCols*nSides);
    const double dy = width / (double)(nCols - 1);
    const double dx = height / (double)(nRows - 1);
    for (int i = 0; i < nRows; ++i)
    {
        for (int j = 0; j < nCols; j++)
        {
            const double y = (double)dy*j;
            const double x = (double)dx*i;
            vertList[i*nCols + j] = Vec3d(x - 20, -10.0, y - 20);
        }
    }

    // c. Add connectivity data
    std::vector<SurfaceMesh::TriangleArray> triangles;
    for (std::size_t i = 0; i < nRows - 1; ++i)
    {
        for (std::size_t j = 0; j < nCols - 1; j++)
        {
            SurfaceMesh::TriangleArray tri[2];
            tri[0] = { { i*nCols + j, i*nCols + j + 1, (i + 1)*nCols + j } };
            tri[1] = { { (i + 1)*nCols + j + 1, (i + 1)*nCols + j, i*nCols + j + 1 } };
            triangles.push_back(tri[0]);
            triangles.push_back(tri[1]);
        }
    }

    int nPointPerSide = nRows * nCols;
    //sidewalls 1 and 2 of box
    width = 10.0;
    height = 40.0;
    nRows = 2;
    nCols = 2;
    const double dz = width / (double)(nCols - 1);
    const double dx1 = height / (double)(nRows - 1);
    for (int i = 0; i < nRows; ++i)
    {
        for (int j = 0; j < nCols; j++)
        {
            const double z = (double)dz*j;
            const double x = (double)dx1*i;
            vertList[(nPointPerSide)+i*nCols + j] = Vec3d(x - 20, z - 10.0, 20);
            vertList[(nPointPerSide*2)+i*nCols + j] = Vec3d(x - 20, z - 10.0, -20);
        }
    }

    // c. Add connectivity data
    for (std::size_t i = 0; i < nRows - 1; ++i)
    {
        for (std::size_t j = 0; j < nCols - 1; j++)
        {
            SurfaceMesh::TriangleArray tri[2];
            tri[0] = { { (nPointPerSide)+i*nCols + j, (nPointPerSide)+i*nCols + j + 1, (nPointPerSide)+(i + 1)*nCols + j } };
            tri[1] = { { (nPointPerSide)+(i + 1)*nCols + j + 1, (nPointPerSide)+(i + 1)*nCols + j, (nPointPerSide)+i*nCols + j + 1 } };
            triangles.push_back(tri[0]);
            triangles.push_back(tri[1]);
            tri[0] = { { (nPointPerSide*2)+i*nCols + j, (nPointPerSide*2)+i*nCols + j + 1, (nPointPerSide*2)+(i + 1)*nCols + j } };
            tri[1] = { { (nPointPerSide*2)+(i + 1)*nCols + j + 1, (nPointPerSide*2)+(i + 1)*nCols + j, (nPointPerSide*2)+i*nCols + j + 1 } };
            triangles.push_back(tri[0]);
            triangles.push_back(tri[1]);
        }
    }

    //sidewalls 3 and 4 of box
    width = 10.0;
    height = 40.0;
    nRows = 2;
    nCols = 2;
    const double dz1 = width / (double)(nCols - 1);
    const double dy1 = height / (double)(nRows - 1);
    for (int i = 0; i < nRows; ++i)
    {
        for (int j = 0; j < nCols; j++)
        {
            const double z = (double)dz1*j;
            const double y = (double)dy1*i;
            vertList[(nPointPerSide * 3)+i*nCols + j] = Vec3d(20, z - 10.0, y-20);
            vertList[(nPointPerSide * 4) + i*nCols + j] = Vec3d(-20, z - 10.0, y-20);
        }
    }

    // c. Add connectivity data
    for (std::size_t i = 0; i < nRows - 1; ++i)
    {
        for (std::size_t j = 0; j < nCols - 1; j++)
        {
            SurfaceMesh::TriangleArray tri[2];
            tri[0] = { { (nPointPerSide * 3)+i*nCols + j, (nPointPerSide * 3)+i*nCols + j + 1, (nPointPerSide * 3)+(i + 1)*nCols + j } };
            tri[1] = { { (nPointPerSide * 3)+(i + 1)*nCols + j + 1, (nPointPerSide * 3)+(i + 1)*nCols + j, (nPointPerSide * 3)+i*nCols + j + 1 } };
            triangles.push_back(tri[0]);
            triangles.push_back(tri[1]);
            tri[0] = { { (nPointPerSide * 4) + i*nCols + j, (nPointPerSide * 4) + i*nCols + j + 1, (nPointPerSide * 4) + (i + 1)*nCols + j } };
            tri[1] = { { (nPointPerSide * 4) + (i + 1)*nCols + j + 1, (nPointPerSide * 4) + (i + 1)*nCols + j, (nPointPerSide * 4) + i*nCols + j + 1 } };
            triangles.push_back(tri[0]);
            triangles.push_back(tri[1]);
        }
    }

    auto floorMeshColliding = std::make_shared<SurfaceMesh>();
    floorMeshColliding->initialize(vertList, triangles);
    auto floorMeshVisual = std::make_shared<SurfaceMesh>();
    floorMeshVisual->initialize(vertList, triangles);
    auto floorMeshPhysics = std::make_shared<SurfaceMesh>();
    floorMeshPhysics->initialize(vertList, triangles);


    auto floorMapP2V = std::make_shared<OneToOneMap>();
    floorMapP2V->setMaster(floorMeshPhysics);
    floorMapP2V->setSlave(floorMeshVisual);
    floorMapP2V->compute();


    auto floorMapP2C = std::make_shared<OneToOneMap>();
    floorMapP2C->setMaster(floorMeshPhysics);
    floorMapP2C->setSlave(floorMeshColliding);
    floorMapP2C->compute();

    auto floorMapC2V = std::make_shared<OneToOneMap>();
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
    pbdModel2->setModelGeometry(floorMeshPhysics);
    pbdModel2->configure(/*Number of Constraints*/ 0,
        /*Mass*/ 0.0,
        /*Proximity*/ 0.1,
        /*Contact stiffness*/ 1.0);
    floor->setDynamicalModel(pbdModel2);

    auto pbdSolverfloor = std::make_shared<PbdSolver>();
    pbdSolverfloor->setPbdObject(floor);
    scene->addNonlinearSolver(pbdSolverfloor);

    scene->addSceneObject(floor);

    // Collisions
    auto colGraph = scene->getCollisionGraph();
    auto pair = std::make_shared<PbdInteractionPair>(PbdInteractionPair(deformableObj, floor));
    pair->setNumberOfInterations(2);

    colGraph->addInteractionPair(pair);

    // Light (white)
    auto whiteLight = std::make_shared<DirectionalLight>("whiteLight");
    whiteLight->setFocalPoint(Vec3d(5, -8, -5));
    whiteLight->setIntensity(7);
    scene->addLight(whiteLight);

    // print UPS
    auto ups = std::make_shared<UPSCounter>();
    apiutils::printUPS(sdk->getSceneManager(scene), ups);

    sdk->setActiveScene(scene);
    sdk->startSimulation(true);
}

void testLineMesh()
{
#ifdef iMSTK_USE_OPENHAPTICS
    // SDK and Scene
    auto sdk = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("TestLineMesh");

    // Device clients
    auto client0 = std::make_shared<HDAPIDeviceClient>(phantomOmni1Name);

    // Device Server
    auto server = std::make_shared<HDAPIDeviceServer>();
    server->addDeviceClient(client0);
    sdk->addModule(server);

    auto blade = std::make_shared<VirtualCouplingPBDObject>("blade", client0);
    auto linesTool = std::make_shared<VirtualCouplingPBDObject>("linesTool", client0);
    auto tool = std::make_shared<VirtualCouplingPBDObject>("tool", client0);

    bool line;
    bool clothTest;

    std::cout << "Select tool: 0 for blade, 1 for lines..." << std::endl;
    std::cin >> line;
    std::cout << "Select deformable: 0 for volumetric mesh, 1 for surface mesh..." << std::endl;
    std::cin >> clothTest;

    if (line)
    {
        // Make LineMesh
        StdVectorOfVec3d vertList;
        vertList.resize(3);
        vertList[0] = Vec3d(0.0, -10.0, -10.0);
        vertList[1] = Vec3d(0.0, 0.0, -10.0);
        vertList[2] = Vec3d(0.0, 0.0, -30.0);
        std::vector<std::vector<int>> connectivity;
        for (int i = 0; i < 2; )
        {
            std::vector<int> line;
            line.push_back(i);
            i++;
            line.push_back(i);

            connectivity.push_back(line);
        }

        auto lineMesh = std::make_shared<LineMesh>();
        lineMesh->setInitialVertexPositions(vertList);
        lineMesh->setVertexPositions(vertList);
        lineMesh->setConnectivity(connectivity);

        linesTool->setCollidingGeometry(lineMesh);
        linesTool->setVisualGeometry(lineMesh);
        linesTool->setPhysicsGeometry(lineMesh);
        //linesTool->initialize(/*Number of constraints*/ 1,
        //                      /*Constraint configuration*/ "Distance 100",
        //                      /*Mass*/ 0.0,
        //                      /*Gravity*/ "0 -9.8 0",
        //                      /*TimeStep*/ 0.002,
        //                      /*FixedPoint*/ "0 1 2",
        //                      /*NumberOfIterationInConstraintSolver*/ 5,
        //                      /*Proximity*/ 0.1,
        //                      /*Contact stiffness*/ 0.1);
        scene->addSceneObject(linesTool);
    }
    else
    {
        std::string path2obj = iMSTK_DATA_ROOT "/ETI/resources/Tools/blade2.obj";

        auto bladeMesh = MeshIO::read(path2obj);

        blade->setCollidingGeometry(bladeMesh);
        blade->setVisualGeometry(bladeMesh);
        blade->setPhysicsGeometry(bladeMesh);
        //blade->initialize(/*Number of constraints*/ 1,
        //                  /*Constraint configuration*/ "Distance 0.1",
        //                  /*Mass*/ 0.0,
        //                  /*Gravity*/ "0 0 0",
        //                  /*TimeStep*/ 0.001,
        //                  /*FixedPoint*/ "",
        //                  /*NumberOfIterationInConstraintSolver*/ 5,
        //                  /*Proximity*/ 0.1,
        //                  /*Contact stiffness*/ 0.01);
        scene->addSceneObject(blade);
    }


    if (clothTest)
    {
        StdVectorOfVec3d vertList;
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
        std::vector<SurfaceMesh::TriangleArray> triangles;
        for (size_t i = 0; i < nRows - 1; ++i)
        {
            for (size_t j = 0; j < nCols - 1; j++)
            {
                SurfaceMesh::TriangleArray tri[2];
                tri[0] = { { i*nCols + j, i*nCols + j + 1, (i + 1)*nCols + j } };
                tri[1] = { { (i + 1)*nCols + j + 1, (i + 1)*nCols + j, i*nCols + j + 1 } };
                triangles.push_back(tri[0]);
                triangles.push_back(tri[1]);
            }
        }

        auto clothMeshVisual = std::make_shared<SurfaceMesh>();
        clothMeshVisual->initialize(vertList, triangles);

        auto clothObject = std::make_shared<PbdObject>("cloth");
        clothObject->setCollidingGeometry(clothMeshVisual);
        clothObject->setVisualGeometry(clothMeshVisual);
        clothObject->setPhysicsGeometry(clothMeshVisual);
        auto clothModel = std::make_shared<PbdModel>();
        clothModel->configure(/*Number of constraints*/ 2,
            /*Constraint configuration*/ "Distance 0.1",
            /*Constraint configuration*/ "Dihedral 0.001",
            /*Mass*/ 0.1,
            /*Gravity*/ "0 -9.8 0",
            /*TimeStep*/ 0.001,
            /*FixedPoint*/ fixed_corner.c_str(),
            /*NumberOfIterationInConstraintSolver*/ 5,
            /*Proximity*/ 0.1,
            /*Contact stiffness*/ 0.1);
        clothObject->setDynamicalModel(clothModel);
        scene->addSceneObject(clothObject);

        std::cout << "nbr of vertices in cloth mesh" << clothMeshVisual->getNumVertices() << std::endl;

        // Collisions
        auto clothTestcolGraph = scene->getCollisionGraph();
        if (line)
        {
            tool = linesTool;
        }
        else
        {
            tool = blade;
        }

        auto pair1 = std::make_shared<PbdInteractionPair>(PbdInteractionPair(tool, clothObject));
        pair1->setNumberOfInterations(5);

        clothTestcolGraph->addInteractionPair(pair1);

        scene->getCamera()->setPosition(0, 0, 50);
    }
    else
    {
        //auto tetMesh = MeshIO::read(iMSTK_DATA_ROOT"/ETI/resources/Human/tongue.veg");
        auto tetMesh = MeshIO::read(iMSTK_DATA_ROOT "/asianDragon/asianDragon.veg");
        if (!tetMesh)
        {
            LOG(WARNING) << "Could not read mesh from file.";
            return;
        }

        auto volTetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(tetMesh);
        if (!volTetMesh)
        {
            LOG(WARNING) << "Dynamic pointer cast from PointSet to TetrahedralMesh failed!";
            return;
        }

        auto vs = volTetMesh->getInitialVertexPositions();
        Vec3d tmpPos;
        for (int i = 0; i < volTetMesh->getNumVertices(); ++i)
        {
            tmpPos = volTetMesh->getVertexPosition(i);
            tmpPos[1] -= 15;
            volTetMesh->setVertexPosition(i, tmpPos);
        }
        volTetMesh->setInitialVertexPositions(volTetMesh->getVertexPositions());

        auto surfMesh = std::make_shared<SurfaceMesh>();
        volTetMesh->extractSurfaceMesh(surfMesh);

        auto surfMeshVisual = std::make_shared<SurfaceMesh>();
        volTetMesh->extractSurfaceMesh(surfMeshVisual);

        auto dragonMapP2V = std::make_shared<OneToOneMap>();
        dragonMapP2V->setMaster(volTetMesh);
        dragonMapP2V->setSlave(surfMeshVisual);
        dragonMapP2V->compute();

        auto dragonMapC2V = std::make_shared<OneToOneMap>();
        dragonMapC2V->setMaster(surfMesh);
        dragonMapC2V->setSlave(surfMeshVisual);
        dragonMapC2V->compute();

        auto dragonMapP2C = std::make_shared<OneToOneMap>();
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

        auto pbdDefModel = std::make_shared<PbdModel>();
        pbdDefModel->configure(/*Number of Constraints*/ 1,
            /*Constraint configuration*/ "FEM NeoHookean 10.0 0.3",
            /*Mass*/ 0.1,
            /*Gravity*/ "0 0 0",
            /*TimeStep*/ 0.001,
            /*FixedPoint*/ "",
            /*NumberOfIterationInConstraintSolver*/ 5,
            /*Proximity*/ 0.1,
            /*Contact stiffness*/ 0.01);
        deformableObj->setDynamicalModel(pbdDefModel);
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

    // Light
    auto light = std::make_shared<DirectionalLight>("light");
    light->setFocalPoint(Vec3d(5, -8, -5));
    light->setIntensity(1);
    scene->addLight(light);

    // Run
    sdk->setActiveScene(scene);
    sdk->startSimulation(true);
#endif
}

void testScreenShotUtility()
{
    // SDK and Scene
    auto sdk = std::make_shared<SimulationManager>();
    auto sceneTest = sdk->createNewScene("ScreenShotUtilityTest");

    // Plane
    auto planeGeom = std::make_shared<Plane>();
    planeGeom->setWidth(10);
    auto planeObj = std::make_shared<VisualObject>("VisualPlane");
    planeObj->setVisualGeometry(planeGeom);

    // Cube
    auto cubeGeom = std::make_shared<Cube>();
    cubeGeom->setWidth(0.5);
    cubeGeom->setPosition(1.0, -1.0, 0.5);
    // rotates could be replaced by cubeGeom->setOrientationAxis(1,1,1) (normalized inside)
    cubeGeom->rotate(UP_VECTOR, PI_4, Geometry::TransformType::ApplyToData);
    cubeGeom->rotate(RIGHT_VECTOR, PI_4, Geometry::TransformType::ApplyToData);
    auto cubeObj = std::make_shared<VisualObject>("VisualCube");
    cubeObj->setVisualGeometry(cubeGeom);

    // Sphere
    auto sphereGeom = std::make_shared<Sphere>();
    sphereGeom->setRadius(0.3);
    sphereGeom->setPosition(0, 2, 0);
    auto sphereObj = std::make_shared<VisualObject>("VisualSphere");
    sphereObj->setVisualGeometry(sphereGeom);

    // Light (white)
    auto whiteLight = std::make_shared<PointLight>("whiteLight");
    whiteLight->setIntensity(100);
    whiteLight->setPosition(Vec3d(5, 8, 5));

    // Light (red)
    auto colorLight = std::make_shared<SpotLight>("colorLight");
    colorLight->setPosition(Vec3d(4, -3, 1));
    colorLight->setFocalPoint(Vec3d(0, 0, 0));
    colorLight->setIntensity(100);
    colorLight->setColor(Color::Red);
    colorLight->setSpotAngle(15);

    // Add in scene
    sceneTest->addSceneObject(planeObj);
    sceneTest->addSceneObject(cubeObj);
    sceneTest->addSceneObject(sphereObj);
    sceneTest->addLight(whiteLight);
    sceneTest->addLight(colorLight);

    // Update Camera
    auto cam1 = sceneTest->getCamera();
    cam1->setPosition(Vec3d(-5.5, 2.5, 32));
    cam1->setFocalPoint(Vec3d(1, 1, 0));

#ifndef iMSTK_USE_Vulkan
    auto viewer = std::dynamic_pointer_cast<VTKViewer>(sdk->getViewer());
    auto screenShotUtility
        = std::dynamic_pointer_cast<VTKScreenCaptureUtility>(viewer->getScreenCaptureUtility());
    // Set up for screen shot
    sdk->getViewer()->getScreenCaptureUtility()->setScreenShotPrefix("screenShot_");
    // Create a call back on key press of 'b' to take the screen shot
    viewer->setOnCharFunction('b', [&](VTKInteractorStyle* c) -> bool
    {
        screenShotUtility->saveScreenShot();
        return false;
    });
#endif

    // Run
    sdk->setActiveScene(sceneTest);
    sdk->startSimulation(true);
}

void testDeformableBodyCollision()
{
    // SDK and Scene
    auto sdk = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("DeformableBodyCollisionTest");

    auto geom = std::make_shared<Plane>();

    geom->setWidth(100);
    geom->setPosition(Vec3d(0., -20., 0.));

    auto planeObj = std::make_shared<CollidingObject>("VisualPlane");
    planeObj->setVisualGeometry(geom);
    planeObj->setCollidingGeometry(geom);
    scene->addSceneObject(planeObj);

    // Load a tetrahedral mesh
    auto tetMesh = MeshIO::read(iMSTK_DATA_ROOT "/oneTet/oneTet.veg");
    if (!tetMesh)
    {
        LOG(WARNING) << "Could not read mesh from file.";
        return;
    }

    // Extract the surface mesh
    auto surfMesh = std::make_shared<SurfaceMesh>();
    auto volTetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(tetMesh);
    if (!volTetMesh)
    {
        LOG(WARNING) << "Dynamic pointer cast from PointSet to TetrahedralMesh failed!";
        return;
    }
    volTetMesh->extractSurfaceMesh(surfMesh);

    // Construct one to one nodal map based on the above meshes
    auto oneToOneNodalMap = std::make_shared<OneToOneMap>();
    oneToOneNodalMap->setMaster(tetMesh);
    oneToOneNodalMap->setSlave(surfMesh);
    oneToOneNodalMap->compute();

    // Configure the dynamic model
    auto dynaModel = std::make_shared<FEMDeformableBodyModel>();
    dynaModel->configure(iMSTK_DATA_ROOT "/oneTet/oneTet.config");
    dynaModel->setModelGeometry(volTetMesh);

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
    //deformableObj->initialize();
    scene->addSceneObject(deformableObj);

    // Create a nonlinear system and solver
    auto nlSystem = std::make_shared<NonLinearSystem>(dynaModel->getFunction(), dynaModel->getFunctionGradient());
    std::vector<LinearProjectionConstraint> linProj;
    for (auto id : dynaModel->getFixNodeIds())
    {
        linProj.push_back(LinearProjectionConstraint(id, true));
    }
    nlSystem->setUnknownVector(dynaModel->getUnknownVec());
    nlSystem->setUpdateFunction(dynaModel->getUpdateFunction());
    nlSystem->setUpdatePreviousStatesFunction(dynaModel->getUpdatePrevStateFunction());

    // create a non-linear solver and add to the scene
    auto nlSolver = std::make_shared<NewtonSolver>();
    auto cgLinSolver = std::make_shared<ConjugateGradient>();// create a linear solver to be used in the NL solver
    cgLinSolver->setLinearProjectors(&linProj);
    nlSolver->setLinearSolver(cgLinSolver);
    nlSolver->setSystem(nlSystem);
    scene->addNonlinearSolver(nlSolver);

    // Create collision detection and handling
    scene->getCollisionGraph()->addInteractionPair(deformableObj,
        planeObj,
        CollisionDetection::Type::PointSetToPlane,
        CollisionHandling::Type::Penalty,
        CollisionHandling::Type::None);

    // Set Camera configuration
    auto cam = scene->getCamera();
    cam->setPosition(Vec3d(0, 20, 20));
    cam->setFocalPoint(Vec3d(0, 0, 0));

    // Light
    auto light = std::make_shared<DirectionalLight>("light");
    light->setFocalPoint(Vec3d(5, -8, -5));
    light->setIntensity(1);
    scene->addLight(light);

    // Run
    sdk->setActiveScene(scene);
    sdk->startSimulation(true);
}

void liverToolInteraction()
{
    // SDK and Scene
    auto sdk = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("LiverToolInteraction");

    //----------------------------------------------------------
    // Create plane visual scene object
    //----------------------------------------------------------
    auto planeObj = apiutils::createVisualAnalyticalSceneObject(
        Geometry::Type::Plane, scene, "VisualPlane", 100, Vec3d(0., -20., 0.));

    //----------------------------------------------------------
    // Create liver FE deformable scene object
    //----------------------------------------------------------

    // Load a tetrahedral mesh
    auto tetMesh = MeshIO::read(iMSTK_DATA_ROOT "/oneTet/oneTet.veg");
    //auto tetMesh = MeshIO::read(iMSTK_DATA_ROOT"/liver/liver.veg");

    if (!tetMesh)
    {
        LOG(WARNING) << "Could not read mesh from file.";
        return;
    }

    // Extract the surface mesh
    auto surfMesh = std::make_shared<SurfaceMesh>();
    auto volTetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(tetMesh);
    if (!volTetMesh)
    {
        LOG(WARNING) << "Dynamic pointer cast from PointSet to TetrahedralMesh failed!";
        return;
    }
    volTetMesh->extractSurfaceMesh(surfMesh);

    // Construct one to one nodal map based on the above meshes
    auto oneToOneNodalMap = std::make_shared<OneToOneMap>();
    oneToOneNodalMap->setMaster(tetMesh);
    oneToOneNodalMap->setSlave(surfMesh);
    oneToOneNodalMap->compute();

    // Configure the dynamic model
    auto dynaModel = std::make_shared<FEMDeformableBodyModel>();
    //dynaModel->configure(iMSTK_DATA_ROOT"/liver/liver.config");
    dynaModel->configure(iMSTK_DATA_ROOT "/oneTet/oneTet.config");
    dynaModel->setModelGeometry(volTetMesh);

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
    //deformableObj->initialize();
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
    nlSystem->setUnknownVector(dynaModel->getUnknownVec());
    nlSystem->setUpdateFunction(dynaModel->getUpdateFunction());
    nlSystem->setUpdatePreviousStatesFunction(dynaModel->getUpdatePrevStateFunction());

    // create a non-linear solver and add to the scene
    auto nlSolver = std::make_shared<NewtonSolver>();
    auto cgLinSolver = std::make_shared<ConjugateGradient>();// create a linear solver to be used in the NL solver
    cgLinSolver->setLinearProjectors(&linProj);
    nlSolver->setLinearSolver(cgLinSolver);
    nlSolver->setSystem(nlSystem);
    //nlSolver->setToFullyImplicit();
    scene->addNonlinearSolver(nlSolver);

    //----------------------------------------------------------
    // Create collision detection and handling
    //----------------------------------------------------------
    //auto collData = std::make_shared<CollisionData>();
    /*auto collisioDet = std::make_shared<MeshToPlaneCD>(volTetMesh,
                                                              std::dynamic_pointer_cast<Plane>(planeObj->getCollidingGeometry()),
                                                              *collData.get());
    auto collHandling = std::make_shared<PenaltyMeshToRigidCH>(CollisionHandling::Side::A, *collData.get(), deformableObj);*/


    //----------------------------------------------------------
    // Create laparoscopic tool controller
    //----------------------------------------------------------
#ifdef iMSTK_USE_OPENHAPTICS

    // Device clients
    auto client = std::make_shared<HDAPIDeviceClient>(phantomOmni1Name);

    // Device Server
    auto server = std::make_shared<HDAPIDeviceServer>();
    server->addDeviceClient(client);
    sdk->addModule(server);

    // Create laparoscopic tool related scene objects
    /*auto pivot = apiutils::createAndAddVisualSceneObject(scene, iMSTK_DATA_ROOT"/laptool/pivot.obj", "pivot");
    auto upperJaw = apiutils::createAndAddVisualSceneObject(scene, iMSTK_DATA_ROOT"/laptool/upper.obj", "upperJaw");
    auto lowerJaw = apiutils::createAndAddVisualSceneObject(scene, iMSTK_DATA_ROOT"/laptool/lower.obj", "lowerJaw");*/

    // Sphere0
    auto sphere0Obj = apiutils::createCollidingAnalyticalSceneObject(
        Geometry::Type::Sphere, scene, "Sphere0", 3, Vec3d(1, 0.5, 0));

    auto trackingCtrl = std::make_shared<DeviceTracker>(client);
    //trackingCtrl->setTranslationScaling(100);
    auto lapToolController = std::make_shared<SceneObjectController>(sphere0Obj, trackingCtrl);
    /*auto lapToolController = std::make_shared<LaparoscopicToolController>(pivot, upperJaw, lowerJaw, trackingCtrl);
    lapToolController->setJawRotationAxis(Vec3d(1.0, 0, 0));*/
    scene->addObjectController(lapToolController);

    scene->getCollisionGraph()->addInteractionPair(deformableObj,
                                                   sphere0Obj,
                                                   CollisionDetection::Type::PointSetToSphere,
                                                   CollisionHandling::Type::Penalty,
                                                   CollisionHandling::Type::None);

#endif

    // Set Camera configuration
    auto cam = scene->getCamera();
    cam->setPosition(Vec3d(0, 20, 20));
    cam->setFocalPoint(Vec3d(0, 0, 0));

    // Light
    auto light = std::make_shared<DirectionalLight>("light");
    light->setFocalPoint(Vec3d(5, -8, -5));
    light->setIntensity(1);
    scene->addLight(light);

    // Run
    sdk->setActiveScene(scene);
    sdk->startSimulation(true);
}

void testVirtualCoupling()
{
    // SDK and Scene
    auto sdk = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("VirtualCouplingTest");

    // Create a plane in the scene
    auto planeGeom = std::make_shared<Plane>();
    planeGeom->setWidth(400);
    planeGeom->setPosition(0.0, -50, 0.0);
    auto planeObj = std::make_shared<CollidingObject>("Plane");
    planeObj->setVisualGeometry(planeGeom);
    planeObj->setCollidingGeometry(planeGeom);
    scene->addSceneObject(planeObj);

    // Create the virtual coupling object controller
#ifdef iMSTK_USE_OPENHAPTICS

    // Device clients
    auto client = std::make_shared<HDAPIDeviceClient>(phantomOmni1Name);

    // Device Server
    auto server = std::make_shared<HDAPIDeviceServer>();
    server->addDeviceClient(client);
    sdk->addModule(server);

    // Device tracker
    auto deviceTracker = std::make_shared<DeviceTracker>(client);

    // Create a virtual coupling object
    auto visualGeom = std::make_shared<Sphere>();
    visualGeom->setRadius(20);
    auto collidingGeom = std::make_shared<Sphere>();
    collidingGeom->setRadius(20);
    auto obj = std::make_shared<CollidingObject>("VirtualCouplingObject");
    obj->setCollidingGeometry(collidingGeom);
    obj->setVisualGeometry(visualGeom);

    auto material = std::make_shared<RenderMaterial>();
    //material->setVisibilityOff();
    visualGeom->setRenderMaterial(material);

    // Add virtual coupling object (with visual, colliding, and physics geometry) in the scene.
    scene->addSceneObject(obj);

    // Create and add virtual coupling object controller in the scene
    auto objController = std::make_shared<SceneObjectController>(obj, deviceTracker);
    scene->addObjectController(objController);

    // Create a collision graph
    auto graph = scene->getCollisionGraph();
    auto pair = graph->addInteractionPair(planeObj, obj,
                                          CollisionDetection::Type::UnidirectionalPlaneToSphere,
                                          CollisionHandling::Type::None,
                                          CollisionHandling::Type::VirtualCoupling);

    // Customize collision handling algorithm
    auto colHandlingAlgo = std::dynamic_pointer_cast<VirtualCouplingCH>(pair->getCollisionHandlingB());
    colHandlingAlgo->setStiffness(5e-01);
    colHandlingAlgo->setDamping(0.005);

#endif

    // Move Camera
    auto cam = scene->getCamera();
    cam->setPosition(Vec3d(200, 200, 200));
    cam->setFocalPoint(Vec3d(0, 0, 0));

    // Light
    auto light = std::make_shared<DirectionalLight>("light");
    light->setFocalPoint(Vec3d(5, -8, -5));
    light->setIntensity(1);
    scene->addLight(light);

    //Run
    sdk->setActiveScene(scene);
    sdk->startSimulation(false);
}

void testGeometryTransforms()
{
    // SDK and Scene
    auto sdk = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("testGeometryTransforms");

    auto sceneObj = apiutils::createAndAddVisualSceneObject(scene, iMSTK_DATA_ROOT "/asianDragon/asianDragon.obj", "Dragon");

    auto surfaceMesh = sceneObj->getVisualGeometry();
    surfaceMesh->scale(5., Geometry::TransformType::ConcatenateToTransform);

    //  Plane
    auto planeGeom = std::make_shared<Plane>();
    planeGeom->scale(80, Geometry::TransformType::ConcatenateToTransform);
    planeGeom->translate(0, -20, 0, Geometry::TransformType::ConcatenateToTransform);
    planeGeom->rotate(Vec3d(0, 1., 0), PI/4, Geometry::TransformType::ConcatenateToTransform);

    auto planeObj = std::make_shared<VisualObject>("Plane");
    planeObj->setVisualGeometry(planeGeom);
    scene->addSceneObject(planeObj);

    //  Cube
    auto cubeGeom = std::make_shared<Cube>();
    cubeGeom->setWidth(20.);
    cubeGeom->scale(0.5, Geometry::TransformType::ConcatenateToTransform);
    cubeGeom->rotate(Vec3d(1., 1., 0), PI / 4, Geometry::TransformType::ApplyToData);

    auto cubeObj = std::make_shared<VisualObject>("Cube");
    cubeObj->setVisualGeometry(cubeGeom);
    scene->addSceneObject(cubeObj);

    //  Cylinder
    auto CylinderGeom = std::make_shared<Cylinder>();
    CylinderGeom->setRadius(4.);
    CylinderGeom->setLength(8.);
    CylinderGeom->scale(0.4, Geometry::TransformType::ConcatenateToTransform);
    CylinderGeom->rotate(Vec3d(1., 1., 0), PI / 2, Geometry::TransformType::ApplyToData);

    auto CylinderObj = std::make_shared<VisualObject>("Cylinder");
    CylinderObj->setVisualGeometry(CylinderGeom);
    scene->addSceneObject(CylinderObj);

    // Rotate the dragon every frame
    auto rotateFunc =
        [&surfaceMesh](Module* module)
        {
            surfaceMesh->rotate(Vec3d(1., 0, 0), PI / 1000, Geometry::TransformType::ApplyToData);
        };
    sdk->getSceneManager(scene)->setPostUpdateCallback(rotateFunc);

    // Set Camera configuration
    auto cam = scene->getCamera();
    cam->setPosition(Vec3d(0, 30, 30));
    cam->setFocalPoint(Vec3d(0, 0, 0));

    // Light
    auto light = std::make_shared<DirectionalLight>("light");
    light->setFocalPoint(Vec3d(5, -8, -5));
    light->setIntensity(1);
    scene->addLight(light);

    // Run
    sdk->setActiveScene(scene);
    sdk->startSimulation(false);
}


void testPicking()
{
    // SDK and Scene
    auto sdk = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("PickingTest");

    //----------------------------------------------------------
    // Create plane visual scene object
    //----------------------------------------------------------
    auto planeObj = apiutils::createVisualAnalyticalSceneObject(
        Geometry::Type::Plane, scene, "VisualPlane", 100, Vec3d(0., -20., 0.));

    //----------------------------------------------------------
    // Create Nidus FE deformable scene object
    //----------------------------------------------------------
    // Load a tetrahedral mesh
    auto tetMesh = MeshIO::read(iMSTK_DATA_ROOT "/oneTet/oneTet.veg");

    if (!tetMesh)
    {
        LOG(WARNING) << "Could not read mesh from file.";
        return;
    }
    // Extract the surface mesh
    auto volTetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(tetMesh);
    if (!volTetMesh)
    {
        LOG(WARNING) << "Dynamic pointer cast from PointSet to TetrahedralMesh failed!";
        return;
    }
    auto surfMesh = std::make_shared<SurfaceMesh>();
    volTetMesh->extractSurfaceMesh(surfMesh);

    // Construct one to one nodal map based on the above meshes
    auto oneToOneNodalMap = std::make_shared<OneToOneMap>();
    oneToOneNodalMap->setMaster(tetMesh);
    oneToOneNodalMap->setSlave(surfMesh);
    oneToOneNodalMap->compute();

    // Configure the dynamic model
    auto dynaModel = std::make_shared<FEMDeformableBodyModel>();
    dynaModel->configure(iMSTK_DATA_ROOT "/oneTet/oneTet.config");
    dynaModel->setModelGeometry(volTetMesh);

    // Create and add Backward Euler time integrator
    auto timeIntegrator = std::make_shared<BackwardEuler>(0.01);
    dynaModel->setTimeIntegrator(timeIntegrator);

    // Configure Scene Object
    auto physicsObj = std::make_shared<DeformableObject>("deformableObj");
    physicsObj->setVisualGeometry(surfMesh);
    physicsObj->setCollidingGeometry(volTetMesh);
    physicsObj->setPhysicsGeometry(volTetMesh);
    physicsObj->setPhysicsToVisualMap(oneToOneNodalMap);
    physicsObj->setDynamicalModel(dynaModel);
    //physicsObj->initialize();
    scene->addSceneObject(physicsObj);

    //----------------------------------------------------------
    // Create a nonlinear system and its solver
    //----------------------------------------------------------
    auto nlSystem = std::make_shared<NonLinearSystem>(dynaModel->getFunction(), dynaModel->getFunctionGradient());
    std::vector<LinearProjectionConstraint> linProj;
    for (auto id : dynaModel->getFixNodeIds())
    {
        linProj.push_back(LinearProjectionConstraint(id, true));
    }
    nlSystem->setUnknownVector(dynaModel->getUnknownVec());
    nlSystem->setUpdateFunction(dynaModel->getUpdateFunction());
    nlSystem->setUpdatePreviousStatesFunction(dynaModel->getUpdatePrevStateFunction());
    std::vector<LinearProjectionConstraint> dynLinProj;

    // create a non-linear solver and add to the scene
    auto nlSolver = std::make_shared<NewtonSolver>();
    auto cgLinSolver = std::make_shared<ConjugateGradient>();// create a linear solver to be used in the NL solver
    cgLinSolver->setLinearProjectors(&linProj);
    cgLinSolver->setDynamicLinearProjectors(&dynLinProj);
    nlSolver->setLinearSolver(cgLinSolver);
    nlSolver->setSystem(nlSystem);
    scene->addNonlinearSolver(nlSolver);

    //----------------------------------------------------------
    // Create object controller
    //----------------------------------------------------------
#ifdef iMSTK_USE_OPENHAPTICS

    // Device clients
    auto client = std::make_shared<HDAPIDeviceClient>(phantomOmni1Name);

    // Device Server
    auto server = std::make_shared<HDAPIDeviceServer>();
    server->addDeviceClient(client);
    sdk->addModule(server);

    // Sphere0
    auto sphereForPickObj = apiutils::createCollidingAnalyticalSceneObject(
        Geometry::Type::Sphere, scene, "Sphere0", 1, Vec3d(0., 0., 0.));

    auto pickTrackingCtrl = std::make_shared<DeviceTracker>(client);
    //pickTrackingCtrl->setTranslationOffset(Vec3d(0., 0., 24.));

    auto pickController = std::make_shared<SceneObjectController>(sphereForPickObj, pickTrackingCtrl);
    scene->addObjectController(pickController);

    CollisionData coldata;
    auto sphereGeo = std::dynamic_pointer_cast<Sphere>(sphereForPickObj->getCollidingGeometry());

    // Create collision detection for picking
    auto pickingCD = std::make_shared<PointSetToSpherePickingCD>(volTetMesh, sphereGeo, coldata);
    pickingCD->setDeviceTrackerAndButton(pickTrackingCtrl, 0);

    // Create contact handling for picking
    auto pickingCH = std::make_shared<PickingCH>(CollisionHandling::Side::A, coldata, physicsObj);
    pickingCH->setDynamicLinearProjectors(&dynLinProj);

    // Create collision pair
    scene->getCollisionGraph()->addInteractionPair(physicsObj, sphereForPickObj, pickingCD, pickingCH, nullptr);
#endif

    // Set Camera configuration
    auto cam = scene->getCamera();
    auto camPosition = Vec3d(0, 40, 80);
    cam->setPosition(camPosition);
    cam->setFocalPoint(Vec3d(0, 0, 0));
    // Run
    sdk->setActiveScene(scene);
    sdk->startSimulation(true);
}


void testBoneDrilling()
{
    // SDK and Scene
    auto sdk = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("BoneDrillingTest");

    // Add virtual coupling object in the scene.
#ifdef iMSTK_USE_OPENHAPTICS

    // Device clients
    auto client = std::make_shared<HDAPIDeviceClient>(phantomOmni1Name);

    // Device Server
    auto server = std::make_shared<HDAPIDeviceServer>();
    server->addDeviceClient(client);
    sdk->addModule(server);

    // Device tracker
    auto deviceTracker = std::make_shared<DeviceTracker>(client);

    // Create bone scene object
    // Load the mesh
    auto tetMesh = MeshIO::read(iMSTK_DATA_ROOT "/asianDragon/asianDragon.veg");
    if (!tetMesh)
    {
        LOG(WARNING) << "Could not read mesh from file.";
        return;
    }
    auto bone = std::make_shared<CollidingObject>("Bone");
    bone->setCollidingGeometry(tetMesh);
    bone->setVisualGeometry(tetMesh);
    scene->addSceneObject(bone);

    // Create a virtual coupling object: Drill
    auto drillVisualGeom = std::make_shared<Sphere>();
    drillVisualGeom->setRadius(3.);
    auto drillCollidingGeom = std::make_shared<Sphere>();
    drillCollidingGeom->setRadius(3.);
    auto drill = std::make_shared<CollidingObject>("Drill");
    drill->setCollidingGeometry(drillCollidingGeom);
    drill->setVisualGeometry(drillVisualGeom);
    scene->addSceneObject(drill);

    // Create and add virtual coupling object controller in the scene
    auto objController = std::make_shared<SceneObjectController>(drill, deviceTracker);
    scene->addObjectController(objController);

    // Create a collision graph
    auto graph = scene->getCollisionGraph();
    auto pair = graph->addInteractionPair(bone,
                                          drill,
                                          CollisionDetection::Type::PointSetToSphere,
                                          CollisionHandling::Type::BoneDrilling,
                                          CollisionHandling::Type::None);

#endif

    // Light
    auto light = std::make_shared<DirectionalLight>("light");
    light->setFocalPoint(Vec3d(5, -8, -5));
    light->setIntensity(1);
    scene->addLight(light);

    //Run
    auto cam = scene->getCamera();
    cam->setPosition(Vec3d(0, 0, 15));

    sdk->setActiveScene(scene);
    sdk->startSimulation(false);
}

// test virtual coupling for cylinder to sphere collision
void testVirtualCouplingCylinder()
{
    // SDK and Scene
    auto sdk = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("VirtualCouplingCylinderSphereTest");

    // Create a plane in the scene (visual)
    auto planeGeom = std::make_shared<Plane>();
    planeGeom->setWidth(10);
    planeGeom->setPosition(0.0, -50, 0.0);
    auto planeObj = std::make_shared<VisualObject>("Plane");
    planeObj->setVisualGeometry(planeGeom);
    scene->addSceneObject(planeObj);

    // Create the virtual coupling object controller
#ifdef iMSTK_USE_OPENHAPTICS

    // Device clients
    auto client = std::make_shared<HDAPIDeviceClient>(phantomOmni1Name);

    // Device Server
    auto server = std::make_shared<HDAPIDeviceServer>();
    server->addDeviceClient(client);
    sdk->addModule(server);

    // Device tracker
    auto deviceTracker = std::make_shared<DeviceTracker>(client);

    // Create a virtual coupling object
    auto visualGeom = std::make_shared<Sphere>();
    visualGeom->setRadius(5.);
    auto collidingGeom = std::make_shared<Sphere>();
    collidingGeom->setRadius(5.);
    auto virtualCouplingSphereObj = std::make_shared<CollidingObject>("VirtualCouplingObject");
    virtualCouplingSphereObj->setCollidingGeometry(collidingGeom);
    virtualCouplingSphereObj->setVisualGeometry(visualGeom);
    scene->addSceneObject(virtualCouplingSphereObj);

    // Create colliding cylinder scene object
    auto CylinderGeomVis = std::make_shared<Cylinder>();
    CylinderGeomVis->setRadius(10.);
    CylinderGeomVis->setLength(40.);

    auto CylinderObj = std::make_shared<CollidingObject>("Cylinder");
    CylinderObj->setVisualGeometry(CylinderGeomVis);
    CylinderObj->setCollidingGeometry(CylinderGeomVis);
    scene->addSceneObject(CylinderObj);

    // Create and add virtual coupling object controller in the scene
    auto objController = std::make_shared<SceneObjectController>(virtualCouplingSphereObj,
                                                                        deviceTracker);
    scene->addObjectController(objController);

    // Create a collision graph
    auto graph = scene->getCollisionGraph();
    auto pair = graph->addInteractionPair(CylinderObj, virtualCouplingSphereObj,
        CollisionDetection::Type::SphereToCylinder,
        CollisionHandling::Type::None,
        CollisionHandling::Type::VirtualCoupling);

    // Customize collision handling algorithm
    auto colHandlingAlgo = std::dynamic_pointer_cast<VirtualCouplingCH>(pair->getCollisionHandlingB());
    colHandlingAlgo->setStiffness(5e-1);
    colHandlingAlgo->setDamping(0.005);

#endif

    // Move Camera
    auto cam = scene->getCamera();
    cam->setPosition(Vec3d(200, 200, 200));
    cam->setFocalPoint(Vec3d(0, 0, 0));

    // Light
    auto light = std::make_shared<DirectionalLight>("light");
    light->setFocalPoint(Vec3d(5, -8, -5));
    light->setIntensity(1);
    scene->addLight(light);

    //Run
    sdk->setActiveScene(scene);
    sdk->startSimulation(false);
}

void testRigidBody()
{
    // SDK and Scene
    auto sdk = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("RigidObjectPhysicsTest");
    // Create a plane in the scene (visual)
    auto planeGeom = std::make_shared<Plane>();
    planeGeom->setWidth(10);
    planeGeom->setPosition(0.0, 2.5, 0.0);
    auto planeObj = std::make_shared<VisualObject>("Plane");
    planeObj->setVisualGeometry(planeGeom);
    scene->addSceneObject(planeObj);

    int counter = 1000;

#ifdef iMSTK_USE_ODE
    //Initialize rigid body
    //Initialize imstk side of things.
    auto sphereObj = apiutils::createVisualAnalyticalSceneObject(
        Geometry::Type::Sphere, scene, "VisualSphere", 1.0, Vec3d(0., 3.0, 0.));

    auto sceneManager = sdk->getSceneManager("RigidObjectPhysics");

    sceneManager->setPreInitCallback([&](Module* module)
    {
        RigidObject::initOde();
        RigidObject::setup();
    });

    sceneManager->setPreUpdateCallback([&](Module* module)
    {
        if (counter == 0)
        {
            RigidObject::simulationStep();
            counter = 1000;
        }
        else
        {
            counter--;
        }
    });
    sceneManager->setPostUpdateCallback([&](Module* module)
    {
        Vec3d pos;
        Mat3d matrix;
        RigidObject::getGeometryConfig(pos, matrix);
        sphereObj->getVisualGeometry()->setTranslation(pos);
    });
    sceneManager->setPostCleanUpCallback([&](Module* module)
    {
        RigidObject::closeOde();
    });
#endif

    // Move Camera
    auto cam = scene->getCamera();
    cam->setPosition(Vec3d(10, 10, 10));
    cam->setFocalPoint(Vec3d(0, 0, 0));

    //Run
    sdk->setActiveScene(scene);
    sdk->startSimulation(false);
}

// Example modified from SFML/Examples
void testSound(const std::string& filename)
{
#ifdef iMSTK_AUDIO_ENABLED
    // Load a sound buffer from a .wav file
    sf::SoundBuffer buffer;
    if (!buffer.loadFromFile(filename))
    {
        LOG(WARNING) << "testSound: Could not open the input sound file: " << filename;
        return;
    }

    // Display sound informations
    std::cout << filename << std::endl;
    std::cout << " " << buffer.getDuration().asSeconds() << " seconds" << std::endl;
    std::cout << " " << buffer.getSampleRate() << " samples / sec" << std::endl;
    std::cout << " " << buffer.getChannelCount() << " channels" << std::endl;

    // Create a sound instance and play it
    sf::Sound sound(buffer);
    sound.setPosition(0., 0., 0.);
    sound.setMinDistance(5.);
    sound.setAttenuation(10.);

    sound.play();

    Vec3d listnerPos(-5., -5., -5.);
    // Loop while the sound is playing
    while (sound.getStatus() == sf::Sound::Playing)
    {
        // Leave some CPU time for other processes
        sf::sleep(sf::milliseconds(100));

        // Move the listener away
        listnerPos += Vec3d(.2, .2, .2);
        sf::Listener::setPosition(listnerPos.x(), listnerPos.y(), listnerPos.z());

        // Display the playing position
        std::cout << "\rPlaying... " << sound.getPlayingOffset().asSeconds() << " sec        ";
        std::cout << std::flush;
    }
    std::cout << "\n" << std::endl;
#else
    LOG(INFO) << "testSound: Audio is supported only on windows!";
#endif
}

// Example modified from SFML/Examples
void playMusic(const std::string& filename)
{
#ifdef iMSTK_AUDIO_ENABLED
    // Load an ogg music file
    sf::Music music;
    if (!music.openFromFile(filename))
    {
        LOG(WARNING) << "playMusic: Could not open the input music file: " << filename;
        return;
    }

    // Display music informations
    std::cout << filename << ":" << std::endl;
    std::cout << " " << music.getDuration().asSeconds() << " seconds" << std::endl;
    std::cout << " " << music.getSampleRate() << " samples / sec" << std::endl;
    std::cout << " " << music.getChannelCount() << " channels" << std::endl;

    // Play it
    music.play();

    // Loop while the music is playing
    while (music.getStatus() == sf::Music::Playing)
    {
        // Leave some CPU time for other processes
        sf::sleep(sf::milliseconds(100));

        // Display the playing position
        std::cout << "\rPlaying... " << music.getPlayingOffset().asSeconds() << " sec        ";
        std::cout << std::flush;
    }
    std::cout << "\n" << std::endl;
#else
    LOG(INFO) << "playMusic: Audio is supported only on windows!";
#endif
}

void testAudio()
{
    std::cout << "--Testing audio--\n\n";

    // Test a sound
    testSound(iMSTK_DATA_ROOT "/sound/canary.wav");

    // Test music from an .ogg file
    playMusic(iMSTK_DATA_ROOT "/sound/orchestral.ogg");
}


int main()
{
    std::cout << "****************\n"
              << "Starting Sandbox\n"
              << "****************\n";

    /*------------------
    Test rendering
    ------------------*/
    //testMultiObjectWithTextures();
    //testViewer();
    //testScreenShotUtility();
    //testCapsule();


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
    //testGeometryTransforms();


    /*------------------
    Test physics
    ------------------*/
    //testPbdVolume();
    //testPbdCloth();
    //testPbdCollision();
    //testPbdFluidBenchmarking();
    //testPbdFluid();
    testDeformableBody();
    //testDeformableBodyCollision();
    //liverToolInteraction();
    //testPicking();


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
    //testAudio();
    //testScenesManagement();
    //testVectorPlotters();
    //testVirtualCoupling();
    //testBoneDrilling();
    //testVirtualCouplingCylinder();
    //testRigidBody();
    //testGraph();

    return 0;
}
