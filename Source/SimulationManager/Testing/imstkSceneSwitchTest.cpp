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
#include "imstkMeshIO.h"
#include "imstkOneToOneMap.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

#include <gtest/gtest.h>

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
    auto clothMesh = std::make_shared<SurfaceMesh>();

    auto                     verticesPtr = std::make_shared<VecDataArray<double, 3>>(nRows * nCols);
    VecDataArray<double, 3>& vertices    = *verticesPtr.get();
    const double             dy = width / static_cast<double>(nCols - 1);
    const double             dx = height / static_cast<double>(nRows - 1);
    for (int i = 0; i < nRows; ++i)
    {
        for (int j = 0; j < nCols; j++)
        {
            vertices[i * nCols + j] = Vec3d(dx * static_cast<double>(i), 1.0, dy * static_cast<double>(j));
        }
    }

    // Add connectivity data
    auto                  indicesPtr = std::make_shared<VecDataArray<int, 3>>();
    VecDataArray<int, 3>& indices    = *indicesPtr.get();
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

std::shared_ptr<Scene>
createSoftBodyScene(std::string sceneName)
{
    auto scene = std::make_shared<Scene>(sceneName);
    scene->getActiveCamera()->setPosition(0.0, 2.0, 15.0);

    // Load a sample mesh
    auto tetMesh = MeshIO::read<TetrahedralMesh>(iMSTK_DATA_ROOT "/asianDragon/asianDragon.veg");

    // Extract the surface mesh
    std::shared_ptr<SurfaceMesh> surfMesh = tetMesh->extractSurfaceMesh();

    auto pbdObj   = std::make_shared<PbdObject>("PbdObj");
    auto pbdModel = std::make_shared<PbdModel>();
    pbdModel->setModelGeometry(tetMesh);

    // Configure model
    auto pbdConfig = std::make_shared<PbdModelConfig>();
    pbdConfig->m_femParams->m_YoungModulus = 1000000.0;
    pbdConfig->m_femParams->m_PoissonRatio = 0.3;
    pbdConfig->m_fixedNodeIds = { 51, 127, 178 };
    pbdConfig->enableFEMConstraint(PbdFEMConstraint::MaterialType::StVK);
    pbdConfig->m_uniformMassValue = 1.0;
    pbdConfig->m_gravity    = Vec3d(0.0, -9.8, 0.0);
    pbdConfig->m_iterations = 5;
    pbdConfig->m_dt = 0.03;
    pbdModel->configure(pbdConfig);

    // Set the geometries
    pbdObj->setDynamicalModel(pbdModel);
    pbdObj->setVisualGeometry(surfMesh);
    pbdObj->getVisualModel(0)->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    pbdObj->setPhysicsGeometry(tetMesh);
    pbdObj->setPhysicsToVisualMap(std::make_shared<OneToOneMap>(tetMesh, surfMesh));
    pbdObj->setDynamicalModel(pbdModel);

    scene->addSceneObject(pbdObj);

    return scene;
}

std::shared_ptr<Scene>
createClothScene(std::string sceneName)
{
    auto clothObj = std::make_shared<PbdObject>("Cloth");
    {
        // Setup the Geometry
        const double                 width     = 10.0;
        const double                 height    = 10.0;
        const int                    dim       = 16;
        std::shared_ptr<SurfaceMesh> clothMesh = makeClothGeometry(width, height, dim, dim);

        // Setup the Parameters
        auto pbdConfig = std::make_shared<PbdModelConfig>();
        pbdConfig->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 1.0e2);
        pbdConfig->enableConstraint(PbdModelConfig::ConstraintGenType::Dihedral, 1.0e1);
        pbdConfig->m_fixedNodeIds     = { 0, static_cast<size_t>(16) - 1 };
        pbdConfig->m_uniformMassValue = width * height / static_cast<double>(dim * dim);
        pbdConfig->m_gravity    = Vec3d(0.0, -9.8, 0.0);
        pbdConfig->m_dt         = 0.03;
        pbdConfig->m_iterations = 5;

        // Setup the Model
        auto pbdModel = std::make_shared<PbdModel>();
        pbdModel->setModelGeometry(clothMesh);
        pbdModel->configure(pbdConfig);

        // Setup the Object
        clothObj->setVisualGeometry(clothMesh);
        clothObj->getVisualModel(0)->getRenderMaterial()->setBackFaceCulling(false);
        clothObj->getVisualModel(0)->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
        clothObj->setPhysicsGeometry(clothMesh);
        clothObj->setDynamicalModel(pbdModel);
    }

    auto scene = std::make_shared<Scene>("PBDCloth");
    scene->addSceneObject(clothObj);
    scene->getActiveCamera()->setFocalPoint(0.0, -5.0, 5.0);
    scene->getActiveCamera()->setPosition(-15.0, -5.0, 25.0);
    return scene;
}

