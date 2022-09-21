/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkDirectionalLight.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLooseOctree.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSceneObject.h"
#include "imstkSimulationManager.h"
#include "imstkSimulationUtils.h"
#include "imstkSurfaceMesh.h"
#include "imstkTextVisualModel.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"
#include "OctreeDebugModel.h"

using namespace imstk;

//#define NUM_MESHES 4u
#define NUM_MESHES 10u

// Load bunny mesh data (vertex positions and triangle faces)
std::pair<std::shared_ptr<VecDataArray<double, 3>>, std::shared_ptr<VecDataArray<int, 3>>> getBunny();
static std::pair<std::shared_ptr<VecDataArray<double, 3>>, std::shared_ptr<VecDataArray<int, 3>>> g_BunnyData = getBunny();

///
/// \brief Read a mesh, create a visual scene object and add to the scene
///
std::shared_ptr<SceneObject>
createMeshObject(const std::string& objectName,
                 const Color&       color)
{
    // Create a surface mesh for the bunny
    imstkNew<SurfaceMesh>                    surfMesh;
    std::shared_ptr<VecDataArray<double, 3>> verticesPtr = std::make_shared<VecDataArray<double, 3>>();
    *verticesPtr = *g_BunnyData.first;
    std::shared_ptr<VecDataArray<int, 3>> indicesPtr = std::make_shared<VecDataArray<int, 3>>();
    *indicesPtr = *g_BunnyData.second;
    surfMesh->initialize(verticesPtr, indicesPtr);

    // Create a visual model
    imstkNew<VisualModel> visualModel;
    visualModel->setGeometry(surfMesh);
    imstkNew<RenderMaterial> material;
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    material->setColor(color); // Wireframe color
    material->setLineWidth(1.0);
    visualModel->setRenderMaterial(material);

    imstkNew<SceneObject> visualObject(objectName);
    visualObject->addVisualModel(visualModel);

    return visualObject;
}

///
/// \brief Generate a random color
///
Color
getRandomColor()
{
    Color color(0, 0, 0, 1);
    while (true)
    {
        for (unsigned int i = 0; i < 3; ++i)
        {
            color.rgba[i] = static_cast<double>(rand()) / static_cast<double>(RAND_MAX);
        }
        if (color.rgba[0] > 0.95
            || color.rgba[1] > 0.95
            || color.rgba[2] > 0.95)
        {
            break;
        }
    }

    return color;
}

