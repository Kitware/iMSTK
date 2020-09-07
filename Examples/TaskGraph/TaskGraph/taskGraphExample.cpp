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

#include "imstkNew.h"
#include "imstkParallelFor.h"
#include "imstkSequentialTaskGraphController.h"
#include "imstkTaskGraph.h"
#include "imstkTbbTaskGraphController.h"
#include "imstkLogger.h"

using namespace imstk;

///
/// \brief This example how to use a TaskGraph standalone
///
int
main()
{
    Logger::startLogger();

    const bool runExampleInParallel = true;

    // Initialize some data arrays
    const int countA = 100;
    const int countB = 255;
    int       x[countA];
    int       y[countA];
    int       w[countB];
    int       z[countB];

    for (int i = 0; i < 100; i++)
    {
        x[i] = i + 5;
        y[i] = i * 6 + 1;
    }
    for (int i = 0; i < 255; i++)
    {
        w[i] = i % 10;
        z[i] = i;
    }

    int sumA = 0;
    int sumB = 0;

    // Now create a graph that computes results = sum_i(x_i + y_i) * sum_i(w_i * z_i)
    imstkNew<TaskGraph> graph;

    // Create and add the nodes
    std::shared_ptr<TaskNode> addNode = graph->addFunction("Add Step",
        [&]()
    {
        for (int i = 0; i < countA; i++)
        {
            sumA += (x[i] + y[i]);
        }
        });
    std::shared_ptr<TaskNode> multNode = graph->addFunction("Mult Step",
        [&]()
    {
        for (int i = 0; i < countB; i++)
        {
            sumB += (w[i] * z[i]);
        }
        });

    // Define the edges, add and mult steps will be done in parallel
    graph->addEdge(graph->getSource(), addNode);
    graph->addEdge(graph->getSource(), multNode);
    graph->addEdge(addNode, graph->getSink());
    graph->addEdge(multNode, graph->getSink());

    // Set which controller to use
    std::shared_ptr<TaskGraphController> controller = nullptr;
    if (runExampleInParallel)
    {
        controller = std::make_shared<TbbTaskGraphController>();
    }
    else
    {
        controller = std::make_shared<SequentialTaskGraphController>();
    }

    // Compute
    controller->setTaskGraph(graph);
    if (!controller->initialize())
    {
        LOG(FATAL) << "TaskGraph failed to initialize";
        return 1;
    }
    controller->execute();
    LOG(INFO) << "Results: " << sumA + sumB;

    return 0;
}
