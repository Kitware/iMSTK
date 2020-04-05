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
#include "imstkLight.h"
#include "imstkSceneObject.h"
#include "imstkDebugRenderGeometry.h"
#include "imstkAPIUtilities.h"
#include "imstkVTKViewer.h"
#include "imstkTimer.h"
#include "imstkVTKTextStatusManager.h"
#include "imstkCollisionData.h"

// This is for collision detection between geometries
#include "imstkOctreeBasedCD.h"

#include <map>
#include <thread>
#include <chrono>

using namespace imstk;

#define NUM_MESHES 5u

// Uncomment this to render octree
// #define DEBUG_RENDER_OCTREE

// Load pre-computed mesh data (vertex positions and triangle faces)
std::pair<StdVectorOfVec3d, std::vector<std::array<size_t, 3>>> getTriangle();
std::pair<StdVectorOfVec3d, std::vector<std::array<size_t, 3>>> getBox();
std::pair<StdVectorOfVec3d, std::vector<std::array<size_t, 3>>> getSphere();
std::pair<StdVectorOfVec3d, std::vector<std::array<size_t, 3>>> getBunny();

static std::pair<StdVectorOfVec3d, std::vector<std::array<size_t, 3>>> g_TriangleData = getTriangle();
static std::pair<StdVectorOfVec3d, std::vector<std::array<size_t, 3>>> g_BoxData      = getBox();
static std::pair<StdVectorOfVec3d, std::vector<std::array<size_t, 3>>> g_SphereData   = getSphere();
static std::pair<StdVectorOfVec3d, std::vector<std::array<size_t, 3>>> g_BunnyData    = getBunny();

// Set MESH_DATA to 1 out of 4 data variables above
#define MESH_DATA g_BunnyData

///
/// \brief Read a mesh, create a visual scene object and add to the scene
///
std::shared_ptr<CollidingObject>
createMeshObject(const std::shared_ptr<imstk::Scene>& scene,
                 const std::string&                   objectName,
                 Color                                color)
{
    // Create a surface mesh
    auto meshObj = std::make_shared<SurfaceMesh>(objectName);
    meshObj->initialize(MESH_DATA.first, MESH_DATA.second);

    // Create a visiual model
    auto visualModel = std::make_shared<VisualModel>(meshObj);
    auto material    = std::make_shared<RenderMaterial>();
    material->setDebugColor(color); // Wireframe color
    material->setLineWidth(2);
    if (meshObj->getNumTriangles() > 100)
    {
        material->setDisplayMode(RenderMaterial::DisplayMode::WIREFRAME_SURFACE);
    }
    else
    {
        material->setDisplayMode(RenderMaterial::DisplayMode::WIREFRAME);
    }
    visualModel->setRenderMaterial(material);

    // Create object and add to scene
    auto meshSceneObject = std::make_shared<CollidingObject>(objectName);
    meshSceneObject->setCollidingGeometry(meshObj);
    meshSceneObject->addVisualModel(visualModel);
    scene->addSceneObject(meshSceneObject);

    LOG_IF(FATAL, (!meshSceneObject)) << "ERROR: Unable to create scene object";
    return meshSceneObject;
}

///
/// \brief Create debug rendering object to render colliding vertices
///
std::shared_ptr<DebugRenderGeometry>
addPointsDebugRendering(const std::shared_ptr<Scene>& scene)
{
    auto debugPoints = std::make_shared<DebugRenderPoints>("Debug Points");
    auto material    = std::make_shared<RenderMaterial>();
    material->setDebugColor(Color::Yellow);
    material->setSphereGlyphSize(.01);
    debugPoints->setRenderMaterial(material);
    scene->addDebugGeometry(debugPoints);

    return std::dynamic_pointer_cast<DebugRenderGeometry>(debugPoints);
}

