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

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "imstkTaskGraph.h"

using namespace imstk;
using testing::UnorderedElementsAre;
using testing::ElementsAre;

namespace
{
/// Check wether an edge is correctly in the taskgraph, i.e. ther is a forward and a backwards
/// connection
bool
correctlyConnected(std::shared_ptr<TaskGraph> graph, std::shared_ptr<TaskNode> a, std::shared_ptr<TaskNode> b)
{
    return graph->containsEdge(a, b) && graph->getInvAdjList().at(b).count(a) != 0;
}

/// Check if an edge was correctly removed from the taskgraph, i.e. both the forward and
/// backwards references a cleared
bool
correctlyRemoved(std::shared_ptr<TaskGraph> graph, std::shared_ptr<TaskNode> a)
{
    return !graph->containsNode(a) && graph->getAdjList().count(a) == 0 && graph->getInvAdjList().count(a) == 0;
}
}

TEST(imstkTaskGraphTest, Constructor)
{
    auto taskGraph = std::make_shared<TaskGraph>();
    EXPECT_EQ(2, taskGraph->getNodes().size());
    EXPECT_NE(taskGraph->getSource(), taskGraph->getSink());
    EXPECT_FALSE(taskGraph->containsEdge(taskGraph->getSource(), taskGraph->getSink()));
}

TEST(imstkTaskGraphTest, ContainsNode)
{
    auto taskGraph = std::make_shared<TaskGraph>();
    auto node1     = std::make_shared<TaskNode>();
    auto node2     = std::make_shared<TaskNode>();

    taskGraph->addNode(node1);

    EXPECT_TRUE(taskGraph->containsNode(node1));
    EXPECT_FALSE(taskGraph->containsNode(node2));
}

TEST(imstkTaskGraphDeathTest, InvalidEdges)
{
    auto taskGraph = std::make_shared<TaskGraph>();
    auto node1     = std::make_shared<TaskNode>();
    auto node2     = std::make_shared<TaskNode>();

    taskGraph->addNode(node1);

    /// Adding Edges to the graph with nodes that are not in the graph
    /// should fail with DEATH
    EXPECT_DEATH(taskGraph->addEdge(node1, node2), "does not exist in graph");
    EXPECT_DEATH(taskGraph->addEdge(node2, node1), "does not exist in graph");
}

TEST(imstkTaskGrapTest, RemoveEdge)
{
    auto taskGraph = std::make_shared<TaskGraph>();
    auto source    = taskGraph->getSource();
    auto sink      = taskGraph->getSink();
    auto node1     = std::make_shared<TaskNode>();
    auto node2     = std::make_shared<TaskNode>();

    /// Simple linear connections source->node1->sink
    taskGraph->addNode(node1);
    taskGraph->addEdges({
        { taskGraph->getSource(), node1 },
        { node1, taskGraph->getSink() }
        });

    EXPECT_NO_FATAL_FAILURE(taskGraph->removeEdge(node1, taskGraph->getSource()));
    EXPECT_TRUE(correctlyConnected(taskGraph, source, node1));
    EXPECT_TRUE(correctlyConnected(taskGraph, node1, sink));

    taskGraph->removeEdge(source, node1);
    EXPECT_FALSE(correctlyConnected(taskGraph, source, node1));
    EXPECT_TRUE(correctlyConnected(taskGraph, node1, sink));

    taskGraph->removeEdge(node1, sink);
    EXPECT_FALSE(correctlyConnected(taskGraph, source, node1));
    EXPECT_FALSE(correctlyConnected(taskGraph, node1, sink));
}

TEST(imstkTaskGraphTest, AddRemoveNodesNoEdges)
{
    auto taskGraph = std::make_shared<TaskGraph>();
    auto node1     = std::make_shared<TaskNode>();
    auto node2     = std::make_shared<TaskNode>();

    taskGraph->addNode(node1);
    EXPECT_THAT(taskGraph->getNodes(), UnorderedElementsAre(taskGraph->getSource(), taskGraph->getSink(), node1));

    taskGraph->addNode(node2);
    EXPECT_THAT(taskGraph->getNodes(), UnorderedElementsAre(taskGraph->getSource(), taskGraph->getSink(), node1, node2));

    taskGraph->removeNode(node1);
    EXPECT_THAT(taskGraph->getNodes(), UnorderedElementsAre(taskGraph->getSource(), taskGraph->getSink(), node2));

    taskGraph->removeNode(node2);
    EXPECT_THAT(taskGraph->getNodes(), UnorderedElementsAre(taskGraph->getSource(), taskGraph->getSink()));
}

