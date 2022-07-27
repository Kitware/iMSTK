/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "gtest/gtest.h"

#include "imstkGraph.h"

using namespace imstk;

bool
verifyColoring(const Graph& graph, const std::vector<unsigned short>& colors)
{
    std::unordered_set<size_t> edges;
    for (size_t i = 0; i < graph.size(); ++i)
    {
        unsigned short color_i = colors[i];
        graph.getEdges(i, edges);
        for (auto j : edges)
        {
            if (color_i == colors[j])
            {
                std::cout << "edge(" << i << "," << j << "): same color!" << std::endl;
                return false;
            }
        }
    }
    return true;
}

TEST(imstkGraphTest, GreedyColoring)
{
    Graph g1(5);
    g1.addEdge(0, 1);
    g1.addEdge(0, 2);
    g1.addEdge(1, 2);
    g1.addEdge(1, 3);
    g1.addEdge(2, 3);
    g1.addEdge(3, 4);
    // g1.print();
    auto colorsG1 = g1.doColoring(Graph::ColoringMethod::Greedy, false);
    EXPECT_EQ(true, verifyColoring(g1, colorsG1.first));

    Graph g2(5);
    g2.addEdge(0, 1);
    g2.addEdge(0, 2);
    g2.addEdge(1, 2);
    g2.addEdge(1, 4);
    g2.addEdge(2, 4);
    g2.addEdge(4, 3);
    // g2.print();
    auto colorsG2 = g2.doColoring(Graph::ColoringMethod::Greedy, false);
    EXPECT_EQ(true, verifyColoring(g2, colorsG2.first));
}

TEST(imstkGraphTest, WelshPowellColoring)
{
    Graph g1(5);
    g1.addEdge(0, 1);
    g1.addEdge(0, 2);
    g1.addEdge(1, 2);
    g1.addEdge(1, 3);
    g1.addEdge(2, 3);
    g1.addEdge(3, 4);
    // g1.print();
    auto colorsG1 = g1.doColoring(Graph::ColoringMethod::WelshPowell, false);
    EXPECT_EQ(true, verifyColoring(g1, colorsG1.first));

    Graph g2(5);
    g2.addEdge(0, 1);
    g2.addEdge(0, 2);
    g2.addEdge(1, 2);
    g2.addEdge(1, 4);
    g2.addEdge(2, 4);
    g2.addEdge(4, 3);
    // g2.print();
    auto colorsG2 = g2.doColoring(Graph::ColoringMethod::WelshPowell, false);
    EXPECT_EQ(true, verifyColoring(g2, colorsG2.first));
}