///
/// \brief Create debug rendering object to render lines connecting colliding vertex and triangle
///
std::shared_ptr<DebugRenderGeometry>
addVTConnectingLinesDebugRendering(const std::shared_ptr<Scene>& scene)
{
    auto debugLines = std::make_shared<DebugRenderLines>("Debug Connecting VT Lines");
    auto material   = std::make_shared<RenderMaterial>();
    material->setBackFaceCulling(false);
    material->setDebugColor(Color::Green);
    material->setLineWidth(4.0);
    debugLines->setRenderMaterial(material);
    scene->addDebugGeometry(debugLines);

    return std::dynamic_pointer_cast<DebugRenderGeometry>(debugLines);
}

///
/// \brief Create debug rendering object to render lines connecting colliding edge-edge
///
std::shared_ptr<DebugRenderGeometry>
addEEConnectingLinesDebugRendering(const std::shared_ptr<Scene>& scene)
{
    auto debugLines = std::make_shared<DebugRenderLines>("Debug Connecting EE Lines");
    auto material   = std::make_shared<RenderMaterial>();
    material->setBackFaceCulling(false);
    material->setDebugColor(Color::Red);
    material->setLineWidth(4.0);
    debugLines->setRenderMaterial(material);
    scene->addDebugGeometry(debugLines);

    return std::dynamic_pointer_cast<DebugRenderGeometry>(debugLines);
}

