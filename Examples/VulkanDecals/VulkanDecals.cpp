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

using namespace imstk;

void testDecals()
{
    // SDK and Scene
    auto sdk = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("DecalsTest");

    // Position camera
    auto cam = scene->getCamera();
    cam->setPosition(0, 3, 6);
    cam->setFocalPoint(0, 0, 0);

    // Decals
    auto decalMaterial = std::make_shared<RenderMaterial>();
    auto decalTexture = std::make_shared<Texture>(iMSTK_DATA_ROOT "/decals/blood_decal.png", Texture::DIFFUSE);
    decalMaterial->addTexture(decalTexture);

    auto decalPool = std::make_shared<DecalPool>();
    auto decalObject = std::make_shared<VisualObject>("Decals");
    decalPool->setRenderMaterial(decalMaterial);
    decalObject->setVisualGeometry(decalPool);

    for (int i = -1; i < 2; i++)
    {
        auto decal = decalPool->addDecal();
        decal->setPosition(i, 0, 0.25);
        decal->setRotation(RIGHT_VECTOR, PI_4);
        decal->setScaling(0.5);
    }

    scene->addSceneObject(decalObject);

    // Sphere
    auto sphere = apiutils::createVisualAnalyticalSceneObject(Geometry::Type::Sphere, scene, "sphere", 0.25);
    sphere->getVisualGeometry()->translate(1, 0, 0);

    // Cube
    auto cube = apiutils::createVisualAnalyticalSceneObject(Geometry::Type::Cube, scene, "cube", 0.25);
    cube->getVisualGeometry()->translate(0, 0, 0.1);
    cube->getVisualGeometry()->rotate(UP_VECTOR, PI_4);

    // Plane
    auto plane = apiutils::createVisualAnalyticalSceneObject(Geometry::Type::Plane, scene, "plane", 10);

    // Light
    auto light = std::make_shared<DirectionalLight>("Light");
    light->setIntensity(7);
    light->setColor(Color(1.0, 0.95, 0.8));
    light->setFocalPoint(Vec3d(-1, -1, 0));
    scene->addLight(light);

    // Run
    sdk->setActiveScene(scene);
    sdk->startSimulation(SimulationStatus::PAUSED);
}

int main()
{
    testDecals();

    return 0;
}
