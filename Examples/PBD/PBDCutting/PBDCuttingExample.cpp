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
#include "imstkHapticDeviceClient.h"
#include "imstkHapticDeviceManager.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkDirectionalLight.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectCuttingPair.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSceneObjectController.h"
#include "imstkSimulationManager.h"
#include "imstkSurfaceMesh.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

using namespace imstk;

// Parameters to play with
const double gWidth  = 50.0;
const double gHeight = 50.0;
const int    gNRows  = 12;
const int    gNCols  = 12;

///
/// \brief Creates cloth geometry
///
static std::shared_ptr<SurfaceMesh>
makeClothGeometry(const double width,
                  const double height,
                  const int    nRows,
                  const int    nCols)
{
    imstkNew<SurfaceMesh> clothMesh("Cloth_SurfaceMesh");

    imstkNew<VecDataArray<double, 3>> verticesPtr(nRows * nCols);
    VecDataArray<double, 3>&          vertices = *verticesPtr.get();
    const double                      dy       = width / static_cast<double>(nCols - 1);
    const double                      dx       = height / static_cast<double>(nRows - 1);
    for (int i = 0; i < nRows; i++)
    {
        for (int j = 0; j < nCols; j++)
        {
            vertices[i * nCols + j] = Vec3d(dx * static_cast<double>(i), 1.0, dy * static_cast<double>(j));
        }
    }

    // Add connectivity data
    imstkNew<VecDataArray<int, 3>> indicesPtr;
    VecDataArray<int, 3>&          indices = *indicesPtr.get();
    for (int i = 0; i < nRows - 1; i++)
    {
        for (int j = 0; j < nCols - 1; j++)
        {
            const int index1 = i * nCols + j;
            const int index2 = index1 + nCols;
            const int index3 = index1 + 1;
            const int index4 = index2 + 1;

            // Interleave [/][\] pattern
            if (i % 2 ^ j % 2)
            {
                indices.push_back(Vec3i(index1, index2, index3));
                indices.push_back(Vec3i(index4, index3, index2));
            }
            else
            {
                indices.push_back(Vec3i(index2, index4, index1));
                indices.push_back(Vec3i(index4, index3, index1));
            }
        }
    }

    clothMesh->initialize(verticesPtr, indicesPtr);

    return clothMesh;
}

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
    std::shared_ptr<SurfaceMesh> clothMesh(makeClothGeometry(width, height, nRows, nCols));

    // Setup the Parameters
    imstkNew<PBDModelConfig> pbdParams;
    pbdParams->enableConstraint(PbdConstraint::Type::Distance, 1.0e3);
    pbdParams->enableConstraint(PbdConstraint::Type::Dihedral, 1.0e3);
    pbdParams->m_fixedNodeIds     = { 0, static_cast<size_t>(nCols) - 1 };
    pbdParams->m_uniformMassValue = width * height / ((double)nRows * (double)nCols);
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

    imstkNew<VisualModel> visualModel(clothMesh);
    visualModel->setRenderMaterial(material);

    // Setup the Object
    clothObj->addVisualModel(visualModel);
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
    imstkNew<Scene> scene("PBDCutting");

    // Create a cutting plane object in the scene
    std::shared_ptr<SurfaceMesh> cutGeom(makeClothGeometry(40, 40, 2, 2));
    cutGeom->setTranslation(Vec3d(-10, -20, 0));
    cutGeom->updatePostTransformData();
    imstkNew<CollidingObject> cutObj("CuttingObject");
    cutObj->setVisualGeometry(cutGeom);
    cutObj->setCollidingGeometry(cutGeom);
    cutObj->getVisualModel(0)->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    scene->addSceneObject(cutObj);

    // Create a pbd cloth object in the scene
    std::shared_ptr<PbdObject> clothObj = makeClothObj("Cloth", gWidth, gHeight, gNRows, gNCols);
    scene->addSceneObject(clothObj);

    // Add interaction pair for pbd cutting
    imstkNew<PbdObjectCuttingPair> cuttingPair(clothObj, cutObj);

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

    //Run the simulation
    {
        // Setup a viewer to render in its own thread
        imstkNew<VTKViewer> viewer;
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene in its own thread
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

        // Queue keypress to be called after scene thread
        queueConnect<ButtonEvent>(client, &HapticDeviceClient::buttonStateChanged, sceneManager,
            [&](ButtonEvent* e)
        {
            // When i is pressed replace the PBD cloth with a cut one
            if (e->m_button == 0 && e->m_buttonState == BUTTON_PRESSED)
            {
                cuttingPair->apply();
            }
            });

        std::cout << "================================================\n";
        std::cout << "Key haptic button 0 to cut the cloth.\n";
        std::cout << "================================================\n\n";

        driver->start();
    }
    return 0;
}
