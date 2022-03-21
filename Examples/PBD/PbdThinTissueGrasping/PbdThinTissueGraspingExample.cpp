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
#include "imstkDirectionalLight.h"
#include "imstkHapticDeviceClient.h"
#include "imstkHapticDeviceManager.h"
#include "imstkImageData.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLaparoscopicToolController.h"
#include "imstkMeshIO.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectCollision.h"
#include "imstkPbdObjectGrasping.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSurfaceMesh.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

using namespace imstk;

///
/// \brief Creates tissue geometry
///
static std::shared_ptr<SurfaceMesh>
makeTriangleGrid(const double width,
                 const double height,
                 const int    nRows,
                 const int    nCols,
                 const double uvScale)
{
    imstkNew<VecDataArray<double, 3>> verticesPtr(nRows * nCols);
    VecDataArray<double, 3>&          vertices = *verticesPtr.get();
    const double                      dy       = width / static_cast<double>(nCols - 1);
    const double                      dx       = height / static_cast<double>(nRows - 1);
    for (int i = 0; i < nRows; i++)
    {
        for (int j = 0; j < nCols; j++)
        {
            vertices[i * nCols + j] = Vec3d(dx * static_cast<double>(i), 0.0, dy * static_cast<double>(j)) - Vec3d(height, 0.0, width) * 0.5;
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

    imstkNew<VecDataArray<float, 2>> uvCoordsPtr(nRows * nCols);
    VecDataArray<float, 2>&          uvCoords = *uvCoordsPtr.get();
    for (int i = 0; i < nRows; ++i)
    {
        for (int j = 0; j < nCols; j++)
        {
            uvCoords[i * nCols + j] = Vec2f(static_cast<float>(i) / nRows, static_cast<float>(j) / nCols) * uvScale;
        }
    }

    imstkNew<SurfaceMesh> mesh;
    mesh->initialize(verticesPtr, indicesPtr);
    mesh->setVertexTCoords("uvs", uvCoordsPtr);
    return mesh;
}

///
/// \brief Creates tissue object
///
static std::shared_ptr<PbdObject>
makeTissueObj(const std::string& name,
              const double       width,
              const double       height,
              const int          rowCount,
              const int          colCount)
{
    // Setup the Geometry
    std::shared_ptr<SurfaceMesh> mesh =
        makeTriangleGrid(width, height, rowCount, colCount, 2.0);

    // Setup the Parameters
    imstkNew<PbdModelConfig> pbdParams;
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 10000.0);
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Dihedral, 0.1);
    for (int x = 0; x < rowCount; x++)
    {
        for (int y = 0; y < colCount; y++)
        {
            if (x == 0 || y == 0 || x == rowCount - 1 || y == colCount - 1)
            {
                pbdParams->m_fixedNodeIds.push_back(x * colCount + y);
            }
        }
    }
    pbdParams->m_uniformMassValue = 1.0;
    pbdParams->m_gravity    = Vec3d(0.0, -0.01, 0.0);
    pbdParams->m_dt         = 0.005;
    pbdParams->m_iterations = 4;
    pbdParams->m_viscousDampingCoeff = 0.01;

    // Setup the Model
    imstkNew<PbdModel> pbdModel;
    pbdModel->setModelGeometry(mesh);
    pbdModel->configure(pbdParams);

    // Setup the VisualModel
    imstkNew<RenderMaterial> material;
    material->setBackFaceCulling(false);
    material->setDisplayMode(RenderMaterial::DisplayMode::Surface);
    material->setShadingModel(RenderMaterial::ShadingModel::PBR);
    auto diffuseTex = MeshIO::read<ImageData>(iMSTK_DATA_ROOT "/textures/fleshDiffuse.jpg");
    material->addTexture(std::make_shared<Texture>(diffuseTex, Texture::Type::Diffuse));
    auto normalTex = MeshIO::read<ImageData>(iMSTK_DATA_ROOT "/textures/fleshNormal.jpg");
    material->addTexture(std::make_shared<Texture>(normalTex, Texture::Type::Normal));
    auto ormTex = MeshIO::read<ImageData>(iMSTK_DATA_ROOT "/textures/fleshORM.jpg");
    material->addTexture(std::make_shared<Texture>(ormTex, Texture::Type::ORM));

    // Setup the Object
    imstkNew<PbdObject> tissueObj(name);
    tissueObj->setVisualGeometry(mesh);
    tissueObj->getVisualModel(0)->setRenderMaterial(material);
    tissueObj->setPhysicsGeometry(mesh);
    tissueObj->setCollidingGeometry(mesh);
    tissueObj->setDynamicalModel(pbdModel);

    return tissueObj;
}