TEST(imstkTaskGraphTest, AddEdgesAndAdjancency)
{
    auto taskGraph = std::make_shared<TaskGraph>();
    auto node1     = std::make_shared<TaskNode>();
    auto node2     = std::make_shared<TaskNode>();
    auto source    = taskGraph->getSource();
    auto sink      = taskGraph->getSink();

    EXPECT_FALSE(taskGraph->containsEdge(node1, node2));

    taskGraph->addNode(node1);
    taskGraph->addNode(node2);
    taskGraph->addEdge(node1, node2);
    EXPECT_TRUE(correctlyConnected(taskGraph, node1, node2));

    taskGraph->addEdge(source, node1);
    taskGraph->addEdge(node1, sink);
    taskGraph->addEdge(node2, sink);

    EXPECT_TRUE(taskGraph->containsEdge(source, node1));
    EXPECT_TRUE(taskGraph->containsEdge(node1, node2));
    EXPECT_TRUE(taskGraph->containsEdge(node1, sink));
    EXPECT_TRUE(taskGraph->containsEdge(node2, sink));

    EXPECT_THAT(taskGraph->getAdjList().at(source), UnorderedElementsAre(node1));
    EXPECT_THAT(taskGraph->getAdjList().at(node1), UnorderedElementsAre(node2, sink));
    EXPECT_THAT(taskGraph->getAdjList().at(node2), UnorderedElementsAre(sink));
    EXPECT_EQ(0, taskGraph->getAdjList().count(sink));

    EXPECT_EQ(0, taskGraph->getInvAdjList().count(source));
    EXPECT_THAT(taskGraph->getInvAdjList().at(node1), UnorderedElementsAre(source));
    EXPECT_THAT(taskGraph->getInvAdjList().at(node2), UnorderedElementsAre(node1));
    EXPECT_THAT(taskGraph->getInvAdjList().at(sink), UnorderedElementsAre(node1, node2));
}

TEST(imstkTaskGraphTest, RemoveNodesWithEdges)
{
    auto taskGraph = std::make_shared<TaskGraph>();
    auto node1     = std::make_shared<TaskNode>();
    auto node2     = std::make_shared<TaskNode>();
    auto source    = taskGraph->getSource();
    auto sink      = taskGraph->getSink();

    /// source - node1 - node2
    ///               \    |
    ///                sink
    taskGraph->addNode(node1);
    taskGraph->addNode(node2);
    taskGraph->addEdge(node1, node2);
    taskGraph->addEdge(source, node1);
    taskGraph->addEdge(node1, sink);
    taskGraph->addEdge(node2, sink);

    taskGraph->removeNode(node2);
    EXPECT_THAT(taskGraph->getNodes(), UnorderedElementsAre(taskGraph->getSource(), taskGraph->getSink(), node1));
    EXPECT_TRUE(correctlyRemoved(taskGraph, node2));
    EXPECT_TRUE(correctlyConnected(taskGraph, source, node1));
    EXPECT_TRUE(correctlyConnected(taskGraph, node1, sink));
}

