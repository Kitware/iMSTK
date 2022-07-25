/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

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
        if (i.first == m_graph->getSource())
        {
            for (const auto& outputNode : i.second)
            {
                TbbContinueNode& tbbNode2 = tbbNodes.at(outputNode);
                make_edge(start, tbbNode2);
            }
        }
        else
        {
            TbbContinueNode& tbbNode1 = tbbNodes.at(i.first);
            for (const auto& outputNode : i.second)
            {
                TbbContinueNode& tbbNode2 = tbbNodes.at(outputNode);
                make_edge(tbbNode1, tbbNode2);
            }
        }
    }

    start.try_put(continue_msg());
    g.wait_for_all();
}
} // namespace imstk