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

#include "imstkBackwardEuler.h"
#include "imstkCamera.h"
#include "imstkDirectionalLight.h"
#include "imstkFeDeformableObject.h"
#include "imstkFemDeformableBodyModel.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLogger.h"
#include "imstkMeshIO.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkPlane.h"
#include "imstkPointwiseMap.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

using namespace imstk;

std::shared_ptr<FeDeformableObject> makeFEDeformableObject(std::shared_ptr<TetrahedralMesh> tetMesh);

enum Geom
{
    Dragon = 0,
    Heart
};

struct Input
{
    std::string meshFileName;
    std::vector<std::size_t> fixedNodeIds;
};

// Geom geom = Dragon;
const Geom geom = Heart;
Input      input;

///
/// \brief This example demonstrates the soft body simulation
/// using Finite elements
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    if (geom == Dragon)
    {
        input.meshFileName = iMSTK_DATA_ROOT "asianDragon/asianDragon.veg";
        input.fixedNodeIds = { 50, 126, 177 };
    }
    else if (geom == Heart)
    {
        input.meshFileName = iMSTK_DATA_ROOT "textured_organs/heart_volume.vtk";
        input.fixedNodeIds = { 75, 82, 84, 94, 95, 105, 110, 124, 139, 150, 161, 171, 350 };
    }

    // Construct the scene
    imstkNew<Scene> scene("DeformableBodyFEM");
    {
        std::shared_ptr<Camera> cam = scene->getActiveCamera();
        cam->setPosition(0.0, 2.0, -25.0);
        cam->setFocalPoint(0.0, 0.0, 0.0);

        // Load a tetrahedral mesh
        std::shared_ptr<TetrahedralMesh> tetMesh = MeshIO::read<TetrahedralMesh>(input.meshFileName);
        CHECK(tetMesh != nullptr) << "Could not read mesh from file.";

        // Scene object 1: fe-FeDeformableObject
        std::shared_ptr<FeDeformableObject> deformableObj = makeFEDeformableObject(tetMesh);
        scene->addSceneObject(deformableObj);

        // Scene object 2: Plane
        imstkNew<Plane> planeGeom;
        planeGeom->setWidth(40.0);
        planeGeom->setPosition(0.0, -8.0, 0.0);
        imstkNew<CollidingObject> planeObj("Plane");
        planeObj->setVisualGeometry(planeGeom);
        planeObj->setCollidingGeometry(planeGeom);
        scene->addSceneObject(planeObj);

        // Light
        imstkNew<DirectionalLight> light;
        light->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
        light->setIntensity(1);
        scene->addLight("light", light);
    }

    // Run the simulation
    {
        // Setup a viewer to render in its own thread
        imstkNew<VTKViewer> viewer;
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene in its own thread
        imstkNew<SceneManager> sceneManager;
        sceneManager->setActiveScene(scene);
        sceneManager->pause();

        imstkNew<SimulationManager> driver;
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.03);

        // Add mouse and keyboard controls to the viewer
        {
            auto mouseControl = std::make_shared<MouseSceneControl>();
            mouseControl->setDevice(viewer->getMouseDevice());
            mouseControl->setSceneManager(sceneManager);
            scene->addControl(mouseControl);

            auto keyControl = std::make_shared<KeyboardSceneControl>();
            keyControl->setDevice(viewer->getKeyboardDevice());
            keyControl->setSceneManager(sceneManager);
            keyControl->setModuleDriver(driver);
            scene->addControl(keyControl);
        }

        driver->start();
    }

    return 0;
}

std::shared_ptr<FeDeformableObject>
makeFEDeformableObject(std::shared_ptr<TetrahedralMesh> tetMesh)
{
    std::shared_ptr<SurfaceMesh> surfMesh = tetMesh->extractSurfaceMesh();

    // Configure dynamic model
    imstkNew<FemDeformableBodyModel> dynaModel;
    imstkNew<FemModelConfig>         config;
    config->m_fixedNodeIds = input.fixedNodeIds;
    dynaModel->configure(config);
    //dynaModel->configure(iMSTK_DATA_ROOT "/asianDragon/asianDragon.config");

    dynaModel->setTimeStepSizeType(TimeSteppingType::Fixed);
    dynaModel->setModelGeometry(tetMesh);
    imstkNew<BackwardEuler> timeIntegrator(0.01); // Create and add Backward Euler time integrator
    dynaModel->setTimeIntegrator(timeIntegrator);

    imstkNew<RenderMaterial> mat;
    mat->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    mat->setPointSize(10.0);
    mat->setLineWidth(2.0);
    mat->setEdgeColor(Color::Orange);

    // Scene object 1: Dragon
    imstkNew<FeDeformableObject> deformableObj("Dragon");
    deformableObj->setVisualGeometry(surfMesh);
    deformableObj->getVisualModel(0)->setRenderMaterial(mat);
    deformableObj->setPhysicsGeometry(tetMesh);
    // Map simulated geometry to visual
    deformableObj->setPhysicsToVisualMap(std::make_shared<PointwiseMap>(tetMesh, surfMesh));
    deformableObj->setDynamicalModel(dynaModel);

    return deformableObj;
}