TEST(imstkTaskGraphTest, RemoveNodesWithEdgesAndRedirect)
{
    auto node1 = std::make_shared<TaskNode>();
    auto node2 = std::make_shared<TaskNode>();
    auto node3 = std::make_shared<TaskNode>();

    {
        SCOPED_TRACE("One In One Out");
        auto taskGraph = std::make_shared<TaskGraph>();
        auto source    = taskGraph->getSource();
        auto sink      = taskGraph->getSink();

        /// Simple linear connections source - node1 - sink
        taskGraph->addNode(node1);
        taskGraph->addEdge(source, node1);
        taskGraph->addEdge(node1, sink);
        taskGraph->removeNodeAndRedirect(node1);
        EXPECT_THAT(taskGraph->getNodes(), UnorderedElementsAre(taskGraph->getSource(), taskGraph->getSink()));
        EXPECT_TRUE(correctlyConnected(taskGraph, source, sink));
        EXPECT_TRUE(correctlyRemoved(taskGraph, node1));
    }

    {
        SCOPED_TRACE("Fan In");
        auto taskGraph = std::make_shared<TaskGraph>();
        auto source    = taskGraph->getSource();
        auto sink      = taskGraph->getSink();

        /// source - node1 - sink
        ///                /
        ///          node2
        taskGraph->addNode(node1);
        taskGraph->addNode(node2);
        taskGraph->addEdge(source, node1);
        taskGraph->addEdge(node2, node1);
        taskGraph->addEdge(node1, sink);
        taskGraph->removeNodeAndRedirect(node1);
        EXPECT_THAT(taskGraph->getNodes(), UnorderedElementsAre(taskGraph->getSource(), taskGraph->getSink(), node2));
        EXPECT_TRUE(correctlyConnected(taskGraph, source, sink));
        EXPECT_TRUE(correctlyConnected(taskGraph, node2, sink));
        EXPECT_TRUE(correctlyRemoved(taskGraph, node1));
    }

    {
        SCOPED_TRACE("Fan Out");
        auto taskGraph = std::make_shared<TaskGraph>();
        auto source    = taskGraph->getSource();
        auto sink      = taskGraph->getSink();

        taskGraph->addNode(node1);
        taskGraph->addNode(node2);

        // *INDENT-OFF*
        /* source - node1 - sink
                        \
                        node2 */
        // *INDENT-ON*
        taskGraph->addEdge(source, node1);
        taskGraph->addEdge(node1, node2);
        taskGraph->addEdge(node1, sink);
        taskGraph->removeNodeAndRedirect(node1);
        EXPECT_THAT(taskGraph->getNodes(), UnorderedElementsAre(taskGraph->getSource(), taskGraph->getSink(), node2));
        EXPECT_TRUE(correctlyConnected(taskGraph, source, sink));
        EXPECT_TRUE(correctlyConnected(taskGraph, source, node2));
        EXPECT_TRUE(correctlyRemoved(taskGraph, node1));
    }

    {
        SCOPED_TRACE("Fan In/Out");
        auto taskGraph = std::make_shared<TaskGraph>();
        auto source    = taskGraph->getSource();
        auto sink      = taskGraph->getSink();

        taskGraph->addNode(node1);
        taskGraph->addNode(node2);
        taskGraph->addNode(node3);

        // *INDENT-OFF*
        /* source - node1 - sink
                /         \
          node2            node3 */
        // *INDENT-ON*
        taskGraph->addEdge(source, node1);
        taskGraph->addEdge(node2, node1);
        taskGraph->addEdge(node1, sink);
        taskGraph->addEdge(node1, node3);
        taskGraph->removeNodeAndRedirect(node1);
        EXPECT_THAT(taskGraph->getNodes(), UnorderedElementsAre(taskGraph->getSource(), taskGraph->getSink(), node2, node3));
        EXPECT_TRUE(correctlyConnected(taskGraph, source, sink));
        EXPECT_TRUE(correctlyConnected(taskGraph, source, node3));
        EXPECT_TRUE(correctlyConnected(taskGraph, node2, node3));
        EXPECT_TRUE(correctlyConnected(taskGraph, node2, sink));
        EXPECT_TRUE(correctlyRemoved(taskGraph, node1));
    }
}

TEST(imstkTaskGraphTest, InsertBefore)
{
    auto taskGraph = std::make_shared<TaskGraph>();
    auto source    = taskGraph->getSource();
    auto sink      = taskGraph->getSink();
    auto node1     = std::make_shared<TaskNode>();
    auto node2     = std::make_shared<TaskNode>();
    auto node3     = std::make_shared<TaskNode>();

    // Fan in
    taskGraph->addNode(node1);
    taskGraph->addNode(node2);
    // *INDENT-OFF*
    /* source - node1 - sink
                      /
                node2 */
    // *INDENT-ON*
    taskGraph->addEdge(source, node1);
    taskGraph->addEdge(node2, node1);
    taskGraph->addEdge(node1, sink);

    taskGraph->insertBefore(node1, node3);

    EXPECT_THAT(taskGraph->getNodes(), UnorderedElementsAre(taskGraph->getSource(), taskGraph->getSink(), node1, node2, node3));
    EXPECT_TRUE(correctlyConnected(taskGraph, source, node3));
    EXPECT_TRUE(correctlyConnected(taskGraph, node2, node3));
    EXPECT_TRUE(correctlyConnected(taskGraph, node3, node1));
    EXPECT_TRUE(correctlyConnected(taskGraph, node1, sink));
}

