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

#include "imstkSimulationManager.h"
#include "imstkSceneObject.h"
#include "imstkDebugRenderGeometry.h"
#include "imstkAPIUtilities.h"
#include "imstkLooseOctree.h"
#include "imstkVTKViewer.h"
#include "imstkTimer.h"
#include "imstkVTKTextStatusManager.h"

#include <thread>
#include <chrono>
#include <sstream>

using namespace imstk;

//#define NUM_MESHES 4u
#define NUM_MESHES 10u

// Load bunny mesh data (vertex positions and triangle faces)
std::pair<StdVectorOfVec3d, std::vector<std::array<size_t, 3>>> getBunny();
static std::pair<StdVectorOfVec3d, std::vector<std::array<size_t, 3>>> g_BunnyData = getBunny();

///
/// \brief Read a mesh, create a visual scene object and add to the scene
///
std::shared_ptr<VisualObject>
createMeshObject(const std::shared_ptr<imstk::Scene>& scene,
                 const std::string&                   objectName,
                 Color                                color)
{
    // Create a surface mesh for the bunny
    auto meshObj = std::make_shared<SurfaceMesh>();
    meshObj->initialize(g_BunnyData.first, g_BunnyData.second);

    // Create a visiual model
    auto visualModel = std::make_shared<VisualModel>(meshObj);
    auto material    = std::make_shared<RenderMaterial>();
    material->setDebugColor(color); // Wireframe color
    material->setLineWidth(4);
    material->setDisplayMode(RenderMaterial::DisplayMode::WIREFRAME_SURFACE);
    visualModel->setRenderMaterial(material);

    auto visualObject = std::make_shared<VisualObject>(objectName);
    visualObject->addVisualModel(visualModel);
    scene->addSceneObject(visualObject);

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
    // SDK and Scene
    auto sdk   = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("Octree Example");
    sdk->setActiveScene(scene);

    // Get the VTKViewer
    auto viewer = std::dynamic_pointer_cast<VTKViewer>(sdk->getViewer());
    viewer->getVtkRenderWindow()->SetSize(1920, 1080);

    auto statusManager = viewer->getTextStatusManager();
    statusManager->setStatusFontSize(VTKTextStatusManager::Custom, 30);
    statusManager->setStatusFontColor(VTKTextStatusManager::Custom, Color::Orange);

    // Get VTK Renderer
    auto renderer = std::dynamic_pointer_cast<VTKRenderer>(viewer->getActiveRenderer());
    LOG_IF(FATAL, (!renderer)) << "Invalid renderer: Only VTKRenderer is supported for debug rendering";

    //    srand(123456); // Deterministic random generation, for random colors
    srand(static_cast<unsigned int>(time(nullptr)));

    // Create meshes
    std::vector<std::shared_ptr<SurfaceMesh>> triMeshes;
    for (unsigned int i = 0; i < NUM_MESHES; ++i)
    {
        const auto sceneObj = createMeshObject(scene, "Mesh-" + std::to_string(triMeshes.size()), getRandomColor());
        triMeshes.push_back(std::dynamic_pointer_cast<SurfaceMesh>(sceneObj->getVisualGeometry()));
    }

    // Compute the scale factor to scale meshes such that meshes with different sizes are still visualized consistently
    Vec3d      lowerCorner, upperCorner;
    const auto pointset = std::dynamic_pointer_cast<PointSet>(triMeshes.front());
    ParallelUtils::findAABB(pointset->getVertexPositions(), lowerCorner, upperCorner);
    const auto scaleFactor = 20.0 / (upperCorner - lowerCorner).norm();
    for (const auto& mesh: triMeshes)
    {
        mesh->scale(scaleFactor, Geometry::TransformType::ApplyToData);
    }

    StopWatch timer;
    timer.start();

    // Create octree
    LooseOctree octree(Vec3d(0, 0, 0), 100.0, 0.125, 2.0, "TestOctree");

    // Add all meshes to the octree
    for (const auto& mesh: triMeshes)
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
    const auto debugOctree = octree.getDebugGeometry(8, true);
    scene->addDebugGeometry(debugOctree);

    // Data for animation
    const double     translation = 15.0;
    StdVectorOfVec3d centers;
    StdVectorOfVec3d dirs;
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

        t          = dirs[i][0];
        dirs[i][0] = std::cos(rotation) * t;
        dirs[i][2] = -std::sin(rotation) * t;
    }

    auto updateFunc =
        [&](Module*) {
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
            for (auto& delegate : renderer->getDebugRenderDelegates())
            {
                delegate->updateDataSource();
            }

            // Pause for a while
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        };
    sdk->getSceneManager(scene)->setPostUpdateCallback(updateFunc);

    // Set Camera configuration
    auto cam = scene->getCamera();
    cam->setPosition(Vec3d(0, 15, 50));
    cam->setFocalPoint(Vec3d(0, 0, 0));

    // Light
    {
        auto light = std::make_shared<DirectionalLight>("Light 1");
        light->setFocalPoint(Vec3d(-1, -1, -1));
        light->setIntensity(1);
        scene->addLight(light);
    }
    {
        auto light = std::make_shared<DirectionalLight>("Light 2");
        light->setFocalPoint(Vec3d(1, -1, -1));
        light->setIntensity(1);
        scene->addLight(light);
    }

    // Run
    sdk->startSimulation(SimulationStatus::PAUSED);

    return 0;
}
