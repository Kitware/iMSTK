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

#include "imstkTbbTaskGraphController.h"
#include "imstkMacros.h"
#include "imstkTaskGraph.h"

DISABLE_WARNING_PUSH
    DISABLE_WARNING_PADDING
#include <tbb/flow_graph.h>
DISABLE_WARNING_POP

using namespace tbb::flow;

namespace imstk
{
void
TbbTaskGraphController::execute()
{
    using TbbContinueNode = continue_node<continue_msg>;

    graph g;

    broadcast_node<continue_msg> start(g);

    // Create a continue node for every TaskNode (except start)
    std::unordered_map<std::shared_ptr<TaskNode>, TbbContinueNode> tbbNodes;
    using NodeKeyValuePair = std::pair<std::shared_ptr<TaskNode>, TbbContinueNode>;

    const TaskNodeVector& nodes = m_graph->getNodes();
    if (nodes.size() == 0)
    {
        return;
    }
    tbbNodes.reserve(nodes.size());
    for (size_t i = 0; i < nodes.size(); i++)
    {
        if (m_graph->getSource() != nodes[i])
        {
            std::shared_ptr<TaskNode> node = nodes[i];
            tbbNodes.insert(NodeKeyValuePair(node, TbbContinueNode(g,
                [node](continue_msg) { node->execute(); })));
        }
    }

    const TaskNodeAdjList& adjList = m_graph->getAdjList();
    for (const auto& i : adjList)
    {
        TbbContinueNode& tbbNode1 = tbbNodes.at(i.first);
        for (const auto& outputNode : i.second)
        {
            TbbContinueNode& tbbNode2 = tbbNodes.at(outputNode);
            make_edge(tbbNode1, tbbNode2);
        }
    }

    start.try_put(continue_msg());
    g.wait_for_all();
}
} // namespace imstk