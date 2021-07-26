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
#include "imstkCollisionGraph.h"
#include "imstkDirectionalLight.h"
#include "imstkImageData.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLineMesh.h"
#include "imstkLogger.h"
#include "imstkMeshIO.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectCollision.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSurfaceMesh.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

#ifdef iMSTK_USE_OPENHAPTICS
#include "imstkHapticDeviceManager.h"
#include "imstkHapticDeviceClient.h"
#else
#include "imstkKeyboardDeviceClient.h"
#endif

using namespace imstk;

///
/// \brief Creates a triangle grid on the xz plane
///
static std::shared_ptr<SurfaceMesh>
makeTriangleGrid(const double width,
                 const double height,
                 const int    nRows,
                 const int    nCols)
{
    imstkNew<VecDataArray<double, 3>> verticesPtr(nRows * nCols);
    VecDataArray<double, 3>&          vertices = *verticesPtr.get();
    const double                      dy       = width / (nCols - 1);
    const double                      dx       = height / (nRows - 1);
    for (int i = 0; i < nRows; ++i)
    {
        for (int j = 0; j < nCols; j++)
        {
            vertices[i * nCols + j] = Vec3d(dx * static_cast<double>(i), 0.0, dy * static_cast<double>(j)) - Vec3d(height, 0.0, width) * 0.5;
        }
    }

    // Add connectivity data
    imstkNew<VecDataArray<int, 3>> indicesPtr;
    VecDataArray<int, 3>&          indices = *indicesPtr;
    for (int i = 0; i < nRows - 1; i++)
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
                indices.push_back(Vec3i(index2, index1, index3));
                indices.push_back(Vec3i(index3, index4, index2));
            }
            else
            {
                indices.push_back(Vec3i(index4, index2, index1));
                indices.push_back(Vec3i(index3, index4, index1));
            }
        }
    }

    imstkNew<VecDataArray<float, 2>> uvCoordsPtr(nRows * nCols);
    VecDataArray<float, 2>&          uvCoords = *uvCoordsPtr.get();
    for (int i = 0; i < nRows; ++i)
    {
        for (int j = 0; j < nCols; j++)
        {
            uvCoords[i * nCols + j] = Vec2f(static_cast<float>(i) / nRows, static_cast<float>(j) / nCols) * 3.0;
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
    std::shared_ptr<SurfaceMesh> clothMesh = makeTriangleGrid(width, height, rowCount, colCount);

    // Setup the Parameters
    imstkNew<PBDModelConfig> pbdParams;
    pbdParams->enableConstraint(PbdConstraint::Type::Distance, 5000.0);
    pbdParams->enableConstraint(PbdConstraint::Type::Dihedral, 5000.0);
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
    pbdParams->m_uniformMassValue = width * height / (rowCount * colCount);
    pbdParams->m_gravity    = Vec3d(0.0, -20.0, 0.0); // Slightly larger gravity to compensate viscosity
    pbdParams->m_dt         = 0.005;
    pbdParams->m_iterations = 2;
    pbdParams->m_viscousDampingCoeff = 0.0;

    // Setup the Model
    imstkNew<PbdModel> pbdModel;
    pbdModel->setModelGeometry(clothMesh);
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

    imstkNew<VisualModel> visualModel(clothMesh);
    visualModel->setRenderMaterial(material);

    // Setup the Object
    imstkNew<PbdObject> pbdObject(name);
    pbdObject->addVisualModel(visualModel);
    pbdObject->setPhysicsGeometry(clothMesh);
    pbdObject->setCollidingGeometry(clothMesh);
    pbdObject->setDynamicalModel(pbdModel);

    return pbdObject;
}

///
/// \brief This example demonstrates the collision interaction
/// using Position based dynamics
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Setup the scene
    imstkNew<Scene> scene("PBDThinTissueContact");
    scene->getActiveCamera()->setPosition(0.12, 4.51, 16.51);
    scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);
    scene->getActiveCamera()->setViewUp(0.0, 0.96, -0.28);

    // Setup a tissue
    std::shared_ptr<PbdObject> tissueObj = makeTissueObj("Tissue", 10.0, 10.0, 16, 16);
    scene->addSceneObject(tissueObj);

    // Setup the tool to press the tissue
    imstkNew<LineMesh>                toolGeometry;
    imstkNew<VecDataArray<double, 3>> verticesPtr(2);
    (*verticesPtr)[0] = Vec3d(0.0, 0.0, 0.0);
    (*verticesPtr)[1] = Vec3d(0.0, 2.0, 0.0);
    imstkNew<VecDataArray<int, 2>> indicesPtr(1);
    (*indicesPtr)[0] = Vec2i(0, 1);
    toolGeometry->initialize(verticesPtr, indicesPtr);
