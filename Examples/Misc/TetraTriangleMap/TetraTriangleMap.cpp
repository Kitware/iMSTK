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

#include <cmath>
#include <cstdlib>
#include <memory>
#include <iostream>

#include "imstkAPIUtilities.h"
#include "imstkTetraTriangleMap.h"
#include "imstkTetrahedralMesh.h"
#include "imstkVolumetricMesh.h"
#include "imstkSimulationManager.h"

using namespace imstk;

/// \brief Create a triangular surface mesh with a single triangle
std::shared_ptr<SurfaceMesh> createSimpleSurfaceMesh(void);

double computeDistance(PointSet& mesh0, PointSet& mesh1);

int
main()
{
    // create a tetrahedral mesh
    Vec3d  aabbMin(0.0, 0.0, 0.0);
    Vec3d  aabbMax(1.0, 1.0, 1.0);
    size_t nx      = 2;
    size_t ny      = 2;
    size_t nz      = 2;
    auto   tetMesh = TetrahedralMesh::createUniformMesh(aabbMin, aabbMax, nx, ny, nz);

    // create triangle mesh
    auto surfaceMesh = createSimpleSurfaceMesh();
    surfaceMesh->flipNormals();

    // create a map
    auto map = std::make_shared<TetraTriangleMap>(tetMesh, surfaceMesh);

    map->compute();

    Vec3d dxyz(0.1, 0.2, 0.3);
    tetMesh->translate(dxyz, Geometry::TransformType::ApplyToData);

    map->apply();

    auto surfaceMesh2 = createSimpleSurfaceMesh();
    surfaceMesh2->translate(dxyz, Geometry::TransformType::ApplyToData);

    auto dist = computeDistance(*surfaceMesh, *surfaceMesh2);

    if (dist > 10. * VERY_SMALL_EPSILON)
    {
        std::cerr << "The TetraTriangleMap is erroneous";
    }

    std::cout << "Distance: " << dist << std::endl;

    auto simManager = std::make_shared<SimulationManager>();
    auto scene      = simManager->createNewScene("DeformableBodyFEM");
    scene->getCamera()->setPosition(5., 5., 5.);

    auto material = std::make_shared<RenderMaterial>();
    material->setDisplayMode(RenderMaterial::DisplayMode::WIREFRAME_SURFACE);
    material->setLineWidth(2.);
    material->setColor(Color(1., 0., 0., 1));
    auto surfMeshModel = std::make_shared<VisualModel>(surfaceMesh);
    surfMeshModel->setRenderMaterial(material);

    auto obj = std::make_shared<VisualObject>("surf");
    obj->addVisualModel(surfMeshModel);
    scene->addSceneObject(obj);

    //---------------

    auto material2 = std::make_shared<RenderMaterial>();
    material2->setDisplayMode(RenderMaterial::DisplayMode::WIREFRAME);
    material2->setLineWidth(3.);
    auto surfMeshModel2 = std::make_shared<VisualModel>(tetMesh);
    surfMeshModel2->setRenderMaterial(material2);

    auto volObj = std::make_shared<VisualObject>("volObj");
    volObj->addVisualModel(surfMeshModel2);
    scene->addSceneObject(volObj);

    //---------------

    // Rotate the dragon every frame
    auto rotateFunc = [&tetMesh, &map, &surfaceMesh2](Module* module)
                      {
                          tetMesh->rotate(Vec3d(1., 0, 0), PI / 10000, Geometry::TransformType::ApplyToData);
                          map->apply();
                      };
    simManager->getSceneManager(scene)->setPostUpdateCallback(rotateFunc);

    // Light
    auto light = std::make_shared<DirectionalLight>("light");
    light->setFocalPoint(Vec3d(5, -8, -5));
    light->setIntensity(1);
    scene->addLight(light);

    // Run the simulation
    simManager->setActiveScene(scene);
    simManager->getViewer()->setBackgroundColors(Vec3d(0.3285, 0.3285, 0.6525), Vec3d(0.13836, 0.13836, 0.2748), true);
    simManager->start();

    return 0;
}

std::shared_ptr<SurfaceMesh>
createSimpleSurfaceMesh()
{
    // coordinates
    StdVectorOfVec3d surfaceVertices(3);
    const double     w[3] = { 2.0 / 3.0, 1.0 / 3.0, 1.0 / 3.0 };
    surfaceVertices[0][0] = w[0] * 0.0 + w[1] * 1.0 + w[2] * 0.0;
    surfaceVertices[0][1] = w[0] * 1.0 + w[1] * 0.0 + w[2] * 0.0;
    surfaceVertices[0][2] = w[0] * 0.0 + w[1] * 0.0 + w[2] * 1.0;

    surfaceVertices[1][0] = w[2] * 0.0 + w[0] * 1.0 + w[1] * 0.0;
    surfaceVertices[1][1] = w[2] * 1.0 + w[0] * 0.0 + w[1] * 0.0;
    surfaceVertices[1][2] = w[2] * 0.0 + w[0] * 0.0 + w[1] * 1.0;

    surfaceVertices[2][0] = w[1] * 0.0 + w[2] * 1.0 + w[0] * 0.0;
    surfaceVertices[2][1] = w[1] * 1.0 + w[2] * 0.0 + w[0] * 0.0;
    surfaceVertices[2][2] = w[1] * 0.0 + w[2] * 0.0 + w[0] * 1.0;

    // connectivity
    std::vector<SurfaceMesh::TriangleArray> surfaceIndices = { { 0, 1, 2 } };
    auto                                    surfaceMesh    = std::make_shared<SurfaceMesh>();
    surfaceMesh->initialize(surfaceVertices, surfaceIndices);

    return surfaceMesh;
}

double
computeDistance(PointSet& mesh0, PointSet& mesh1)
{
    assert(mesh0.getNumVertices() == mesh1.getNumVertices());

    auto   verts0 = mesh0.getVertexPositions();
    auto   verts1 = mesh1.getVertexPositions();
    double s      = 0.;
    for (size_t i = 0; i < mesh0.getNumVertices(); ++i)
    {
        s += (verts0[i] - verts1[i]).norm();
    }

    return s;
}
