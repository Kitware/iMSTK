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
/// \brief This benchmark is to test the speed of various rendering operations
///
int main()
{
    // SDK and Scene
    auto sdk = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("RenderingBenchmarkTest");

    auto camera = scene->getCamera();
    camera->setPosition(0, 0, 1);
    camera->setFocalPoint(0, 0, -1);

    int numMeshes = 2000;
    std::cout << "Enter number of meshes: ";
    std::cin >> numMeshes;

    size_t numLines = 100;
    std::cout << "Enter number of triangles: ";
    std::cin >> numLines;

    bool dynamicMesh = true;
    std::cout << "Enter dynamic mesh: ";
    std::cin >> dynamicMesh;

    std::vector<LineMesh::LineArray> lines;
    StdVectorOfVec3d vertices;

    vertices.resize(numLines * 2);
    lines.resize(numLines);

    for (int i = 0; i < numLines; i++)
    {
        vertices[i * 2] = Vec3d(0, 0, 0);
        vertices[i * 2 + 1] = Vec3d(0, 0, 0.1);
        lines[i][0] = i * 2;
        lines[i][1] = i * 2 + 1;
    }

    for (int j = 0; j < numMeshes; j++)
    {
        // Construct surface mesh
        auto mesh = std::make_shared<LineMesh>();

        mesh->initialize(vertices, lines);

        if (dynamicMesh)
        {
            auto object = std::make_shared<PbdObject>(std::string("mesh") + std::to_string(j));
            object->setVisualGeometry(mesh);

            auto model = std::make_shared<PbdModel>();
            model->setModelGeometry(mesh);
            object->setDynamicalModel(model);
            scene->addSceneObject(object);
        }
        else
        {
            auto object = std::make_shared<VisualObject>(std::string("mesh") + std::to_string(j));
            object->setVisualGeometry(mesh);
            scene->addSceneObject(object);
        }

    }

    int frame = 0;
    auto watch = std::make_shared<StopWatch>();
    auto startWatch = std::make_shared<StopWatch>();
    double endTime = 0;
    int startFrame = -1;
    int startupFrame = -1;

    sdk->getViewer()->setOnTimerFunction([&](InteractorStyle* c) -> bool
        {
            if (dynamicMesh)
            {
                for (int j = 0; j < numMeshes; j++)
                {
                    auto object = scene->getSceneObject(std::string("mesh") + std::to_string(j));
                    auto mesh = std::dynamic_pointer_cast<LineMesh>(object->getVisualGeometry());

                    mesh->setVertexPositions(vertices);
                    mesh->setLinesVertices(lines);
                }
            }

            if (scene->getSceneObject("mesh" + std::to_string(numMeshes - 1)) && startupFrame == -1)
            {
                startupFrame = frame;
                LOG(INFO) << "Start time: " << startWatch->getTimeElapsed();
            }
            if (frame == startupFrame + 100)
            {
                startFrame = frame;
                watch->start();
                watch->reset();
                LOG(INFO) << "Starting time";
            }
            else if (frame == startFrame + 100 && startFrame != -1)
            {
                endTime = watch->getTimeElapsed();
                LOG(INFO) << "Total time: " << endTime;
                LOG(INFO) << "Frame time: " << endTime / (frame - startFrame);
            }
            frame++;
            return true;
        }
    );

    // Start simulation
    startWatch->start();
    startWatch->reset();
    sdk->setActiveScene(scene);
    sdk->startSimulation();
}
