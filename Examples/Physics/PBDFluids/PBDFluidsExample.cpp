/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkCollider.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkMeshIO.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkPbdSystem.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectCollision.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSimulationUtils.h"
#include "imstkSurfaceMesh.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

using namespace imstk;

// mesh file name
const std::string tetMeshFileName = iMSTK_DATA_ROOT "/asianDragon/asianDragon.vtu";

///
/// \brief create a PbdObject for fluids
///
std::shared_ptr<PbdObject>
createPbdFluid(const std::string& tetMeshName)
{
    // Load a sample mesh
    std::shared_ptr<PointSet> tetMesh = MeshIO::read(tetMeshName);

    imstkNew<PointSet> fluidMesh;
    fluidMesh->initialize(tetMesh->getInitialVertexPositions());

    imstkNew<VisualModel> fluidVisualModel;
    fluidVisualModel->setGeometry(fluidMesh);

    imstkNew<RenderMaterial> material;
    material->setDisplayMode(RenderMaterial::DisplayMode::Fluid);
    material->setVertexColor(Color::Red);
    material->setPointSize(0.5); // Control visual particle size
    fluidVisualModel->setRenderMaterial(material);

    imstkNew<PbdObject> deformableObj("Dragon");
    deformableObj->addVisualModel(fluidVisualModel);
    deformableObj->addComponent<Collider>()->setGeometry(fluidMesh);
    deformableObj->setPhysicsGeometry(fluidMesh);

    imstkNew<PbdSystem> pbdSystem;

    // Configure model
    auto         pbdParams      = std::make_shared<PbdModelConfig>();
    const double particleRadius = 0.5;
    pbdParams->enableConstantDensityConstraint(1.0, particleRadius);
    pbdParams->m_gravity    = Vec3d(0.0, -9.8, 0.0);
    pbdParams->m_dt         = 0.005;
    pbdParams->m_iterations = 2;
    pbdSystem->configure(pbdParams);

    deformableObj->setDynamicalModel(pbdSystem);
    deformableObj->getPbdBody()->uniformMassValue = 1.0;

    return deformableObj;
}