TEST(imstkTaskGraphTest, InsertAfter)
{
    auto taskGraph = std::make_shared<TaskGraph>();
    auto source    = taskGraph->getSource();
    auto sink      = taskGraph->getSink();
    auto node1     = std::make_shared<TaskNode>();
    auto node2     = std::make_shared<TaskNode>();
    auto node3     = std::make_shared<TaskNode>();

    // Fan out
    taskGraph->addNode(node1);
    taskGraph->addNode(node2);
    // *INDENT-OFF*
    /* source - node1 - sink
                       \
                        node2 */
    // *INDENT-ON*
    taskGraph->addEdge(source, node1);
    taskGraph->addEdge(node1, node2);
    taskGraph->addEdge(node1, sink);

    taskGraph->insertAfter(node1, node3);

    EXPECT_THAT(taskGraph->getNodes(), UnorderedElementsAre(taskGraph->getSource(), taskGraph->getSink(), node1, node2, node3));
    EXPECT_TRUE(correctlyConnected(taskGraph, source, node1));
    EXPECT_TRUE(correctlyConnected(taskGraph, node1, node3));
    EXPECT_TRUE(correctlyConnected(taskGraph, node3, node2));
    EXPECT_TRUE(correctlyConnected(taskGraph, node3, sink));
}

TEST(imstkTaskGraphTest, AddSubGraph)
{
    auto subGraph   = std::make_shared<TaskGraph>();
    auto innerNode1 = std::make_shared<TaskNode>();
    auto innerNode2 = std::make_shared<TaskNode>();
    {
        auto source = subGraph->getSource();
        auto sink   = subGraph->getSink();

        // *INDENT-OFF*
        /// Diamond Pattern
        /* source - innerNode1 - sink
                 \            /
                   innerNode2 */
        // *INDENT-ON*
        subGraph->addNodes({ innerNode1, innerNode2 });
        subGraph->addEdges({
            { source, innerNode1 },
            { source, innerNode2 },
            { innerNode1, sink },
            { innerNode2, sink },
         });
    }

    auto taskGraph = std::make_shared<TaskGraph>();
    auto source    = taskGraph->getSource();
    auto sink      = taskGraph->getSink();
    auto node1     = std::make_shared<TaskNode>();
    auto node2     = std::make_shared<TaskNode>();
    auto node3     = std::make_shared<TaskNode>();
    taskGraph->addNodes({ node1, node2 });
    // Diamond pattern again
    taskGraph->addEdges({
        { source, node1 },
        { source, node2 },
        { node1, sink },
        { node2, sink },
        });

    /// Expected shape
    /// node1 connected to subgraph->source maintaining subgraph structure, and then
    /// subgraph->sink connecting to node2
    taskGraph->nestGraph(subGraph, node1, node2);
    EXPECT_THAT(taskGraph->getNodes(), UnorderedElementsAre(source, sink, node1, node2,
        innerNode1, innerNode2, subGraph->getSource(), subGraph->getSink()));
    EXPECT_TRUE(correctlyConnected(taskGraph, source, node1));
    EXPECT_TRUE(correctlyConnected(taskGraph, node1, subGraph->getSource()));
    EXPECT_TRUE(correctlyConnected(taskGraph, subGraph->getSource(), innerNode1));
    EXPECT_TRUE(correctlyConnected(taskGraph, subGraph->getSource(), innerNode2));
    EXPECT_TRUE(correctlyConnected(taskGraph, innerNode1, subGraph->getSink()));
    EXPECT_TRUE(correctlyConnected(taskGraph, innerNode2, subGraph->getSink()));
    EXPECT_TRUE(correctlyConnected(taskGraph, subGraph->getSink(), node2));
    EXPECT_TRUE(correctlyConnected(taskGraph, node2, sink));
}

