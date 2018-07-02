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
#include "imstkDecalPool.h"

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

// global variables
const std::string phantomOmni1Name = "Phantom1";
const std::string phantomOmni2Name = "Phantom2";
const std::string novintFalcon1Name = "device0";
const std::string novintFalcon2Name = "device1";

using namespace imstk;

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
    sdk->startSimulation(SimulationStatus::RUNNING);
}

int main()
{
   testVirtualCoupling();
    
    return 0;
}
