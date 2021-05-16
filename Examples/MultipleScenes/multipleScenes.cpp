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
#include "imstkConsoleModule.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkLight.h"
#include "imstkLogger.h"
#include "imstkMeshIO.h"
#include "imstkNew.h"
#include "imstkOneToOneMap.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPlane.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkViewer.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

using namespace imstk;

///
/// \brief Creates cloth geometry
///
static std::shared_ptr<SurfaceMesh>
makeClothGeometry(const double width,
                  const double height,
                  const int    nRows,
                  const int    nCols)
{
    imstkNew<SurfaceMesh> clothMesh;

    imstkNew<VecDataArray<double, 3>> verticesPtr(nRows * nCols);
    VecDataArray<double, 3>&          vertices = *verticesPtr.get();
    const double                      dy       = width / static_cast<double>(nCols - 1);
    const double                      dx       = height / static_cast<double>(nRows - 1);
    for (int i = 0; i < nRows; ++i)
    {
        for (int j = 0; j < nCols; j++)
        {
            vertices[i * nCols + j] = Vec3d(dx * static_cast<double>(i), 1.0, dy * static_cast<double>(j));
        }
    }

    // Add connectivity data
    imstkNew<VecDataArray<int, 3>> indicesPtr;
    VecDataArray<int, 3>&          indices = *indicesPtr.get();
    for (int i = 0; i < nRows - 1; ++i)
    {
        for (int j = 0; j < nCols - 1; j++)
        {
            const int index1 = i * nCols + j;
            const int index2 = index1 + nCols;
            const int index3 = index1 + 1;
            const int index4 = index2 + 1;

            // Interleave [/][\]
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
    pbdParams->enableConstraint(PbdConstraint::Type::Distance, 1.0e2);
    pbdParams->enableConstraint(PbdConstraint::Type::Dihedral, 1.0e1);
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
    clothObj->setDynamicalModel(pbdModel);

    return clothObj;
}

std::shared_ptr<Scene>
createSoftBodyScene(std::string sceneName)
{
    imstkNew<SceneConfig> sceneConfig;
    sceneConfig->lazyInitialization = true;
    imstkNew<Scene> scene(sceneName, sceneConfig);

    scene->getActiveCamera()->setPosition(0.0, 2.0, 15.0);

    // Load a sample mesh
    auto tetMesh = MeshIO::read<TetrahedralMesh>(iMSTK_DATA_ROOT "/asianDragon/asianDragon.veg");

    // Extract the surface mesh
    std::shared_ptr<SurfaceMesh> surfMesh = tetMesh->extractSurfaceMesh();

    imstkNew<RenderMaterial> material;
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    imstkNew<VisualModel> surfMeshModel(surfMesh);
    surfMeshModel->setRenderMaterial(material);

    imstkNew<PbdObject> deformableObj("Dragon");
    imstkNew<PbdModel>  pbdModel;
    pbdModel->setModelGeometry(tetMesh);

    // configure model
    imstkNew<PBDModelConfig> pbdParams;

    // FEM constraint
    pbdParams->m_femParams->m_YoungModulus = 100.0;
    pbdParams->m_femParams->m_PoissonRatio = 0.3;
    pbdParams->m_fixedNodeIds = { 51, 127, 178 };
    pbdParams->enableFEMConstraint(PbdConstraint::Type::FEMTet, PbdFEMConstraint::MaterialType::StVK);

    // Other parameters
    pbdParams->m_uniformMassValue = 1.0;
    pbdParams->m_gravity    = Vec3d(0.0, -9.8, 0.0);
    pbdParams->m_iterations = 45;

    // Set the parameters
    pbdModel->configure(pbdParams);
    deformableObj->setDynamicalModel(pbdModel);
    deformableObj->addVisualModel(surfMeshModel);
    deformableObj->setPhysicsGeometry(tetMesh);
    deformableObj->setPhysicsToVisualMap(std::make_shared<OneToOneMap>(tetMesh, surfMesh));
    deformableObj->setDynamicalModel(pbdModel);

    scene->addSceneObject(deformableObj);

    imstkNew<Plane> planeGeom;
    planeGeom->setWidth(40.0);
    planeGeom->setTranslation(0, -6, 0);
    imstkNew<CollidingObject> planeObj("Plane");
    planeObj->setVisualGeometry(planeGeom);
    planeObj->setCollidingGeometry(planeGeom);
    scene->addSceneObject(planeObj);

    return scene;
}

std::shared_ptr<Scene>
createClothScene(std::string sceneName)
{
    imstkNew<Scene> scene("PBDCloth");
    {
        std::shared_ptr<PbdObject> clothObj = makeClothObj("Cloth", 10.0, 10.0, 16, 16);
        scene->addSceneObject(clothObj);

        // Light (white)
        imstkNew<DirectionalLight> whiteLight("whiteLight");
        whiteLight->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
        whiteLight->setIntensity(1.0);
        scene->addLight(whiteLight);

        // Light (red)
        imstkNew<SpotLight> colorLight("colorLight");
        colorLight->setPosition(Vec3d(-5.0, -3.0, 5.0));
        colorLight->setFocalPoint(Vec3d(0.0, -5.0, 5.0));
        colorLight->setIntensity(100.);
        colorLight->setColor(Color::Red);
        colorLight->setSpotAngle(30.0);
        scene->addLight(colorLight);

        // Adjust camera
        scene->getActiveCamera()->setFocalPoint(0.0, -5.0, 5.0);
        scene->getActiveCamera()->setPosition(-15.0, -5.0, 25.0);
    }
    return scene;
}

void
testMultipleScenesInBackendMode()
{
    imstkNew<SceneManager> sceneManager("SceneManager");
    auto                   scene1 = createClothScene("clothScene");
    sceneManager->addScene(scene1);
    auto scene2 = createSoftBodyScene("deformableBodyScene");
    sceneManager->addScene(scene2);

    scene1->initialize();
    scene2->initialize();

    // set to scene 1
    sceneManager->setActiveScene(scene1);

    sceneManager->update();
    sceneManager->update();
    sceneManager->update();
    sceneManager->update();

    // set to scene 2
    sceneManager->setActiveScene(scene2);

    sceneManager->update();
    sceneManager->update();
    sceneManager->update();
    sceneManager->update();

    // set to scene 1
    sceneManager->setActiveScene(scene1);

    sceneManager->update();
    sceneManager->update();
    sceneManager->update();
    sceneManager->update();

    // set to scene 2
    sceneManager->setActiveScene(scene2);

    sceneManager->update();
    sceneManager->update();
    sceneManager->update();
    sceneManager->update();

    std::cout << "Press any key to exit..." << std::endl;

    getchar();
}

void
testMultipleScenesInRenderMode()
{
    // Simulation manager defaults to rendering mode
    std::shared_ptr<Scene> scene1 = createClothScene("clothScene");
    std::shared_ptr<Scene> scene2 = createSoftBodyScene("deformableBodyScene");

    scene1->initialize();
    scene2->initialize();

    // Setup a viewer to render in its own thread
    imstkNew<VTKViewer> viewer("Viewer");
    viewer->setActiveScene(scene2);

    imstkNew<SceneManager> sceneManager("Scene Manager");
    sceneManager->setActiveScene(scene2);

    std::shared_ptr<SimulationManager> driver = std::make_shared<SimulationManager>();
    driver->addModule(viewer);
    driver->addModule(sceneManager);

    // Create a call back on key press of 's' to switch scenes
    LOG(INFO) << "s/S followed by enter to switch scenes";
    LOG(INFO) << "q/Q followed by enter to quit";

    connect<KeyEvent>(viewer->getKeyboardDevice(), &KeyboardDeviceClient::keyPress,
        [&, driver](KeyEvent* e)
    {
        if (e->m_key == 's' || e->m_key == 'S')
        {
            if (sceneManager->getActiveScene() == scene1)
            {
                LOG(INFO) << "Switching to scene2";
                sceneManager->setActiveScene(scene2);
                viewer->setActiveScene(scene2);
            }
            else
            {
                LOG(INFO) << "Switching to scene1";
                sceneManager->setActiveScene(scene1);
                viewer->setActiveScene(scene1);
            }
        }
        else if (e->m_key == 'q' || e->m_key == 'Q')
        {
            driver->requestStatus(ModuleDriverStopped);
        }
    });

    driver->start();
}

void
testMultipleScenesInBackgroundMode()
{
    imstkNew<ConsoleModule> console;

    imstkNew<SceneManager> sceneManager("SceneManager");
    auto                   scene1 = createClothScene("clothScene");
    auto                   scene2 = createSoftBodyScene("deformableBodyScene");
    sceneManager->addScene(scene1);
    sceneManager->addScene(scene2);

    imstkNew<SimulationManager> driver;
    driver->addModule(console);
    driver->addModule(sceneManager);

    scene1->initialize();
    scene2->initialize();

    sceneManager->setActiveScene(scene1);

    LOG(INFO) << "s/S followed by enter to switch scenes";
    LOG(INFO) << "q/Q followed by enter to quit";
    auto keyPressFunc =
        [&](KeyEvent* e)
        {
            if (e->m_key == 's' || e->m_key == 'S')
            {
                if (sceneManager->getActiveScene() == scene1)
                {
                    LOG(INFO) << "Switching to scene2";
                    sceneManager->setActiveScene(scene2);
                }
                else
                {
                    LOG(INFO) << "Switching to scene1";
                    sceneManager->setActiveScene(scene1);
                }
            }
            else if (e->m_key == 'q' || e->m_key == 'Q')
            {
                LOG(INFO) << "Exiting background mode";
                driver->requestStatus(ModuleDriverStopped);
            }
        };
    connect<KeyEvent>(console->getKeyboardDevice(), &KeyboardDeviceClient::keyPress, keyPressFunc);

    driver->start();
}

///
/// \brief Test multiple scenes
/// \todo: Move to unit test
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    testMultipleScenesInBackendMode();
    testMultipleScenesInBackgroundMode();
    testMultipleScenesInRenderMode();

    return 0;
}