// Static Function Tests
TEST(imstkTaskGraphTest, TopologicalSortTrivial)
{
    auto taskGraph = std::make_shared<TaskGraph>();
    EXPECT_NO_FATAL_FAILURE(TaskGraph::topologicalSort(taskGraph));

    taskGraph->addNode(std::make_shared<TaskNode>());
    EXPECT_NO_FATAL_FAILURE(TaskGraph::topologicalSort(taskGraph));
    auto& nodes = taskGraph->getNodes();

    taskGraph->addEdges({
        { nodes[0], nodes[1] }
        });

    auto           sorted = TaskGraph::topologicalSort(taskGraph);
    TaskNodeVector result;
    std::copy(sorted->begin(), sorted->end(), std::back_inserter(result));
    EXPECT_THAT(result, ElementsAre(nodes[0], nodes[1]));
}
TEST(imstkTaskGraphTest, TopologicalSort)
{
    auto taskGraph = std::make_shared<TaskGraph>();
    taskGraph->addNodes({ std::make_shared<TaskNode>(), std::make_shared<TaskNode>(), std::make_shared<TaskNode>() });
    auto& nodes = taskGraph->getNodes();
    nodes[2]->m_name = "two";
    nodes[3]->m_name = "three";
    nodes[4]->m_name = "four";

    taskGraph->addEdges({
        { nodes[0], nodes[2] },
        { nodes[0], nodes[3] },
        { nodes[2], nodes[4] },
        { nodes[3], nodes[4] },
        { nodes[4], nodes[1] }
        });

    auto           sorted = TaskGraph::topologicalSort(taskGraph);
    TaskNodeVector result;
    EXPECT_EQ(5, sorted->size());
    std::copy(sorted->begin(), sorted->end(), std::back_inserter(result));

    std::string message;
    std::for_each(result.begin(), result.end(), [&message](std::shared_ptr<TaskNode>& node) { message += node->m_name + ", "; });

    EXPECT_EQ(nodes[0], result[0]);

    // The order varies between source, two, three, four, sink
    // and source, three, two, four, sink
    EXPECT_TRUE(nodes[2] == result[1] || nodes[3] == result[1]);
    EXPECT_TRUE(nodes[2] == result[2] || nodes[3] == result[2]);
    EXPECT_NE(result[1], result[2]);
    EXPECT_EQ(nodes[4], result[3]);
    EXPECT_EQ(nodes[1], result[4]);
}

TEST(imstkTaskGraphTest, IsCyclic)
{
    auto node1 = std::make_shared<TaskNode>();
    auto node2 = std::make_shared<TaskNode>();
    auto node3 = std::make_shared<TaskNode>();

    {
        SCOPED_TRACE("Reject Unconnected");
        auto taskGraph = std::make_shared<TaskGraph>();
        EXPECT_FALSE(TaskGraph::isCyclic(taskGraph));
    }

    {
        SCOPED_TRACE("Trivial No Cycle");
        auto taskGraph = std::make_shared<TaskGraph>();
        taskGraph->addEdge(taskGraph->getSource(), taskGraph->getSink());
        EXPECT_FALSE(TaskGraph::isCyclic(taskGraph));
    }

    {
        SCOPED_TRACE("Trivial Cycle");
        auto taskGraph = std::make_shared<TaskGraph>();
        taskGraph->addEdge(taskGraph->getSource(), taskGraph->getSink());
        taskGraph->addEdge(taskGraph->getSink(), taskGraph->getSource());
        EXPECT_TRUE(TaskGraph::isCyclic(taskGraph));
    }

    {
        SCOPED_TRACE("Self Cycle");
        auto taskGraph = std::make_shared<TaskGraph>();
        taskGraph->addEdge(taskGraph->getSource(), taskGraph->getSource()); \
        EXPECT_TRUE(TaskGraph::isCyclic(taskGraph));
    }

    {
        SCOPED_TRACE("Cycle");
        auto taskGraph = std::make_shared<TaskGraph>();
        auto source    = taskGraph->getSource();
        auto sink      = taskGraph->getSink();
        taskGraph->addNodes({ node1, node2, node3 });
        taskGraph->addEdges({
            { source, node1 },
            { node1, node2 },
            { node2, node3 },
            { node3, sink },
            { node3, node1 }
            });
        taskGraph->addEdge(taskGraph->getSource(), taskGraph->getSink());
        taskGraph->addEdge(taskGraph->getSink(), taskGraph->getSource());
        EXPECT_TRUE(TaskGraph::isCyclic(taskGraph));
    }
}

