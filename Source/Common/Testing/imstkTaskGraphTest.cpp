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

#include "imstkTaskGraph.h"

using namespace imstk;

TEST(imstkTaskGraphTest, removeUnusedNodes_)
{
    auto node1 = std::make_shared<TaskNode>();
    auto node2 = std::make_shared<TaskNode>();

    auto taskGraph = std::make_shared<TaskGraph>();

    taskGraph->addNode(node1);
    taskGraph->addNode(node2);

    taskGraph->addEdge(taskGraph->getSource(), node1);
    taskGraph->addEdge(node1, taskGraph->getSink());

    EXPECT_EQ(4, taskGraph->getNodes().size());
    EXPECT_EQ(3, taskGraph->removeUnusedNodes(taskGraph)->getNodes().size());
    EXPECT_EQ(node1, taskGraph->getNodes()[2]);// test if the left out node is the one that is wired
}

