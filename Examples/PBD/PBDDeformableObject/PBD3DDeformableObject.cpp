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

#include "imstkCamera.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLight.h"
#include "imstkLogger.h"
#include "imstkMeshIO.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkTetraTriangleMap.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

using namespace imstk;

///
/// \brief Create a PbdObject and add it to a \p scene
///
std::shared_ptr<PbdObject> createAndAddPbdObject(const std::string& tetMeshName);

// mesh file names
const std::string& tetMeshFileName = iMSTK_DATA_ROOT "textured_organs/heart_volume.vtk";

///
/// \brief This example demonstrates the soft body simulation
/// using Position based dynamics
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    imstkNew<Scene> scene("PBDVolume");
    scene->getActiveCamera()->setPosition(0, 2.0, 15.0);

    // create and add a PBD object
    scene->addSceneObject(createAndAddPbdObject(tetMeshFileName));

    // Light
    imstkNew<DirectionalLight> light("light");
    light->setFocalPoint(Vec3d(5, -8, -5));
    light->setIntensity(1.1);
    scene->addLight(light);

    // Run the simulation
    {
        // Setup a viewer to render in its own thread
        imstkNew<VTKViewer> viewer("Viewer");
        viewer->setActiveScene(scene);
        viewer->setBackgroundColors(Vec3d(0.3285, 0.3285, 0.6525), Vec3d(0.13836, 0.13836, 0.2748), true);

        // Setup a scene manager to advance the scene in its own thread
        imstkNew<SceneManager> sceneManager("Scene Manager");
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

std::shared_ptr<PbdObject>
createAndAddPbdObject(const std::string& tetMeshName)
{
    auto tetMesh = MeshIO::read<TetrahedralMesh>(tetMeshName);
    tetMesh->rotate(Vec3d(1.0, 0.0, 0.0), -1.3, Geometry::TransformType::ApplyToData);
    imstkNew<SurfaceMesh> surfMesh;
    tetMesh->extractSurfaceMesh(surfMesh, true);

    imstkNew<RenderMaterial> material;
    material->setDisplayMode(RenderMaterial::DisplayMode::Surface);
    material->setColor(Color(220. / 255.0, 100. / 255.0, 70. / 255.0));
    material->setMetalness(100.9f);
    material->setRoughness(0.5);
    material->setEdgeColor(Color::Teal);
    material->setAmbientLightCoeff(50.);
    material->setShadingModel(RenderMaterial::ShadingModel::Phong);
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    imstkNew<VisualModel> visualModel(surfMesh.get());
    visualModel->setRenderMaterial(material);

    imstkNew<PbdObject> deformableObj("DeformableObject");
    imstkNew<PbdModel> pbdModel;
    pbdModel->setModelGeometry(tetMesh);

    // Configure model
    imstkNew<PBDModelConfig> pbdParams;

    // FEM constraint
    pbdParams->m_femParams->m_YoungModulus = 500.0;
    pbdParams->m_femParams->m_PoissonRatio = 0.3;
    pbdParams->m_fixedNodeIds = { 75, 82, 84, 94, 95, 105, 110, 124, 139, 150, 161, 171, 350 };
    pbdParams->enableFEMConstraint(PbdConstraint::Type::FEMTet, PbdFEMConstraint::MaterialType::StVK);

    // Other parameters
    pbdParams->m_uniformMassValue = 1.0;
    pbdParams->m_gravity    = Vec3d(0, -9.8, 0);
    pbdParams->m_iterations = 6;

    // Set the parameters
    pbdModel->configure(pbdParams);
    pbdModel->setDefaultTimeStep(0.02);
    pbdModel->setTimeStepSizeType(imstk::TimeSteppingType::Fixed);

    deformableObj->setDynamicalModel(pbdModel);
    deformableObj->addVisualModel(visualModel);
    deformableObj->setPhysicsGeometry(tetMesh);
    deformableObj->setPhysicsToVisualMap(std::make_shared<TetraTriangleMap>(tetMesh, surfMesh));

    return deformableObj;
}
