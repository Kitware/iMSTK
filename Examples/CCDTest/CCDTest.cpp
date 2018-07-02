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
    sdk->startSimulation(SimulationStatus::PAUSED);
}

int main()
{
    testMeshCCD();
    
    return 0;
}
