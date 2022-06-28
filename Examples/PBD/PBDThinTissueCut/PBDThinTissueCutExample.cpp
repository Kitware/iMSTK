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
#include "imstkDirectionalLight.h"
#include "imstkGeometryUtilities.h"
#include "imstkHapticDeviceClient.h"
#include "imstkHapticDeviceManager.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectCutting.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSceneObjectController.h"
#include "imstkSimulationManager.h"
#include "imstkSurfaceMesh.h"
#include "imstkVertexLabelVisualModel.h"
#include "imstkVTKViewer.h"

using namespace imstk;

///
/// \brief Creates cloth object
///
static std::shared_ptr<PbdObject>
makeClothObj(const std::string& name,
             const double       width,
             const double       height,
             const int          nRows,
             const int          nCols)
{
    imstkNew<PbdObject> clothObj(name);

    // Setup the Geometry
    std::shared_ptr<SurfaceMesh> clothMesh =
        GeometryUtils::toTriangleGrid(Vec3d::Zero(),
            Vec2d(width, height), Vec2i(nRows, nCols));

    // Setup the Parameters
    imstkNew<PbdModelConfig> pbdParams;
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 1.0e3);
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Dihedral, 1.0e3);
    pbdParams->m_fixedNodeIds     = { 0, static_cast<size_t>(nCols) - 1 };
    pbdParams->m_uniformMassValue = width * height / static_cast<double>(nRows * nCols);
    pbdParams->m_gravity    = Vec3d(0.0, -9.8, 0.0);
    pbdParams->m_dt         = 0.005;
    pbdParams->m_iterations = 5;

    // Setup the Model
    imstkNew<PbdModel> pbdModel;
    pbdModel->setModelGeometry(clothMesh);
    pbdModel->configure(pbdParams);

    // Setup the VisualModel
    imstkNew<RenderMaterial> material;
    material->setBackFaceCulling(false);
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);

    imstkNew<VisualModel> visualModel;
    visualModel->setGeometry(clothMesh);
    visualModel->setRenderMaterial(material);

    imstkNew<VertexLabelVisualModel> vertexLabelModel;
    vertexLabelModel->setGeometry(clothMesh);
    vertexLabelModel->setFontSize(20.0);
    vertexLabelModel->setTextColor(Color::Red);

    // Setup the Object
    clothObj->addVisualModel(visualModel);
    clothObj->addVisualModel(vertexLabelModel);
    clothObj->setPhysicsGeometry(clothMesh);
    clothObj->setCollidingGeometry(clothMesh);
    clothObj->setDynamicalModel(pbdModel);

    return clothObj;
}

///
/// \brief This example demonstrates the concept of PBD cutting
/// for haptic interaction. NOTE: Requires GeoMagic Touch device
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Scene
    imstkNew<Scene> scene("PBDThinTissueCut");

    // Create a cutting plane object in the scene
    std::shared_ptr<SurfaceMesh> cutGeom =
        GeometryUtils::toTriangleGrid(Vec3d::Zero(),
            Vec2d(40.0, 40.0), Vec2i(2, 2));
    cutGeom->setTranslation(Vec3d(-10, -20, 0));
    cutGeom->updatePostTransformData();
    imstkNew<CollidingObject> cutObj("CuttingObject");
    cutObj->setVisualGeometry(cutGeom);
    cutObj->setCollidingGeometry(cutGeom);
    cutObj->getVisualModel(0)->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    cutObj->getVisualModel(0)->getRenderMaterial()->setBackFaceCulling(false);
    scene->addSceneObject(cutObj);

    // Create a pbd cloth object in the scene
    std::shared_ptr<PbdObject> clothObj = makeClothObj("Cloth", 50.0, 50.0, 7, 7);
    scene->addSceneObject(clothObj);

    // Add interaction pair for pbd cutting
    imstkNew<PbdObjectCutting> cuttingInteraction(clothObj, cutObj);

    // Device Server
    imstkNew<HapticDeviceManager>       server;
    std::shared_ptr<HapticDeviceClient> client = server->makeDeviceClient();

    // Create the virtual coupling object controller
    imstkNew<SceneObjectController> controller(cutObj, client);
    scene->addController(controller);

    // Camera
    scene->getActiveCamera()->setPosition(Vec3d(1.0, 1.0, 1.0) * 100.0);
    scene->getActiveCamera()->setFocalPoint(Vec3d(0, -50, 0));

    // Light
    imstkNew<DirectionalLight> light;
    light->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
    light->setIntensity(1.0);
    scene->addLight("light", light);

    {
        // Setup a viewer to render
        imstkNew<VTKViewer> viewer;
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene
        imstkNew<SceneManager> sceneManager;
        sceneManager->setActiveScene(scene);
        sceneManager->pause(); // Start simulation paused

        imstkNew<SimulationManager> driver;
        driver->addModule(server);
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.001);

        // Add mouse and keyboard controls to the viewer
        {
            imstkNew<MouseSceneControl> mouseControl(viewer->getMouseDevice());
            mouseControl->setSceneManager(sceneManager);
            viewer->addControl(mouseControl);

            imstkNew<KeyboardSceneControl> keyControl(viewer->getKeyboardDevice());
            keyControl->setSceneManager(sceneManager);
            keyControl->setModuleDriver(driver);
            viewer->addControl(keyControl);
        }

        // Queue haptic button press to be called after scene thread
        queueConnect<ButtonEvent>(client, &HapticDeviceClient::buttonStateChanged, sceneManager,
            [&](ButtonEvent* e)
            {
                // When button 0 is pressed replace the PBD cloth with a cut one
                if (e->m_button == 0 && e->m_buttonState == BUTTON_PRESSED)
                {
                    cuttingInteraction->apply();
                }
            });

        std::cout << "================================================\n";
        std::cout << "Haptic button 0 to cut the cloth.\n";
        std::cout << "================================================\n\n";

        driver->start();
    }
    return 0;
}