TEST(imstkTaskGraphTest, TransitiveReduce)
{
    auto node1 = std::make_shared<TaskNode>();
    auto node2 = std::make_shared<TaskNode>();
    auto node3 = std::make_shared<TaskNode>();

    {
        SCOPED_TRACE("Trivial, shouldn't remove anything");
        auto taskGraph = std::make_shared<TaskGraph>();
        auto result    = TaskGraph::transitiveReduce(taskGraph);
        EXPECT_THAT(result->getNodes(), ElementsAre(taskGraph->getSource(), taskGraph->getSink()));
    }

    {
        SCOPED_TRACE("Trivial, shouldn't remove anything");
        auto taskGraph = std::make_shared<TaskGraph>();
        auto result    = TaskGraph::transitiveReduce(taskGraph);
        taskGraph->addEdge(taskGraph->getSource(), taskGraph->getSink());
        EXPECT_THAT(result->getNodes(), ElementsAre(taskGraph->getSource(), taskGraph->getSink()));
        EXPECT_TRUE(correctlyConnected(taskGraph, taskGraph->getSource(), taskGraph->getSink()));
    }

    {
        SCOPED_TRACE("Should remove ");
        auto taskGraph = std::make_shared<TaskGraph>();

        taskGraph->addNodes({ node1, node2 });
        taskGraph->addEdges({
            { taskGraph->getSource(), node1 },
            { node1, node2 },
            { node2, taskGraph->getSink() },
            { node1, taskGraph->getSink() }
            }
            );
        auto result = TaskGraph::transitiveReduce(taskGraph);
        EXPECT_THAT(result->getNodes(), ElementsAre(taskGraph->getSource(), taskGraph->getSink(), node1, node2));
        EXPECT_TRUE(correctlyConnected(result, taskGraph->getSource(), node1));
        EXPECT_TRUE(correctlyConnected(result, node1, node2));
        EXPECT_TRUE(correctlyConnected(result, node2, taskGraph->getSink()));
        EXPECT_FALSE(correctlyConnected(result, node1, taskGraph->getSink()));
    }
}

TEST(imstkTaskGraphTest, CriticalNode)
{
    auto taskGraph = std::make_shared<TaskGraph>();
    auto node1     = std::make_shared<TaskNode>();
    node1->m_isCritical = true;
    node1->m_name       = "one";
    auto node2 = std::make_shared<TaskNode>();
    node2->m_isCritical = true;
    node2->m_name       = "two";
    auto node3 = std::make_shared<TaskNode>();
    taskGraph->addNodes({ node1, node2, node3 });
    taskGraph->addEdges({
        { taskGraph->getSource(), node1 },
        { taskGraph->getSource(), node2 },
        { node1, node3 },
        { node2, taskGraph->getSink() },
        { node3, taskGraph->getSink() },
        }
        );

    auto result = TaskGraph::resolveCriticalNodes(taskGraph);
    EXPECT_THAT(result->getNodes(), ElementsAre(taskGraph->getSource(), taskGraph->getSink(), node1, node2, node3));
    EXPECT_TRUE(correctlyConnected(result, node1, node2) || correctlyConnected(result, node2, node1));
}

TEST(imstkTaskGraphTest, RemoveUnusedNodes)
{
    auto node1 = std::make_shared<TaskNode>();
    auto node2 = std::make_shared<TaskNode>();

    {
        SCOPED_TRACE("Remove One");
        auto taskGraph = std::make_shared<TaskGraph>();

        taskGraph->addNode(node1);
        taskGraph->addNode(node2);

        taskGraph->addEdge(taskGraph->getSource(), node1);
        taskGraph->addEdge(node1, taskGraph->getSink());

        auto result = taskGraph->removeUnusedNodes(taskGraph);

        EXPECT_EQ(4, taskGraph->getNodes().size());
        EXPECT_EQ(3, result->getNodes().size());
        EXPECT_THAT(result->getNodes(), UnorderedElementsAre(taskGraph->getSource(), taskGraph->getSink(), node1));
    }
    {
        SCOPED_TRACE("All Nodes connected");
        auto taskGraph = std::make_shared<TaskGraph>();

        taskGraph->addNode(node1);
        taskGraph->addNode(node2);

        taskGraph->addEdge(taskGraph->getSource(), node1);
        taskGraph->addEdge(node1, node2);
        taskGraph->addEdge(node2, taskGraph->getSink());

        auto result = taskGraph->removeUnusedNodes(taskGraph);

        EXPECT_EQ(4, taskGraph->getNodes().size());
        EXPECT_EQ(4, result->getNodes().size());
        EXPECT_THAT(result->getNodes(), UnorderedElementsAre(taskGraph->getSource(), taskGraph->getSink(), node1, node2));
    }
    {
        SCOPED_TRACE("No Connected nodes");
        // Note source and sink are not connected
        auto taskGraph = std::make_shared<TaskGraph>();

        taskGraph->addNode(node1);
        taskGraph->addNode(node2);
        EXPECT_EQ(4, taskGraph->getNodes().size());

        auto result = taskGraph->removeUnusedNodes(taskGraph);
        EXPECT_EQ(0, result->getNodes().size());
    }
}
