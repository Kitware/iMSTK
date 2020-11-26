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
#include "imstkFeDeformableObject.h"
#include "imstkFEMDeformableBodyModel.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLight.h"
#include "imstkLogger.h"
#include "imstkMeshIO.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkOneToOneMap.h"
#include "imstkPlane.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
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
        planeGeom->setPosition(0.0, -6.0, 0.0);
        imstkNew<CollidingObject> planeObj("Plane");
        planeObj->setVisualGeometry(planeGeom);
        planeObj->setCollidingGeometry(planeGeom);
        scene->addSceneObject(planeObj);

        // Light
        imstkNew<DirectionalLight> light("light");
        light->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
        light->setIntensity(1);
        scene->addLight(light);
    }

    // Run the simulation
    {
        // Setup a viewer to render in its own thread
        imstkNew<VTKViewer> viewer("Viewer 1");
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene in its own thread
        imstkNew<SceneManager> sceneManager("Scene Manager 1");
        sceneManager->setActiveScene(scene);
        viewer->addChildThread(sceneManager); // SceneManager will start/stop with viewer

        // Add mouse and keyboard controls to the viewer
        {
            imstkNew<MouseSceneControl> mouseControl(viewer->getMouseDevice());
            mouseControl->setSceneManager(sceneManager);
            viewer->addControl(mouseControl);

            imstkNew<KeyboardSceneControl> keyControl(viewer->getKeyboardDevice());
            keyControl->setSceneManager(sceneManager);
            keyControl->setViewer(viewer);
            viewer->addControl(keyControl);
        }

        // Start viewer running, scene as paused
        sceneManager->requestStatus(ThreadStatus::Paused);
        viewer->start();
    }

    return 0;
}

std::shared_ptr<FeDeformableObject>
makeFEDeformableObject(std::shared_ptr<TetrahedralMesh> tetMesh)
{
    imstkNew<SurfaceMesh> surfMesh;
    tetMesh->extractSurfaceMesh(surfMesh, true);
    surfMesh->flipNormals();

    // Configure dynamic model
    imstkNew<FEMDeformableBodyModel> dynaModel;
    imstkNew<FEMModelConfig>         config;
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
    mat->setLineWidth(4.0);
    mat->setEdgeColor(Color::Orange);
    imstkNew<VisualModel> surfMeshModel(surfMesh.get());
    surfMeshModel->setRenderMaterial(mat);

    // Scene object 1: Dragon
    imstkNew<FeDeformableObject> deformableObj("Dragon");
    deformableObj->addVisualModel(surfMeshModel);
    deformableObj->setPhysicsGeometry(tetMesh);
    // Map simulated geometry to visual
    deformableObj->setPhysicsToVisualMap(std::make_shared<OneToOneMap>(tetMesh, surfMesh));
    deformableObj->setDynamicalModel(dynaModel);

    return deformableObj;
}
