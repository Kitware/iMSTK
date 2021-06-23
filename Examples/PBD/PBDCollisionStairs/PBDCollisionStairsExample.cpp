///*=========================================================================
//
//   Library: iMSTK
//
//   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
//   & Imaging in Medicine, Rensselaer Polytechnic Institute.
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//	  http://www.apache.org/licenses/LICENSE-2.0.txt
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.
//
//=========================================================================*/

#include "imstkCamera.h"
#include "imstkCollisionGraph.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkDirectionalLight.h"
#include "imstkLogger.h"
#include "imstkMeshIO.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkObjectInteractionFactory.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkTetraTriangleMap.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

using namespace imstk;

///
/// \brief Creates a non-manifold top part of a staircase
//
static std::shared_ptr<SurfaceMesh>
buildStairs(int nSteps, double width, double height, double depth)
{
    // Build stair geometry
    const double halfWidth  = width * 0.5;
    const double halfHeight = height * 0.5;
    const double halfDepth  = depth * 0.5;
    const double dz = depth / static_cast<double>(nSteps);
    const double dy = height / static_cast<double>(nSteps);

    // Create vertices
    imstkNew<VecDataArray<double, 3>> verticesPtr;
    VecDataArray<double, 3>&          vertices = *verticesPtr.get();
    // 4 verts per step, 2 back, then 2 bottom
    vertices.reserve(nSteps * 4 + 4);
    for (size_t i = 0; i < nSteps; i++)
    {
        const double z  = static_cast<double>(dz * i) - halfDepth;
        const double y1 = static_cast<double>(dy * i) - halfHeight;
        vertices.push_back(Vec3d(-halfWidth, y1, z));
        vertices.push_back(Vec3d(halfWidth, y1, z));

        const double y2 = static_cast<double>(dy * (i + 1)) - halfHeight;
        vertices.push_back(Vec3d(-halfWidth, y2, z));
        vertices.push_back(Vec3d(halfWidth, y2, z));
    }
    {
        const double z    = static_cast<double>(dz * nSteps) - halfDepth;
        const double yTop = static_cast<double>(dy * nSteps) - halfHeight;
        vertices.push_back(Vec3d(-halfWidth, yTop, z));
        vertices.push_back(Vec3d(halfWidth, yTop, z));

        const double yBot = -halfHeight;
        vertices.push_back(Vec3d(-halfWidth, yBot, z));
        vertices.push_back(Vec3d(halfWidth, yBot, z));
    }

    // Create cells
    imstkNew<VecDataArray<int, 3>> trianglesPtr;
    VecDataArray<int, 3>&          triangles = *trianglesPtr.get();
    // Create sides and tops of steps
    for (int i = 0; i < nSteps; ++i)
    {
        // Stair front side
        triangles.push_back(Vec3i(i * 4 + 3, i * 4 + 1, i * 4));
        triangles.push_back(Vec3i(i * 4 + 2, i * 4 + 3, i * 4));
        // Stair top
        triangles.push_back(Vec3i((i + 1) * 4, i * 4 + 3, i * 4 + 2));
        triangles.push_back(Vec3i((i + 1) * 4, (i + 1) * 4 + 1, i * 4 + 3));
    }

    imstkNew<SurfaceMesh> stairMesh;
    stairMesh->initialize(verticesPtr, trianglesPtr);
    return stairMesh;
}

