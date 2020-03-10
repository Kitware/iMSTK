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

void translate(PointSet& mesh, const Vec3d& v);

/// \brief Create a triangular surface mesh with a single triangle
std::shared_ptr<SurfaceMesh> createSimpleSurfaceMesh(void);

double computeDistance(PointSet& mesh0, PointSet& mesh1);

int
main()
{
    // create a tet mesh
    Vec3d  aabbMin(0.0, 0.0, 0.0);
    Vec3d  aabbMax(1.0, 1.0, 1.0);
    size_t nx      = 2;
    size_t ny      = 2;
    size_t nz      = 2;
    auto   tetMesh = TetrahedralMesh::createUniformMesh(aabbMin, aabbMax, nx, ny, nz);

    // create tri mesh
    auto surfaceMesh = createSimpleSurfaceMesh();
    surfaceMesh->flipNormals();

    // create a map
    auto map = std::make_shared<TetraTriangleMap>(tetMesh, surfaceMesh);

    map->compute();

    Vec3d dxyz(0.1, 0.2, 0.3);
    translate(*tetMesh.get(), dxyz);

    map->apply();

    auto surfaceMesh2 = createSimpleSurfaceMesh();
    translate(*surfaceMesh2.get(), dxyz);

    auto dist = computeDistance(*surfaceMesh, *surfaceMesh2);

    std::cout << "Distance: " << dist << std::endl;

    auto simManager = std::make_shared<SimulationManager>();
    auto scene = simManager->createNewScene("DeformableBodyFEM");
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

    // Light
    auto light = std::make_shared<DirectionalLight>("light");
    light->setFocalPoint(Vec3d(5, -8, -5));
    light->setIntensity(1);
    scene->addLight(light);

    // Run the simulation
    simManager->setActiveScene(scene);
    simManager->getViewer()->setBackgroundColors(Vec3d(0.3285, 0.3285, 0.6525), Vec3d(0.13836, 0.13836, 0.2748), true);
    simManager->startSimulation();

   
    return 0;
}

std::shared_ptr<SurfaceMesh>
createSimpleSurfaceMesh()
{
    // coordinates
    // std::vector<Vec3d> surfaceVertices(3);
    StdVectorOfVec3d surfaceVertices(3);
    const double             w[3] = {2.0 / 3.0, 1.0 / 3.0, 1.0 / 3.0};
    surfaceVertices[0][0]   = w[0] * 0.0 + w[1] * 1.0 + w[2] * 0.0;
    surfaceVertices[0][1]   = w[0] * 1.0 + w[1] * 0.0 + w[2] * 0.0;
    surfaceVertices[0][2]   = w[0] * 0.0 + w[1] * 0.0 + w[2] * 1.0;

    surfaceVertices[1][0] = w[2] * 0.0 + w[0] * 1.0 + w[1] * 0.0;
    surfaceVertices[1][1] = w[2] * 1.0 + w[0] * 0.0 + w[1] * 0.0;
    surfaceVertices[1][2] = w[2] * 0.0 + w[0] * 0.0 + w[1] * 1.0;

    surfaceVertices[2][0] = w[1] * 0.0 + w[2] * 1.0 + w[0] * 0.0;
    surfaceVertices[2][1] = w[1] * 1.0 + w[2] * 0.0 + w[0] * 0.0;
    surfaceVertices[2][2] = w[1] * 0.0 + w[2] * 0.0 + w[0] * 1.0;

    // connectivity
    std::vector<SurfaceMesh::TriangleArray> surfaceIndices = {{0, 1, 2}};
    auto surfaceMesh = std::make_shared<SurfaceMesh>();
    surfaceMesh->initialize(surfaceVertices, surfaceIndices);

    return surfaceMesh;
}

void
translate(PointSet& mesh, const Vec3d& dxyz)
{
    // mesh.translateVertices(dxyz);
    // mesh.updatePostTransformData();
    mesh.print();
    std::cout << "num of points = " << mesh.getNumVertices() << std::endl; 

    for (size_t i = 0; i < mesh.getNumVertices(); ++i)
    {

        Vec3d xyz(mesh.getVertexPosition(i));
        xyz += dxyz;
        mesh.setVertexPosition(i, xyz);
    }
    return;
}

double
computeDistance(PointSet& mesh0, PointSet& mesh1)
{
    double s = 0.0;
    if (mesh0.getNumVertices() != mesh1.getNumVertices()) {
        return 1;
    }

    for (size_t i = 0; i < mesh0.getNumVertices(); ++i)
    {
        auto xyz0 = mesh0.getVertexPosition(i);
        auto xyz1 = mesh1.getVertexPosition(i);
        auto dx   = xyz0[0] - xyz1[0];
        auto dy   = xyz0[1] - xyz1[1];
        auto dz   = xyz0[2] - xyz1[2];
        s += dx * dx + dy * dy + dz * dz;
    }

    return sqrt(s);
}