///
/// \brief Create debug rendering object to render highlighted segments/triangles
///
std::shared_ptr<DebugRenderGeometry>
addHighlightedLinesDebugRendering(const std::shared_ptr<Scene>& scene)
{
    auto debugLines = std::make_shared<DebugRenderLines>("Debug Highlighted Lines");
    auto material   = std::make_shared<RenderMaterial>();
    material->setBackFaceCulling(false);
    material->setDebugColor(Color::Orange);
    material->setLineWidth(8.0);
    debugLines->setRenderMaterial(material);
    scene->addDebugGeometry(debugLines);

    return std::dynamic_pointer_cast<DebugRenderGeometry>(debugLines);
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
/// \brief This example demonstrates collision detection for meshes in iMSTK and debug rendering with debug geometries
///
int
main()
{
    // simManager and Scene
    auto simManager = std::make_shared<SimulationManager>();
    auto scene      = simManager->createNewScene("Collision Test");
    simManager->setActiveScene(scene);

    // Get the VTKViewer
    auto viewer = std::dynamic_pointer_cast<VTKViewer>(simManager->getViewer());
    viewer->getVtkRenderWindow()->SetSize(1920, 1080);

    auto statusManager = viewer->getTextStatusManager();
    statusManager->setStatusFontSize(VTKTextStatusManager::Custom, 25);
    statusManager->setStatusFontColor(VTKTextStatusManager::Custom, Color::Orange);

    // Get VTK Renderer
    auto renderer = std::dynamic_pointer_cast<VTKRenderer>(viewer->getActiveRenderer());
    LOG_IF(FATAL, (!renderer)) << "Invalid renderer: Only VTKRenderer is supported for debug rendering";

//    srand(123456); // Deterministic random generation, for random colors
    srand(static_cast<unsigned int>(time(nullptr)));

    // Create NUM_MESHES objects
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
    for (const auto& obj: triMeshes)
    {
        obj->scale(scaleFactor, Geometry::TransformType::ApplyToData);
    }

    StopWatch timer;
    timer.start();

    // Create an octree
    OctreeBasedCD octreeCD(Vec3d(0, 0, 0), 100.0, 0.125, 1);

    for (const auto& obj: triMeshes)
    {
        octreeCD.addTriangleMesh(std::dynamic_pointer_cast<SurfaceMesh>(obj));
    }

    // Build octree after adding all geometries
    octreeCD.build();
    LOG(INFO) << "Build octree time: " << timer.getTimeElapsed() << " ms";

    // Add collision pairs between objects
    for (unsigned int i = 0; i < NUM_MESHES; ++i)
    {
        for (unsigned int j = i; j < NUM_MESHES; ++j)
        {
            if (i != j)
            {
                octreeCD.addCollisionPair(triMeshes[i], triMeshes[j], CollisionDetection::Type::SurfaceMeshToSurfaceMesh);
            }
        }
    }

    // Debug rendering objects
    const auto debugPoints = addPointsDebugRendering(scene);
    const auto debugVTConnectingLines = addVTConnectingLinesDebugRendering(scene);
    const auto debugEEConnectingLines = addEEConnectingLinesDebugRendering(scene);
    const auto debugHighlightedLines  = addHighlightedLinesDebugRendering(scene);

    // Add debug geometry to render octree
#ifdef DEBUG_RENDER_OCTREE
    const auto debugOctree = octreeCD.getDebugGeometry(8, false);
    scene->addDebugGeometry(debugOctree);
#endif

    // Helper variables for animation
    const double     translation = 10.0;
    StdVectorOfVec3d centers;
    StdVectorOfVec3d dirs;
    for (unsigned int i = 0; i < NUM_MESHES; ++i)
    {
        centers.push_back(Vec3d(translation, 0, 0));
        dirs.push_back(Vec3d(-0.1, 0, 0));
    }

    // Transform the objects
    const double angle = 2.0 * PI / NUM_MESHES;
    for (unsigned int i = 0; i < NUM_MESHES; ++i)
    {
        const auto rotation = angle * static_cast<double>(i);
        triMeshes[i]->translate(translation, 0, -1, Geometry::TransformType::ApplyToData);
        triMeshes[i]->rotate(Vec3d(0, 1, 0), rotation, Geometry::TransformType::ApplyToData);
        if (i == 0 && triMeshes[i]->getNumTriangles() == 1)
        {
            triMeshes[i]->rotate(Vec3d(1, 0, 0), PI * 0.5, Geometry::TransformType::ApplyToData);
            triMeshes[i]->translate(Vec3d(0, 7, 0), Geometry::TransformType::ApplyToData);
        }

        auto t = centers[i][0];
        centers[i][0] = std::cos(rotation) * t;
        centers[i][2] = -std::sin(rotation) * t;

        t = dirs[i][0];
        dirs[i][0] = std::cos(rotation) * t;
        dirs[i][2] = -std::sin(rotation) * t;
    }

    auto updateFunc =
        [&](Module*) {
            for (size_t i = 0; i < triMeshes.size(); ++i)
            {
                triMeshes[i]->translate(dirs[i][0], dirs[i][1], dirs[i][2], Geometry::TransformType::ApplyToData);
                centers[i] += dirs[i];

                // Trick to force update geometry postUpdateTransform
                const auto positions = triMeshes[i]->getVertexPositions();
                (void)positions;
            }

            Vec3d lowerCorners, upperCorner;
            ParallelUtils::findAABB(centers, lowerCorners, upperCorner);
            if ((lowerCorners - upperCorner).norm() > 70.0)
            {
                for (auto& dir : dirs)
                {
                    dir = -dir;
                }
            }

            StopWatch timer;

            // Update octree
            timer.start();
            octreeCD.update();
            const auto updateTime = timer.getTimeElapsed();
#ifdef DEBUG_RENDER_OCTREE
            octreeCD.updateDebugGeometry();
#endif

            // Detect collision
            timer.start();
            octreeCD.detectCollision();
            const auto CDTime = timer.getTimeElapsed();

            const auto numActiveNodes = octreeCD.getNumActiveNodes();
            const auto numTotalNodes  = octreeCD.getNumAllocatedNodes();
            const auto numPrimitives  = octreeCD.getPrimitiveCount(OctreePrimitiveType::Point) +
                                        octreeCD.getPrimitiveCount(OctreePrimitiveType::Triangle) +
                                        octreeCD.getPrimitiveCount(OctreePrimitiveType::AnalyticalGeometry);
            const auto maxNumPrimitivesInTree = octreeCD.getMaxNumPrimitivesInNodes();

            // Clear collision debug rendering
            debugPoints->clear();
            debugVTConnectingLines->clear();
            debugEEConnectingLines->clear();
            debugHighlightedLines->clear();

            size_t numVTCollisions = 0;
            size_t numEECollisions = 0;
            for (const auto& geoPair : octreeCD.getCollidingGeometryPairs())
            {
                const auto& colData = octreeCD.getCollisionPairData(geoPair.first->getGlobalIndex(),
                                                                    geoPair.second->getGlobalIndex());
                if (colData->VTColData.getSize())
                {
                    numVTCollisions += colData->VTColData.getSize();
                    const auto mesh1 = static_cast<SurfaceMesh*>(geoPair.first);
                    const auto mesh2 = static_cast<SurfaceMesh*>(geoPair.second);
                    for (size_t i = 0; i < colData->VTColData.getSize(); ++i)
                    {
                        const auto tv = colData->VTColData[i];
                        const auto v  = mesh1->getVertexPosition(tv.vertexIdx);
                        debugPoints->appendVertex(v);

                        const auto face = mesh2->getTrianglesVertices()[tv.triIdx];
                        const auto tv0  = mesh2->getVertexPosition(face[0]);
                        const auto tv1  = mesh2->getVertexPosition(face[1]);
                        const auto tv2  = mesh2->getVertexPosition(face[2]);

                        debugHighlightedLines->appendVertex(tv0);
                        debugHighlightedLines->appendVertex(tv1);
                        debugHighlightedLines->appendVertex(tv1);
                        debugHighlightedLines->appendVertex(tv2);
                        debugHighlightedLines->appendVertex(tv2);
                        debugHighlightedLines->appendVertex(tv0);

                        debugVTConnectingLines->appendVertex(v);
                        debugVTConnectingLines->appendVertex((tv0 + tv1 + tv2) / 3.0);
                    }
                }

                if (colData->EEColData.getSize())
                {
                    numEECollisions += colData->EEColData.getSize();
                    const auto mesh1 = static_cast<SurfaceMesh*>(geoPair.first);
                    const auto mesh2 = static_cast<SurfaceMesh*>(geoPair.second);
                    for (size_t i = 0; i < colData->EEColData.getSize(); ++i)
                    {
                        const auto ee = colData->EEColData[i];

                        const auto e0v0     = mesh1->getVertexPosition(ee.edgeIdA.first);
                        const auto e0v1     = mesh1->getVertexPosition(ee.edgeIdA.second);
                        const auto e0Center = (e0v0 + e0v1) * 0.5;
                        debugPoints->appendVertex(e0Center);

                        const auto e1v0     = mesh2->getVertexPosition(ee.edgeIdB.first);
                        const auto e1v1     = mesh2->getVertexPosition(ee.edgeIdB.second);
                        const auto e1Center = (e1v0 + e1v1) * 0.5;
                        debugPoints->appendVertex(e1Center);

                        debugHighlightedLines->appendVertex(e0v0);
                        debugHighlightedLines->appendVertex(e0v1);
                        debugHighlightedLines->appendVertex(e1v0);
                        debugHighlightedLines->appendVertex(e1v1);

                        debugEEConnectingLines->appendVertex(e0Center);
                        debugEEConnectingLines->appendVertex(e1Center);
                    }
                }
            }

            debugPoints->setDataModified(true);
            debugVTConnectingLines->setDataModified(true);
            debugEEConnectingLines->setDataModified(true);
            debugHighlightedLines->setDataModified(true);

            std::stringstream ss;
            ss << "Octree update time: " << updateTime << " ms\n"
               << "Active nodes: " << numActiveNodes
               << " (" << static_cast<double>(numActiveNodes) / static_cast<double>(numTotalNodes) * 100.0
               << " % usage / total allocated nodes: " << numTotalNodes << ")\n"
               << "Num. primitives: " << numPrimitives
               << " | Max number of primitives in tree nodes: " << maxNumPrimitivesInTree
               << "\nCollision detection time: " << CDTime << " ms"
               << "\nNum. collision: " << numVTCollisions << " (VT) | "
               << numEECollisions << " (EE)";
            statusManager->setCustomStatus(ss.str());

            // Update debug rendering data
            for (auto& delegate : renderer->getDebugRenderDelegates())
            {
                delegate->updateDataSource();
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        };
    simManager->getSceneManager(scene)->setPostUpdateCallback(updateFunc);

    // Press 'b' key to move objects in the reverse directions
    viewer->setOnCharFunction('b',
        [&](InteractorStyle*) -> bool
    {
        for (auto& dir : dirs)
        {
            dir = -dir;
        }
        return true;
    });

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
    simManager->start(SimulationStatus::paused);

    return 0;
}