////
/// \brief Creates an Armadillo PbdObject
///
static std::shared_ptr<PbdObject>
makeArmadilloPbdObject(const std::string& name)
{
    imstkNew<PbdObject> pbdObj(name);

    // Read in the armadillo mesh
    auto tetMesh = MeshIO::read<TetrahedralMesh>(iMSTK_DATA_ROOT "armadillo/armadillo_volume.vtk");
    tetMesh->scale(Vec3d(0.07, 0.07, 0.07), Geometry::TransformType::ApplyToData);
    tetMesh->rotate(Vec3d(1.0, 0.0, 0.0), 1.3, Geometry::TransformType::ApplyToData);
    tetMesh->translate(Vec3d(0.0f, 10.0f, 0.0f), Geometry::TransformType::ApplyToData);
    std::shared_ptr<SurfaceMesh> surfMesh = tetMesh->extractSurfaceMesh();

    // Setup the Parameters
    imstkNew<PBDModelConfig> pbdParams;
    pbdParams->m_femParams->m_YoungModulus = 1000.0;
    pbdParams->m_femParams->m_PoissonRatio = 0.3;
    pbdParams->enableFEMConstraint(PbdConstraint::Type::FEMTet,
        PbdFEMConstraint::MaterialType::StVK);
    pbdParams->m_uniformMassValue = 1.0;
    pbdParams->m_gravity    = Vec3d(0, -10.0, 0);
    pbdParams->m_dt         = 0.01;
    pbdParams->m_iterations = 5;
    pbdParams->collisionParams->m_proximity = 0.3;
    pbdParams->collisionParams->m_stiffness = 0.1;

    // Setup the Model
    imstkNew<PbdModel> pbdModel;
    pbdModel->setModelGeometry(tetMesh);
    pbdModel->configure(pbdParams);

    // Setup the VisualModel
    imstkNew<RenderMaterial> material;
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    imstkNew<VisualModel> surfMeshModel(surfMesh);
    surfMeshModel->setRenderMaterial(material);

    // Setup the Object
    pbdObj->addVisualModel(surfMeshModel);
    pbdObj->setCollidingGeometry(surfMesh);
    pbdObj->setPhysicsGeometry(tetMesh);
    pbdObj->setPhysicsToVisualMap(std::make_shared<TetraTriangleMap>(tetMesh, surfMesh));
    pbdObj->setDynamicalModel(pbdModel);

    return pbdObj;
}

static std::shared_ptr<PbdObject>
makeStairsPbdObject(const std::string& name, int numSteps, double width, double height, double depth)
{
    imstkNew<PbdObject> stairObj(name);

    std::shared_ptr<SurfaceMesh> stairMesh = buildStairs(numSteps, width, height, depth);

    // Setup the parameters
    imstkNew<PBDModelConfig> pbdParams;
    pbdParams->m_uniformMassValue = 0.0;
    pbdParams->collisionParams->m_proximity = -0.1;
    pbdParams->m_iterations = 0;

    // Setup the model
    imstkNew<PbdModel> pbdModel;
    pbdModel->setModelGeometry(stairMesh);
    pbdModel->configure(pbdParams);

    // Setup the VisualModel
    imstkNew<RenderMaterial> stairMaterial;
    stairMaterial->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    imstkNew<VisualModel> visualModel(stairMesh);
    visualModel->setRenderMaterial(stairMaterial);

    stairObj->addVisualModel(visualModel);
    stairObj->setDynamicalModel(pbdModel);
    stairObj->setCollidingGeometry(stairMesh);
    stairObj->setPhysicsGeometry(stairMesh);

    return stairObj;
}

///
/// \brief This example demonstrates the collision interaction
/// using Position based dynamic on a more complex mesh
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    imstkNew<Scene> scene("PbdStairsCollision");
    {
        scene->getActiveCamera()->setPosition(0.0, 0.0, -30.0);
        scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);

        // Create and add the dragon to the scene
        auto pbdDragon1 = makeArmadilloPbdObject("Armadillo");
        scene->addSceneObject(pbdDragon1);

        auto stairObj = makeStairsPbdObject("PbdStairs", 12, 20.0, 10.0, 20.0);
        scene->addSceneObject(stairObj);

        // Collision
        scene->getCollisionGraph()->addInteraction(makeObjectInteractionPair(pbdDragon1, stairObj,
            InteractionType::PbdObjToPbdObjCollision, CollisionDetection::Type::MeshToMeshBruteForce));

        // Light
        imstkNew<DirectionalLight> light;
        light->setFocalPoint(Vec3d(5.0, -8.0, 5.0));
        light->setIntensity(1.0);
        scene->addLight("light", light);

        imstkNew<DirectionalLight> light2;
        light2->setFocalPoint(-Vec3d(5, -8, 5));
        light2->setIntensity(1.2);
        scene->addLight("light2", light2);
    }

    // Run the simulation
    {
        // Setup a viewer to render
        imstkNew<VTKViewer> viewer("Viewer");
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene
        imstkNew<SceneManager> sceneManager("Scene Manager");
        sceneManager->setActiveScene(scene);
        sceneManager->pause(); // Start simulation paused

        imstkNew<SimulationManager> driver;
        driver->addModule(viewer);
        driver->addModule(sceneManager);

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
