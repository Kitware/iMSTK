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

TEST(imstkTaskGraphTest, Constructor)
{
    auto taskGraph = std::make_shared<TaskGraph>();
    EXPECT_EQ(2, taskGraph->getNodes().size());
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
