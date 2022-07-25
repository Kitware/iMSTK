/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include <gtest/gtest.h>

#include "imstkTbbTaskGraphController.h"
#include "imstkTaskGraph.h"
#include "imstkTaskNode.h"

using namespace imstk;

TEST(imstkTbbTaskGraphControllerTest, SumData)
{
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
    auto graph = std::make_shared<TaskGraph>();

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
    TbbTaskGraphController controller;
    controller.setTaskGraph(graph);
    EXPECT_EQ(controller.initialize(), true) << "TaskGraph failed to initialize";
    controller.execute();
}