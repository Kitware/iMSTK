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
#include "imstkKeyboardSceneControl.h"
#include "imstkLight.h"
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
#include "imstkSurfaceMesh.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

using namespace imstk;

///
/// \brief create a PbdObject for fluids
///
std::shared_ptr<PbdObject> createAndAddPbdObject(std::shared_ptr<Scene> scene,
                                                 const std::string&     tetMeshName);

///
/// \brief Create a box mesh to hold the fluid
///
std::shared_ptr<SurfaceMesh> createCollidingSurfaceMesh();

// mesh file name
const std::string tetMeshFileName = iMSTK_DATA_ROOT "/asianDragon/asianDragon.veg";

///
/// \brief This example demonstrates the fluids simulation
/// using Position based dynamics
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    imstkNew<Scene> scene("PBDFluid");
    {
        scene->getActiveCamera()->setPosition(0.0, 15.0, 20.0);

        std::shared_ptr<PbdObject>   deformableObj      = createAndAddPbdObject(scene, tetMeshFileName);
        std::shared_ptr<SurfaceMesh> floorMeshColliding = createCollidingSurfaceMesh();

        imstkNew<PbdObject> floorObj("Floor");
        floorObj->setCollidingGeometry(floorMeshColliding);
        floorObj->setVisualGeometry(floorMeshColliding);
        floorObj->setPhysicsGeometry(floorMeshColliding);

        imstkNew<PbdModel> pbdModel;
        pbdModel->setModelGeometry(floorMeshColliding);

        // Configure model
        imstkNew<PBDModelConfig> pbdParams;
        pbdParams->m_uniformMassValue = 0.0;
        pbdParams->collisionParams->m_proximity = 0.1;

        pbdModel->configure(pbdParams);
        floorObj->setDynamicalModel(pbdModel);

        scene->addSceneObject(floorObj);

        // Collisions
        scene->getCollisionGraph()->addInteraction(makeObjectInteractionPair(deformableObj, floorObj,
            InteractionType::PbdObjToPbdObjCollision,
            CollisionDetection::Type::MeshToMeshBruteForce));

        // Light (white)
        imstkNew<DirectionalLight> whiteLight("whiteLight");
        whiteLight->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
        whiteLight->setIntensity(7);
        scene->addLight(whiteLight);
    }

    // Run the simulation
    {
        // Setup a viewer to render in its own thread
        imstkNew<VTKViewer> viewer("Viewer");
        viewer->setActiveScene(scene);

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
createAndAddPbdObject(std::shared_ptr<Scene> scene,
                      const std::string&     tetMeshName)
{
    // Load a sample mesh
    std::shared_ptr<PointSet> tetMesh = MeshIO::read(tetMeshName);

    imstkNew<PointSet> fluidMesh;
    fluidMesh->initialize(tetMesh->getInitialVertexPositions());

    imstkNew<VisualModel> fluidVisualModel(fluidMesh.get());

    imstkNew<RenderMaterial> material;
    material->setDisplayMode(RenderMaterial::DisplayMode::Fluid);
    material->setVertexColor(Color::Red);
    material->setPointSize(0.5);
    fluidVisualModel->setRenderMaterial(material);

    imstkNew<PbdObject> deformableObj("Dragon");
    deformableObj->addVisualModel(fluidVisualModel);
    deformableObj->setCollidingGeometry(fluidMesh);
    deformableObj->setPhysicsGeometry(fluidMesh);

    imstkNew<PbdModel> pbdModel;
    pbdModel->setModelGeometry(fluidMesh);

    // Configure model
    imstkNew<PBDModelConfig> pbdParams;

    // Constant density constraint with stiffness
    pbdParams->enableConstraint(PbdConstraint::Type::ConstantDensity, 1.0);

    // Other parameters
    pbdParams->m_uniformMassValue = 1.0;
    pbdParams->m_gravity    = Vec3d(0.0, -9.8, 0.0);
    pbdParams->m_defaultDt  = 0.005;
    pbdParams->m_iterations = 2;
    pbdParams->collisionParams->m_proximity = 0.01;

    // Set the parameters
    pbdModel->configure(pbdParams);
    pbdModel->setTimeStepSizeType(TimeSteppingType::Fixed);
    deformableObj->setDynamicalModel(pbdModel);

    scene->addSceneObject(deformableObj);

    return deformableObj;
}

std::shared_ptr<SurfaceMesh>
createCollidingSurfaceMesh()
{
    StdVectorOfVec3d vertList;
    int              nSides = 5;
    double           width  = 40.0;
    double           height = 40.0;
    int              nRows  = 2;
    int              nCols  = 2;
    vertList.resize(nRows * nCols * nSides);
    const double dy = width / (double)(nCols - 1);
    const double dx = height / (double)(nRows - 1);
    for (int i = 0; i < nRows; ++i)
    {
        for (int j = 0; j < nCols; j++)
        {
            const double y = (double)dy * j;
            const double x = (double)dx * i;
            vertList[i * nCols + j] = Vec3d(x - 20, -10.0, y - 20);
        }
    }

    // c. Add connectivity data
    std::vector<SurfaceMesh::TriangleArray> triangles;
    for (std::size_t i = 0; i < nRows - 1; ++i)
    {
        for (std::size_t j = 0; j < nCols - 1; j++)
        {
            SurfaceMesh::TriangleArray tri[2];
            tri[0] = { { i* nCols + j, i* nCols + j + 1, (i + 1) * nCols + j } };
            tri[1] = { { (i + 1) * nCols + j + 1, (i + 1) * nCols + j, i* nCols + j + 1 } };
            triangles.push_back(tri[0]);
            triangles.push_back(tri[1]);
        }
    }

    int nPointPerSide = nRows * nCols;
    //sidewalls 1 and 2 of box
    width  = 10.0;
    height = 40.0;
    nRows  = 2;
    nCols  = 2;
    const double dz  = width / (double)(nCols - 1);
    const double dx1 = height / (double)(nRows - 1);
    for (int i = 0; i < nRows; ++i)
    {
        for (int j = 0; j < nCols; j++)
        {
            const double z = (double)dz * j;
            const double x = (double)dx1 * i;
            vertList[(nPointPerSide) + i * nCols + j]     = Vec3d(x - 20, z - 10.0, 20);
            vertList[(nPointPerSide * 2) + i * nCols + j] = Vec3d(x - 20, z - 10.0, -20);
        }
    }

    // c. Add connectivity data
    for (std::size_t i = 0; i < nRows - 1; ++i)
    {
        for (std::size_t j = 0; j < nCols - 1; j++)
        {
            SurfaceMesh::TriangleArray tri[2];
            tri[0] = { { (nPointPerSide) + i * nCols + j, (nPointPerSide) + i * nCols + j + 1, (nPointPerSide) + (i + 1) * nCols + j } };
            tri[1] = { { (nPointPerSide) + (i + 1) * nCols + j + 1, (nPointPerSide) + (i + 1) * nCols + j, (nPointPerSide) + i * nCols + j + 1 } };
            triangles.push_back(tri[0]);
            triangles.push_back(tri[1]);
            tri[0] = { { (nPointPerSide * 2) + i * nCols + j, (nPointPerSide * 2) + i * nCols + j + 1, (nPointPerSide * 2) + (i + 1) * nCols + j } };
            tri[1] = { { (nPointPerSide * 2) + (i + 1) * nCols + j + 1, (nPointPerSide * 2) + (i + 1) * nCols + j, (nPointPerSide * 2) + i * nCols + j + 1 } };
            triangles.push_back(tri[0]);
            triangles.push_back(tri[1]);
        }
    }

    //sidewalls 3 and 4 of box
    width  = 10.0;
    height = 40.0;
    nRows  = 2;
    nCols  = 2;
    const double dz1 = width / (double)(nCols - 1);
    const double dy1 = height / (double)(nRows - 1);
    for (int i = 0; i < nRows; ++i)
    {
        for (int j = 0; j < nCols; j++)
        {
            const double z = (double)dz1 * j;
            const double y = (double)dy1 * i;
            vertList[(nPointPerSide * 3) + i * nCols + j] = Vec3d(20, z - 10.0, y - 20);
            vertList[(nPointPerSide * 4) + i * nCols + j] = Vec3d(-20, z - 10.0, y - 20);
        }
    }

    // c. Add connectivity data
    for (std::size_t i = 0; i < nRows - 1; ++i)
    {
        for (std::size_t j = 0; j < nCols - 1; j++)
        {
            SurfaceMesh::TriangleArray tri[2];
            tri[0] = { { (nPointPerSide * 3) + i * nCols + j, (nPointPerSide * 3) + i * nCols + j + 1, (nPointPerSide * 3) + (i + 1) * nCols + j } };
            tri[1] = { { (nPointPerSide * 3) + (i + 1) * nCols + j + 1, (nPointPerSide * 3) + (i + 1) * nCols + j, (nPointPerSide * 3) + i * nCols + j + 1 } };
            triangles.push_back(tri[0]);
            triangles.push_back(tri[1]);
            tri[0] = { { (nPointPerSide * 4) + i * nCols + j, (nPointPerSide * 4) + i * nCols + j + 1, (nPointPerSide * 4) + (i + 1) * nCols + j } };
            tri[1] = { { (nPointPerSide * 4) + (i + 1) * nCols + j + 1, (nPointPerSide * 4) + (i + 1) * nCols + j, (nPointPerSide * 4) + i * nCols + j + 1 } };
            triangles.push_back(tri[0]);
            triangles.push_back(tri[1]);
        }
    }

    imstkNew<SurfaceMesh> floorMeshColliding;
    floorMeshColliding->initialize(vertList, triangles);
    return floorMeshColliding;
}
