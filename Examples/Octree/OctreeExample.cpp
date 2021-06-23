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
#include "imstkDebugRenderGeometry.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkDirectionalLight.h"
#include "imstkLogger.h"
#include "imstkLooseOctree.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSceneObject.h"
#include "imstkSimulationManager.h"
#include "imstkSurfaceMesh.h"
#include "imstkTimer.h"
#include "imstkVisualModel.h"
#include "imstkVTKRenderDelegate.h"
#include "imstkVTKRenderer.h"
#include "imstkVTKTextStatusManager.h"
#include "imstkVTKViewer.h"

#include <vtkRenderWindow.h>

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
    imstkNew<VisualModel>    visualModel(surfMesh.get());
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
    imstkNew<VTKViewer> viewer("Viewer");
    viewer->setActiveScene(scene);
    viewer->setWindowTitle("Octree Example");
    viewer->getVtkRenderWindow()->SetSize(1920, 1080);

    auto statusManager = viewer->getTextStatusManager();
    statusManager->setStatusFontSize(VTKTextStatusManager::StatusType::Custom, 30);
    statusManager->setStatusFontColor(VTKTextStatusManager::StatusType::Custom, Color::Orange);

    // Get VTK Renderer
    auto renderer = std::dynamic_pointer_cast<VTKRenderer>(viewer->getActiveRenderer());
    LOG_IF(FATAL, (!renderer)) << "Invalid renderer: Only VTKRenderer is supported for debug rendering";

    //    srand(123456); // Deterministic random generation, for random colors
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
    LooseOctree octree(Vec3d(0.0, 0.0, 0.0), 100.0, 0.125, 2.0, "TestOctree");

    // Add all meshes to the octree
    for (const auto& mesh : triMeshes)
    {
        octree.addTriangleMesh(mesh);
    }

    // Build octree after adding all geometries
    octree.build();
    LOG(INFO) << "Build octree time: " << timer.getTimeElapsed() << " ms";

    // Always rebuild tree from scratch in each update (default update is incremental update)
    // This is significantly slower than incremental update!
    // octree.setAlwaysRebuild(true);

    // Create debug geometry for the octree (render up to 8 levels, and render all non-empty nodes)
    std::shared_ptr<DebugRenderGeometry> debugOctree = octree.getDebugGeometry(8, true);

    imstkNew<RenderMaterial> matDbgViz;
    matDbgViz->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);
    matDbgViz->setColor(Color::Green);
    matDbgViz->setLineWidth(1.0);
    imstkNew<VisualModel> octreeVizDbgModel(debugOctree, matDbgViz);
    scene->addDebugVisualModel(octreeVizDbgModel);

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
                for (size_t i = 0; i < dirs.size(); ++i)
                {
                    dirs[i] = -dirs[i]; // Change moving direction to move the objects back if they have moved too far
                }
            }

            StopWatch timer;
            timer.start();
            octree.update();
            const auto updateTime = timer.getTimeElapsed();

            const auto numActiveNodes          = octree.getNumActiveNodes();
            const auto numAllocatedNodes       = octree.getNumAllocatedNodes();
            const auto maxNumPrimitivesInNodes = octree.getMaxNumPrimitivesInNodes();

            std::stringstream ss;
            ss << "Octree update time: " << updateTime << " ms\n"
               << "Active nodes: " << numActiveNodes
               << " (" << static_cast<double>(numActiveNodes) / static_cast<double>(numAllocatedNodes) * 100.0
               << " % usage / total allocated nodes: " << numAllocatedNodes << ")\n"
               << "Max number of primitives in tree nodes: " << maxNumPrimitivesInNodes;

            statusManager->setCustomStatus(ss.str());

            // Update debug rendering data
            octree.updateDebugGeometry();
            debugOctree->setDataModified(true);
            for (auto& delegate : renderer->getDebugRenderDelegates())
            {
                delegate->processEvents();
            }

            // Pause for a while
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
        imstkNew<SceneManager> sceneManager("Scene Manager");
        sceneManager->setActiveScene(scene);
        sceneManager->pause(); // Start simulation paused
        connect<Event>(sceneManager, &SceneManager::postUpdate, updateFunc);

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
