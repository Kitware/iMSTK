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
#include "imstkTetrahedralMesh.h"
#include "imstkMeshIO.h"
#include "imstkDebugGeometry.h"
#include "imstkAPIUtilities.h"
#include "imstkSurfaceMesh.h"
#include <memory>

using namespace imstk;

///
/// \brief This example demonstrates the debug rendering
///
int main()
{
    auto sdk = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("DebugRendering");
    scene->getCamera()->setPosition(0, 2.0, 15.0);    

    // Create debug triangles
    auto debugTriangleGeo = std::make_shared<DebugRenderTriangles>("debugtriangles", 9);    
    StdVectorOfVec3d triVerts;
    triVerts.push_back(Vec3d(0., 0., 0.));
    triVerts.push_back(Vec3d(0., 10., 0.));
    triVerts.push_back(Vec3d(0., 0., 10.));

    triVerts.push_back(Vec3d(0., 0., 0.));
    triVerts.push_back(Vec3d(10., 0., 0.));
    triVerts.push_back(Vec3d(0., 0., 10.));

    triVerts.push_back(Vec3d(0., 0., 0.));
    triVerts.push_back(Vec3d(10., 0., 0.));
    triVerts.push_back(Vec3d(0., 10., 0.));

    debugTriangleGeo->setVertexData(triVerts);

    auto material = std::make_shared<RenderMaterial>();
    material->setBackFaceCulling(false);
    material->setDebugColor(Color::Red);
    material->setDisplayMode(RenderMaterial::DisplayMode::WIREFRAME_SURFACE);
    debugTriangleGeo->setRenderMaterial(material);

    scene->addDebugGeometry(debugTriangleGeo);

    // Create debug Lines
    auto debugLinesGeo = std::make_shared<DebugRenderLines>("debugLines", 2);

    StdVectorOfVec3d linesVerts;
    linesVerts.push_back(Vec3d(0, 0, 0));
    linesVerts.push_back(Vec3d(100, 100, 100));

    debugLinesGeo->setVertexData(linesVerts);

    auto materialLines = std::make_shared<RenderMaterial>();
    materialLines->setBackFaceCulling(false);
    materialLines->setDebugColor(Color::Green);
    materialLines->setLineWidth(2.0);
    debugLinesGeo->setRenderMaterial(materialLines);

    scene->addDebugGeometry(debugLinesGeo);

    // Create debug points
    auto debugPointsGeo = std::make_shared<DebugRenderPoints>("debugPoints", 50);    
    StdVectorOfVec3d points;
    for (unsigned int i = 0; i < 50; ++i)
    {
        points.push_back(Vec3d(i, i, i));
    }
    debugPointsGeo->setVertexData(points);

    auto materialPoints = std::make_shared<RenderMaterial>();
    materialPoints->setBackFaceCulling(false);
    materialPoints->setDebugColor(Color::Pink);
    debugPointsGeo->setRenderMaterial(materialPoints);

    scene->addDebugGeometry(debugPointsGeo);
    
    // Add light
    auto light = std::make_shared<DirectionalLight>("light");
    light->setFocalPoint(Vec3d(5, -8, -5));
    light->setIntensity(1);
    scene->addLight(light);

    // Run the simulation
    sdk->setActiveScene(scene);
    sdk->startSimulation(SimulationStatus::PAUSED);

    return 0;
}