///
/// \brief Test we are able to switch scenes while rendering
///
TEST(imstkSimulationManagerTest, TestSceneSwitchWithRendering)
{
    // Simulation manager defaults to rendering mode
    std::shared_ptr<Scene> scene1 = createClothScene("clothScene");
    std::shared_ptr<Scene> scene2 = createSoftBodyScene("deformableBodyScene");

    scene1->initialize();
    scene2->initialize();

    // Setup a viewer to render in its own thread
    auto viewer = std::make_shared<VTKViewer>("Viewer");
    viewer->setActiveScene(scene1);

    auto sceneManager = std::make_shared<SceneManager>();
    sceneManager->setActiveScene(scene1);

    auto driver = std::make_shared<SimulationManager>();
    driver->addModule(viewer);
    driver->addModule(sceneManager);
    driver->setDesiredDt(0.03);

    // After 4s switch scenes, after 8s switch back, after 12s stop
    double elapsedTime = 0.0;
    connect<Event>(sceneManager, &SceneManager::postUpdate, [&](Event*)
        {
            elapsedTime += sceneManager->getDt();
            if (elapsedTime > 12.0)
            {
                driver->requestStatus(ModuleDriverStopped);
            }
            else if (elapsedTime > 8.0)
            {
                if (sceneManager->getActiveScene() != scene1)
                {
                    LOG(INFO) << "Switching to scene1";
                    sceneManager->setActiveScene(scene1);
                    viewer->setActiveScene(scene1);
                }
            }
            else if (elapsedTime > 4.0)
            {
                if (sceneManager->getActiveScene() != scene2)
                {
                    LOG(INFO) << "Switching to scene2";
                    sceneManager->setActiveScene(scene2);
                    viewer->setActiveScene(scene2);
                }
            }
        });

    driver->start();
}

///
/// \brief Test we are able to switch scenes without rendering
///
TEST(imstkSimulationManagerTest, TestSceneSwitchWithoutRendering)
{
    // Simulation manager defaults to rendering mode
    std::shared_ptr<Scene> scene1 = createClothScene("clothScene");
    std::shared_ptr<Scene> scene2 = createSoftBodyScene("deformableBodyScene");

    scene1->initialize();
    scene2->initialize();

    auto sceneManager = std::make_shared<SceneManager>();
    sceneManager->setActiveScene(scene1);

    auto driver = std::make_shared<SimulationManager>();
    driver->addModule(sceneManager);
    driver->setDesiredDt(0.03);

    // After 4s switch scenes, after 8s switch back, after 12s stop
    double elapsedTime = 0.0;
    connect<Event>(sceneManager, &SceneManager::postUpdate, [&](Event*)
        {
            elapsedTime += sceneManager->getDt();
            if (elapsedTime > 6.0)
            {
                driver->requestStatus(ModuleDriverStopped);
            }
            else if (elapsedTime > 4.0)
            {
                if (sceneManager->getActiveScene() != scene1)
                {
                    LOG(INFO) << "Switching to scene1";
                    sceneManager->setActiveScene(scene1);
                }
            }
            else if (elapsedTime > 2.0)
            {
                if (sceneManager->getActiveScene() != scene2)
                {
                    LOG(INFO) << "Switching to scene2";
                    sceneManager->setActiveScene(scene2);
                }
            }
        });

    driver->start();
}