///
/// \brief Create a box mesh to hold the fluid
///
std::shared_ptr<SurfaceMesh>
createCollidingSurfaceMesh()
{
    imstkNew<VecDataArray<double, 3>> verticesPtr;
    VecDataArray<double, 3>&          vertices = *verticesPtr.get();
    int                               nSides   = 5;
    double                            width    = 40.0;
    double                            height   = 40.0;
    int                               nRows    = 2;
    int                               nCols    = 2;
    vertices.resize(nRows * nCols * nSides);
    const double dy = width / static_cast<double>(nCols - 1);
    const double dx = height / static_cast<double>(nRows - 1);
    for (int i = 0; i < nRows; ++i)
    {
        for (int j = 0; j < nCols; j++)
        {
            const double y = static_cast<double>(dy * j);
            const double x = static_cast<double>(dx * i);
            vertices[i * nCols + j] = Vec3d(x - 20, -10.0, y - 20);
        }
    }

    // c. Add connectivity data
    std::shared_ptr<VecDataArray<int, 3>> trianglesPtr = std::make_shared<VecDataArray<int, 3>>();
    VecDataArray<int, 3>&                 triangles    = *trianglesPtr;
    for (int i = 0; i < nRows - 1; ++i)
    {
        for (int j = 0; j < nCols - 1; j++)
        {
            triangles.push_back(Vec3i(i * nCols + j, i * nCols + j + 1, (i + 1) * nCols + j));
            triangles.push_back(Vec3i((i + 1) * nCols + j + 1, (i + 1) * nCols + j, i * nCols + j + 1));
        }
    }

    int nPointPerSide = nRows * nCols;
    //sidewalls 1 and 2 of box
    width  = 10.0;
    height = 40.0;
    nRows  = 2;
    nCols  = 2;
    const double dz  = width / static_cast<double>(nCols - 1);
    const double dx1 = height / static_cast<double>(nRows - 1);
    for (int i = 0; i < nRows; ++i)
    {
        for (int j = 0; j < nCols; j++)
        {
            const double z = static_cast<double>(dz * j);
            const double x = static_cast<double>(dx1 * i);
            vertices[(nPointPerSide) + i * nCols + j]     = Vec3d(x - 20, z - 10.0, 20);
            vertices[(nPointPerSide * 2) + i * nCols + j] = Vec3d(x - 20, z - 10.0, -20);
        }
    }

    // c. Add connectivity data
    for (int i = 0; i < nRows - 1; ++i)
    {
        for (int j = 0; j < nCols - 1; j++)
        {
            triangles.push_back(Vec3i((nPointPerSide) + i * nCols + j, (nPointPerSide) + i * nCols + j + 1, (nPointPerSide) + (i + 1) * nCols + j));
            triangles.push_back(Vec3i((nPointPerSide) + (i + 1) * nCols + j + 1, (nPointPerSide) + (i + 1) * nCols + j, (nPointPerSide) + i * nCols + j + 1));

            triangles.push_back(Vec3i((nPointPerSide * 2) + i * nCols + j + 1, (nPointPerSide * 2) + i * nCols + j, (nPointPerSide * 2) + (i + 1) * nCols + j));
            triangles.push_back(Vec3i((nPointPerSide * 2) + (i + 1) * nCols + j, (nPointPerSide * 2) + (i + 1) * nCols + j + 1, (nPointPerSide * 2) + i * nCols + j + 1));
        }
    }

    //sidewalls 3 and 4 of box
    width  = 10.0;
    height = 40.0;
    nRows  = 2;
    nCols  = 2;
    const double dz1 = width / static_cast<double>(nCols - 1);
    const double dy1 = height / static_cast<double>(nRows - 1);
    for (int i = 0; i < nRows; ++i)
    {
        for (int j = 0; j < nCols; j++)
        {
            const double z = static_cast<double>(dz1 * j);
            const double y = static_cast<double>(dy1 * i);
            vertices[(nPointPerSide * 3) + i * nCols + j] = Vec3d(20, z - 10.0, y - 20);
            vertices[(nPointPerSide * 4) + i * nCols + j] = Vec3d(-20, z - 10.0, y - 20);
        }
    }

    // c. Add connectivity data
    for (int i = 0; i < nRows - 1; ++i)
    {
        for (int j = 0; j < nCols - 1; j++)
        {
            triangles.push_back(Vec3i((nPointPerSide * 3) + i * nCols + j + 1, (nPointPerSide * 3) + i * nCols + j, (nPointPerSide * 3) + (i + 1) * nCols + j));
            triangles.push_back(Vec3i((nPointPerSide * 3) + (i + 1) * nCols + j, (nPointPerSide * 3) + (i + 1) * nCols + j + 1, (nPointPerSide * 3) + i * nCols + j + 1));

            triangles.push_back(Vec3i((nPointPerSide * 4) + i * nCols + j, (nPointPerSide * 4) + i * nCols + j + 1, (nPointPerSide * 4) + (i + 1) * nCols + j));
            triangles.push_back(Vec3i((nPointPerSide * 4) + (i + 1) * nCols + j + 1, (nPointPerSide * 4) + (i + 1) * nCols + j, (nPointPerSide * 4) + i * nCols + j + 1));
        }
    }

    imstkNew<SurfaceMesh> floorMeshColliding;
    floorMeshColliding->initialize(verticesPtr, trianglesPtr);
    return floorMeshColliding;
}

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

        std::shared_ptr<PbdObject> fluidObj = createPbdFluid(tetMeshFileName);
        scene->addSceneObject(fluidObj);

        imstkNew<SceneObject>        floorObj("Floor");
        std::shared_ptr<SurfaceMesh> floorGeom = createCollidingSurfaceMesh();
        floorObj->addComponent<VisualModel>()->setGeometry(floorGeom);
        floorObj->addComponent<Collider>()->setGeometry(floorGeom);
        scene->addSceneObject(floorObj);

        // Collisions
        scene->addInteraction(std::make_shared<PbdObjectCollision>(fluidObj, floorObj));
    }

    // Run the simulation
    {
        // Setup a viewer to render
        imstkNew<VTKViewer> viewer;
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene
        imstkNew<SceneManager> sceneManager;
        sceneManager->setActiveScene(scene);
        sceneManager->pause(); // Start simulation paused

        imstkNew<SimulationManager> driver;
        driver->addModule(viewer);
        driver->addModule(sceneManager);

        // Add default mouse and keyboard controls to the viewer
        std::shared_ptr<Entity> mouseAndKeyControls =
            SimulationUtils::createDefaultSceneControl(driver);
        scene->addSceneObject(mouseAndKeyControls);

        driver->start();
    }

    return 0;
}