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
#include "imstkCapsule.h"
#include "imstkCollisionGraph.h"
#include "imstkDirectionalLight.h"
#include "imstkHapticDeviceClient.h"
#include "imstkHapticDeviceManager.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLaparoscopicToolController.h"
#include "imstkLogger.h"
#include "imstkMeshIO.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectCollision.h"
#include "imstkPbdObjectPicking.h"
#include "imstkPBDPickingCH.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSurfaceMesh.h"
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
    pbdParams->enableConstraint(PbdConstraint::Type::Distance, 4000.0);
    pbdParams->enableConstraint(PbdConstraint::Type::Dihedral, 100.0);
    pbdParams->m_fixedNodeIds     = { 0, static_cast<size_t>(nCols) - 1 };
    pbdParams->m_uniformMassValue = width * height / ((double)nRows * (double)nCols);
    pbdParams->m_gravity    = Vec3d(0.0, -140.0, 0.0);
    pbdParams->m_dt         = 0.01;
    pbdParams->m_iterations = 5;
    pbdParams->m_viscousDampingCoeff = 0.01;

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
/// \brief This example demonstrates the concept of PBD picking
/// for haptic interaction. NOTE: Requires GeoMagic Touch device
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Scene
    imstkNew<Scene> scene("PBDPicking");

    // Create the virtual coupling object controller

    // Device Server
    imstkNew<HapticDeviceManager> server;
    server->setSleepDelay(1.0);
    std::shared_ptr<HapticDeviceClient> client = server->makeDeviceClient();

    // Load the meshes
    auto upperSurfMesh = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/laptool/upper.obj");
    auto lowerSurfMesh = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/laptool/lower.obj");
    auto pivotSurfMesh = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/laptool/pivot.obj");

    imstkNew<Capsule> geomShaft;
    geomShaft->setLength(20.0);
    geomShaft->setRadius(1.0);
    geomShaft->setOrientation(Quatd(Rotd(PI_2, Vec3d(1.0, 0.0, 0.0))));
    geomShaft->setTranslation(Vec3d(0.0, 0.0, 10.0));
    imstkNew<CollidingObject> objShaft("ShaftObject");
    objShaft->setVisualGeometry(pivotSurfMesh);
    objShaft->setCollidingGeometry(geomShaft);
    scene->addSceneObject(objShaft);

    imstkNew<Capsule> geomUpperJaw;
    geomUpperJaw->setLength(25.0);
    geomUpperJaw->setTranslation(Vec3d(0.0, 1.0, -12.5));
    geomUpperJaw->setRadius(2.0);
    geomUpperJaw->setOrientation(Quatd(Rotd(PI_2, Vec3d(1.0, 0.0, 0.0))));
    imstkNew<CollidingObject> objUpperJaw("UpperJawObject");
    objUpperJaw->setVisualGeometry(upperSurfMesh);
    objUpperJaw->setCollidingGeometry(geomUpperJaw);
    scene->addSceneObject(objUpperJaw);

    imstkNew<Capsule> geomLowerJaw;
    geomLowerJaw->setLength(25.0);
    geomLowerJaw->setTranslation(Vec3d(0.0, -1.0, -12.5));
    geomLowerJaw->setRadius(2.0);
    geomLowerJaw->setOrientation(Quatd(Rotd(PI_2, Vec3d(1.0, 0.0, 0.0))));
    imstkNew<CollidingObject> objLowerJaw("LowerJawObject");
    objLowerJaw->setVisualGeometry(lowerSurfMesh);
    objLowerJaw->setCollidingGeometry(geomLowerJaw);
    scene->addSceneObject(objLowerJaw);

    std::shared_ptr<PbdObject> clothObj = makeClothObj("Cloth", 50.0, 50.0, 15, 15);
    scene->addSceneObject(clothObj);

    // Create and add virtual coupling object controller in the scene
    imstkNew<LaparoscopicToolController> controller(objShaft, objUpperJaw, objLowerJaw, client);
    controller->setJawAngleChange(6.0e-3);
    scene->addController(controller);

    // Add collision for both jaws of the tool
    auto upperJawCollision = std::make_shared<PbdObjectCollision>(clothObj, objUpperJaw, "SurfaceMeshToCapsuleCD");
    auto lowerJawCollision = std::make_shared<PbdObjectCollision>(clothObj, objLowerJaw, "SurfaceMeshToCapsuleCD");
    scene->getCollisionGraph()->addInteraction(upperJawCollision);
    scene->getCollisionGraph()->addInteraction(lowerJawCollision);

    // Add picking interaction for both jaws of the tool
    auto upperJawPicking = std::make_shared<PbdObjectPicking>(clothObj, objUpperJaw, "PointSetToCapsuleCD");
    auto lowerJawPicking = std::make_shared<PbdObjectPicking>(clothObj, objLowerJaw, "PointSetToCapsuleCD");
    scene->getCollisionGraph()->addInteraction(upperJawPicking);
    scene->getCollisionGraph()->addInteraction(lowerJawPicking);

    // Camera
    scene->getActiveCamera()->setPosition(Vec3d(1.0, 1.0, 1.0) * 100.0);
    scene->getActiveCamera()->setFocalPoint(Vec3d(0.0, -50.0, 0.0));

    // Light
    imstkNew<DirectionalLight> light;
    light->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
    light->setIntensity(1.0);
    scene->addLight("light", light);

    // Run the simulation
    {
        // Setup a viewer to render
        imstkNew<VTKViewer> viewer("Viewer");
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene
        imstkNew<SceneManager> sceneManager("Scene Manager");
        sceneManager->setActiveScene(scene);
        sceneManager->setExecutionType(Module::ExecutionType::ADAPTIVE);
        sceneManager->pause(); // Start simulation paused

        imstkNew<SimulationManager> driver;
        driver->addModule(server);
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.005);

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

        connect<Event>(sceneManager, &SceneManager::postUpdate, [&](Event*)
        {
            // Simulate the cloth in real time
            clothObj->getPbdModel()->getParameters()->m_dt = sceneManager->getDt();
        });

        connect<Event>(controller, &LaparoscopicToolController::JawClosed,
            [&](Event*)
        {
            LOG(INFO) << "Jaw Closed!";
            auto chUpper = std::dynamic_pointer_cast<PBDPickingCH>(upperJawPicking->getCollisionHandlingA());
            auto chLower = std::dynamic_pointer_cast<PBDPickingCH>(lowerJawPicking->getCollisionHandlingA());

            chUpper->beginPick();
            chLower->beginPick();
            });
        connect<Event>(controller, &LaparoscopicToolController::JawOpened,
            [&](Event*)
        {
            LOG(INFO) << "Jaw Opened!";
            auto chUpper = std::dynamic_pointer_cast<PBDPickingCH>(upperJawPicking->getCollisionHandlingA());
            auto chLower = std::dynamic_pointer_cast<PBDPickingCH>(lowerJawPicking->getCollisionHandlingA());

            chUpper->endPick();
            chLower->endPick();
            });

        driver->start();
    }

    return 0;
}