///
/// \brief This example demonstrates the usage of octree
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    imstkNew<Scene> scene("Octree Example");

    // Setup a viewer to render in its own thread
    imstkNew<VTKViewer> viewer;
    viewer->setActiveScene(scene);
    viewer->setWindowTitle("Octree Example");
    viewer->setSize(1920, 1080);

    auto statusText = std::make_shared<TextVisualModel>("StatusText");
    statusText->setFontSize(30.0);
    statusText->setTextColor(Color::Orange);
    statusText->setPosition(TextVisualModel::DisplayPosition::UpperLeft);

    // Seed based on CPU time for random colors
    srand(static_cast<unsigned int>(time(nullptr)));

    // Create meshes
    std::vector<std::shared_ptr<SurfaceMesh>> triMeshes;
    for (unsigned int i = 0; i < NUM_MESHES; ++i)
    {
        std::shared_ptr<SceneObject> sceneObj = createMeshObject("Mesh-" + std::to_string(triMeshes.size()), getRandomColor());
        scene->addSceneObject(sceneObj);
        triMeshes.push_back(std::dynamic_pointer_cast<SurfaceMesh>(sceneObj->getVisualGeometry()));
    }

    // Compute the scale factor to scale meshes such that meshes with different sizes are still visualized consistently
    Vec3d      lowerCorner, upperCorner;
    const auto pointset = std::dynamic_pointer_cast<PointSet>(triMeshes.front());
    ParallelUtils::findAABB(*pointset->getVertexPositions(), lowerCorner, upperCorner);
    const auto scaleFactor = 20.0 / (upperCorner - lowerCorner).norm();
    for (const auto& mesh : triMeshes)
    {
        mesh->scale(scaleFactor, Geometry::TransformType::ApplyToData);
    }

    StopWatch timer;
    timer.start();

    // Create octree
    imstkNew<LooseOctree> octree(Vec3d(0.0, 0.0, 0.0), 100.0, 0.125, 2.0, "TestOctree");

    // Add all meshes to the octree
    for (const auto& mesh : triMeshes)
    {
        octree->addTriangleMesh(mesh);
    }

    // Build octree after adding all geometries
    octree->build();
    LOG(INFO) << "Build octree time: " << timer.getTimeElapsed() << " ms";

    // Always rebuild tree from scratch in each update (default update is incremental update)
    // This is significantly slower than incremental update!
    // octree.setAlwaysRebuild(true);

    auto debugOctreeObj   = std::make_shared<Entity>();
    auto debugOctreeModel = debugOctreeObj->addComponent<OctreeDebugModel>();
    debugOctreeModel->setInputOctree(octree);
    debugOctreeModel->setLineWidth(1.0);
    debugOctreeModel->setLineColor(Color::Green);
    scene->addSceneObject(debugOctreeObj);

    // Data for animation
    const double            translation = 15.0;
    VecDataArray<double, 3> centers;
    VecDataArray<double, 3> dirs;
    for (unsigned int i = 0; i < NUM_MESHES; ++i)
    {
        centers.push_back(Vec3d(translation, 0, 0));
        dirs.push_back(Vec3d(-1, 0, 0));
    }

    // Transform the mesh objects
    const double angle = 2.0 * PI / NUM_MESHES;
    for (unsigned int i = 0; i < NUM_MESHES; ++i)
    {
        const auto rotation = angle * static_cast<double>(i);
        triMeshes[i]->translate(translation, 0, 1, Geometry::TransformType::ApplyToData);
        triMeshes[i]->rotate(Vec3d(0, 1, 0), rotation, Geometry::TransformType::ApplyToData);

        auto t = centers[i][0];
        centers[i][0] = std::cos(rotation) * t;
        centers[i][2] = -std::sin(rotation) * t;

        t = dirs[i][0];
        dirs[i][0] = std::cos(rotation) * t;
        dirs[i][2] = -std::sin(rotation) * t;
    }

    auto updateFunc =
        [&](Event*) {
            // Move objects
            for (size_t i = 0; i < triMeshes.size(); ++i)
            {
                triMeshes[i]->translate(dirs[i][0], dirs[i][1], dirs[i][2], Geometry::TransformType::ApplyToData);
                centers[i] += dirs[i];
            }

            Vec3d lowerCorners, upperCorner;
            ParallelUtils::findAABB(centers, lowerCorners, upperCorner);
            if ((lowerCorners - upperCorner).norm() > 70.0)
            {
                for (size_t i = 0; i < static_cast<size_t>(dirs.size()); i++)
                {
                    dirs[i] = -dirs[i]; // Change moving direction to move the objects back if they have moved too far
                }
            }

            StopWatch timer;
            timer.start();
            octree->update();
            const auto updateTime = timer.getTimeElapsed();

            const auto numActiveNodes          = octree->getNumActiveNodes();
            const auto numAllocatedNodes       = octree->getNumAllocatedNodes();
            const auto maxNumPrimitivesInNodes = octree->getMaxNumPrimitivesInNodes();

            std::stringstream ss;
            ss << "Octree update time: " << updateTime << " ms\n"
               << "Active nodes: " << numActiveNodes
               << " (" << static_cast<double>(numActiveNodes) / static_cast<double>(numAllocatedNodes) * 100.0
               << " % usage / total allocated nodes: " << numAllocatedNodes << ")\n"
               << "Max number of primitives in tree nodes: " << maxNumPrimitivesInNodes;

            statusText->setText(ss.str());
        };

    // Set Camera configuration
    auto cam = scene->getActiveCamera();
    cam->setPosition(Vec3d(0, 15, 50));
    cam->setFocalPoint(Vec3d(0, 0, 0));

    // Lights
    {
        imstkNew<DirectionalLight> light1;
        light1->setFocalPoint(Vec3d(-1.0, -1.0, -1.0));
        light1->setIntensity(1.0);
        scene->addLight("light 1", light1);

        imstkNew<DirectionalLight> light2;
        light2->setFocalPoint(Vec3d(1.0, -1.0, -1.0));
        light2->setIntensity(1.0);
        scene->addLight("light 2", light2);
    }

    // Run the simulation
    {
        // Setup a scene manager to advance the scene in its own thread
        imstkNew<SceneManager> sceneManager;
        sceneManager->setActiveScene(scene);
        sceneManager->pause(); // Start simulation paused
        connect<Event>(sceneManager, &SceneManager::postUpdate, updateFunc);

        imstkNew<SimulationManager> driver;
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.05);

        // Update debug visual representation every render
        connect<Event>(viewer, &Viewer::preUpdate, [&](Event*)
            {
                // Update debug rendering data
                // Involves a larger buffer update so we only do it before rendering
                debugOctreeModel->debugUpdate(8, true);
            });

        // Add default mouse and keyboard controls to the viewer
        std::shared_ptr<Entity> mouseAndKeyControls =
            SimulationUtils::createDefaultSceneControl(driver);
        mouseAndKeyControls->addComponent(statusText);
        scene->addSceneObject(mouseAndKeyControls);

        driver->start();
    }

    return 0;
}