#ifndef iMSTK_USE_OPENHAPTICS
    toolGeometry->translate(Vec3d(0.5, 2.0, 0.5));
#endif

    imstkNew<CollidingObject> toolObj("Tool");
    toolObj->setVisualGeometry(toolGeometry);
    toolObj->setCollidingGeometry(toolGeometry);
    toolObj->getVisualModel(0)->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);
    toolObj->getVisualModel(0)->getRenderMaterial()->setLineWidth(5.0);
    toolObj->getVisualModel(0)->getRenderMaterial()->setRecomputeVertexNormals(false);
    toolObj->getVisualModel(0)->getRenderMaterial()->setBackFaceCulling(false);
    scene->addSceneObject(toolObj);

    // Add a collision interaction between the tools
    auto interaction = std::make_shared<PbdObjectCollision>(tissueObj, toolObj.get(), "MeshToMeshBruteForceCD");
    scene->getCollisionGraph()->addInteraction(interaction);

    // Light
    imstkNew<DirectionalLight> light;
    light->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
    light->setIntensity(1.0);
    scene->addLight("Light", light);

    // Run the simulation
    {
        // Setup a viewer to render
        imstkNew<VTKViewer> viewer("Viewer");
        viewer->setVtkLoggerMode(VTKViewer::VTKLoggerMode::MUTE);
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene
        imstkNew<SceneManager> sceneManager("Scene Manager");
        sceneManager->setActiveScene(scene);
        sceneManager->setExecutionType(Module::ExecutionType::ADAPTIVE);
        sceneManager->pause(); // Start simulation paused

#ifdef iMSTK_USE_OPENHAPTICS
        imstkNew<HapticDeviceManager> hapticManager;
        hapticManager->setSleepDelay(1.0); // Delay for 1ms (haptics thread is limited to max 1000hz)
        std::shared_ptr<HapticDeviceClient> hapticDeviceClient = hapticManager->makeDeviceClient();
#endif

        imstkNew<SimulationManager> driver;
        driver->addModule(viewer);
        driver->addModule(sceneManager);
#ifdef iMSTK_USE_OPENHAPTICS
        driver->addModule(hapticManager);
#endif
        driver->setDesiredDt(0.005);

        Mat3d rotationalOffset = Mat3d::Identity();
        connect<Event>(sceneManager, SceneManager::preUpdate, [&](Event*)
        {
#ifdef iMSTK_USE_OPENHAPTICS
            hapticDeviceClient->update();
            const Quatd deviceOrientation = (Quatd(rotationalOffset) * hapticDeviceClient->getOrientation()).normalized();
            const Vec3d devicePosition    = (rotationalOffset * hapticDeviceClient->getPosition()) * 0.05 + Vec3d(0.0, 0.0, 0.0);
            toolGeometry->setRotation(deviceOrientation);
            toolGeometry->setTranslation(devicePosition);
            toolGeometry->postModified();
#else
            if (viewer->getKeyboardDevice()->getButton('i') == KEY_PRESS)
            {
                toolGeometry->translate(Vec3d(0.0, 0.01, 0.0));
                toolGeometry->postModified();
            }
            else if (viewer->getKeyboardDevice()->getButton('k') == KEY_PRESS)
            {
                toolGeometry->translate(Vec3d(0.0, -0.01, 0.0));
                toolGeometry->postModified();
            }
            else if (viewer->getKeyboardDevice()->getButton('j') == KEY_PRESS)
            {
                toolGeometry->translate(Vec3d(-0.01, 0.0, 0.0));
                toolGeometry->postModified();
            }
            else if (viewer->getKeyboardDevice()->getButton('l') == KEY_PRESS)
            {
                toolGeometry->translate(Vec3d(0.01, 0.0, 0.0));
                toolGeometry->postModified();
            }
            else if (viewer->getKeyboardDevice()->getButton('u') == KEY_PRESS)
            {
                toolGeometry->translate(Vec3d(0.0, 0.0, 0.01));
                toolGeometry->postModified();
            }
            else if (viewer->getKeyboardDevice()->getButton('o') == KEY_PRESS)
            {
                toolGeometry->translate(Vec3d(0.0, 0.0, -0.01));
                toolGeometry->postModified();
            }
#endif
            });

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

        driver->start();
    }

    return 0;
}