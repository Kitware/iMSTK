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
#include "imstkMeshIO.h"
#include "imstkPbdObject.h"
#include "imstkPbdSolver.h"
#include "imstkOneToOneMap.h"
#include "imstkAPIUtilities.h"
#include "imstkLineMesh.h"

using namespace imstk;

///
/// \brief This example demonstrates line mesh rendering
///
int
main()
{
    // simManager and Scene
    auto simManager   = std::make_shared<SimulationManager>();
    auto scene = simManager->createNewScene("LineMeshRenderingTest");

    // Construct line mesh
    auto lineMesh         = std::make_shared<LineMesh>();
    auto lineMeshMaterial = std::make_shared<RenderMaterial>();
    lineMeshMaterial->setLineWidth(3);
    auto lineObject = std::make_shared<VisualObject>("lineMesh");

    std::vector<LineMesh::LineArray> lines;
    StdVectorOfVec3d                 points;
    std::vector<Color>               colors;

    size_t resolution = 16;
    size_t numVoxels  = resolution * resolution * resolution;

    points.resize(numVoxels * 8);
    colors.resize(numVoxels * 8);
    lines.resize(numVoxels * 12);

    size_t index     = 0;
    size_t lineIndex = 0;

    for (int z = 0; z < resolution; z++)
    {
        for (int y = 0; y < resolution; y++)
        {
            for (int x = 0; x < resolution; x++)
            {
                Color color = Color((float)x / resolution, (float)y / resolution, (float)z / resolution);

                points[index + 0] = Vec3d(x, y, z);
                points[index + 1] = Vec3d(x, y, z + 1);
                points[index + 2] = Vec3d(x, y + 1, z);
                points[index + 3] = Vec3d(x, y + 1, z + 1);
                points[index + 4] = Vec3d(x + 1, y, z);
                points[index + 5] = Vec3d(x + 1, y, z + 1);
                points[index + 6] = Vec3d(x + 1, y + 1, z);
                points[index + 7] = Vec3d(x + 1, y + 1, z + 1);

                colors[index + 0] = color;
                colors[index + 1] = color;
                colors[index + 2] = color;
                colors[index + 3] = color;
                colors[index + 4] = color;
                colors[index + 5] = color;
                colors[index + 6] = color;
                colors[index + 7] = color;

                lines[lineIndex + 0][0] = index + 0;
                lines[lineIndex + 0][1] = index + 1;
                lines[lineIndex + 1][0] = index + 2;
                lines[lineIndex + 1][1] = index + 3;
                lines[lineIndex + 2][0] = index + 4;
                lines[lineIndex + 2][1] = index + 5;
                lines[lineIndex + 3][0] = index + 6;
                lines[lineIndex + 3][1] = index + 7;

                lines[lineIndex + 4][0] = index + 0;
                lines[lineIndex + 4][1] = index + 2;
                lines[lineIndex + 5][0] = index + 1;
                lines[lineIndex + 5][1] = index + 3;
                lines[lineIndex + 6][0] = index + 4;
                lines[lineIndex + 6][1] = index + 6;
                lines[lineIndex + 7][0] = index + 5;
                lines[lineIndex + 7][1] = index + 7;

                lines[lineIndex + 8][0]  = index + 0;
                lines[lineIndex + 8][1]  = index + 4;
                lines[lineIndex + 9][0]  = index + 1;
                lines[lineIndex + 9][1]  = index + 5;
                lines[lineIndex + 10][0] = index + 2;
                lines[lineIndex + 10][1] = index + 6;
                lines[lineIndex + 11][0] = index + 3;
                lines[lineIndex + 11][1] = index + 7;

                index     += 8;
                lineIndex += 12;
            }
        }
    }

    lineMesh->initialize(points, lines);
    lineMesh->setVertexColors(colors);
    auto lineModel = std::make_shared<VisualModel>(lineMesh);
    lineModel->setRenderMaterial(lineMeshMaterial);
    lineObject->addVisualModel(lineModel);

    auto camera = scene->getCamera();
    camera->setPosition(resolution / 2.0, resolution / 2.0, resolution * 4.0);
    camera->setFocalPoint(resolution / 2.0, resolution / 2.0, resolution / 2.0);

    lineObject->setVisualGeometry(lineMesh);
    scene->addSceneObject(lineObject);

    // Start simulation
    simManager->setActiveScene(scene);
    simManager->startSimulation();
}