///
/// \brief This example demonstrates Pbd grasping. PbdObjectGrasping allows
/// us to hold onto parts of a tissue or other pbd deformable with a tool
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Scene
    imstkNew<Scene> scene("PbdThinTissueGraspingExample");
    scene->getActiveCamera()->setPosition(0.001, 0.05, 0.15);
    scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);
    scene->getActiveCamera()->setViewUp(0.0, 0.96, -0.28);

    imstkNew<Capsule> geomShaft;
    geomShaft->setLength(1.0);
    geomShaft->setRadius(0.005);
    geomShaft->setOrientation(Quatd(Rotd(PI_2, Vec3d(1.0, 0.0, 0.0))));
    geomShaft->setTranslation(Vec3d(0.0, 0.0, 0.5));
    imstkNew<CollidingObject> objShaft("ShaftObject");
    objShaft->setVisualGeometry(MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/laptool/pivot.obj"));
    objShaft->setCollidingGeometry(geomShaft);
    scene->addSceneObject(objShaft);

    imstkNew<Capsule> geomUpperJaw;
    geomUpperJaw->setLength(0.05);
    geomUpperJaw->setTranslation(Vec3d(0.0, 0.0013, -0.016));
    geomUpperJaw->setRadius(0.004);
    geomUpperJaw->setOrientation(Quatd(Rotd(PI_2, Vec3d(1.0, 0.0, 0.0))));
    imstkNew<CollidingObject> objUpperJaw("UpperJawObject");
    objUpperJaw->setVisualGeometry(MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/laptool/upper.obj"));
    objUpperJaw->setCollidingGeometry(geomUpperJaw);
    scene->addSceneObject(objUpperJaw);

    imstkNew<Capsule> geomLowerJaw;
    geomLowerJaw->setLength(0.05);
    geomLowerJaw->setTranslation(Vec3d(0.0, -0.0013, -0.016));
    geomLowerJaw->setRadius(0.004);
    geomLowerJaw->setOrientation(Quatd(Rotd(PI_2, Vec3d(1.0, 0.0, 0.0))));
    imstkNew<CollidingObject> objLowerJaw("LowerJawObject");
    objLowerJaw->setVisualGeometry(MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/laptool/lower.obj"));
    objLowerJaw->setCollidingGeometry(geomLowerJaw);
    scene->addSceneObject(objLowerJaw);

    imstkNew<Capsule> pickGeom;
    pickGeom->setLength(0.05);
    pickGeom->setTranslation(Vec3d(0.0, 0.0, -0.016));
    pickGeom->setRadius(0.006);
    pickGeom->setOrientation(Quatd(Rotd(PI_2, Vec3d(1.0, 0.0, 0.0))));

    // 300mm x 300mm patch of tissue
    std::shared_ptr<PbdObject> tissueObj = makeTissueObj("Tissue", 0.1, 0.1, 16, 16);
    scene->addSceneObject(tissueObj);

    imstkNew<HapticDeviceManager> deviceManager;
    deviceManager->setSleepDelay(1.0);
    std::shared_ptr<HapticDeviceClient> client = deviceManager->makeDeviceClient();

    // Create and add virtual coupling object controller in the scene
    imstkNew<LaparoscopicToolController> controller(objShaft, objUpperJaw, objLowerJaw, pickGeom, client);
    controller->setJawAngleChange(1.0);
    controller->setTranslationScaling(0.001);
    scene->addController(controller);

    // Add collision for both jaws of the tool
    auto upperJawCollision = std::make_shared<PbdObjectCollision>(tissueObj, objUpperJaw, "SurfaceMeshToCapsuleCD");
    auto lowerJawCollision = std::make_shared<PbdObjectCollision>(tissueObj, objLowerJaw, "SurfaceMeshToCapsuleCD");
    scene->addInteraction(upperJawCollision);
    scene->addInteraction(lowerJawCollision);

    // Add picking interaction for both jaws of the tool
    auto jawPicking = std::make_shared<PbdObjectGrasping>(tissueObj);
    scene->addInteraction(jawPicking);

    // Light
    imstkNew<DirectionalLight> light;
    light->setFocalPoint(Vec3d(.0, -1.0, -1.0));
    light->setIntensity(1.0);
    scene->addLight("light", light);

    // Run the simulation
    {
        // Setup a viewer to render
        imstkNew<VTKViewer> viewer;
        viewer->setActiveScene(scene);
        viewer->setDebugAxesLength(0.01, 0.01, 0.01);

        // Setup a scene manager to advance the scene
        imstkNew<SceneManager> sceneManager;
        sceneManager->setActiveScene(scene);
        sceneManager->pause(); // Start simulation paused

        imstkNew<SimulationManager> driver;
        driver->addModule(deviceManager);
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

        connect<Event>(sceneManager, &SceneManager::postUpdate,
            [&](Event*)
            {
                // Simulate the cloth in real time
                tissueObj->getPbdModel()->getConfig()->m_dt = sceneManager->getDt();
            });

        connect<Event>(controller, &LaparoscopicToolController::JawClosed,
            [&](Event*)
            {
                LOG(INFO) << "Jaw Closed!";

                upperJawCollision->setEnabled(false);
                lowerJawCollision->setEnabled(false);
                jawPicking->beginCellGrasp(pickGeom, "SurfaceMeshToCapsuleCD");
            });
        connect<Event>(controller, &LaparoscopicToolController::JawOpened,
            [&](Event*)
            {
                LOG(INFO) << "Jaw Opened!";

                upperJawCollision->setEnabled(true);
                lowerJawCollision->setEnabled(true);
                jawPicking->endGrasp();
            });

        driver->start();
    }

    return 0;
}
