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

#include "imstkTbbComputeGraphController.h"
#include "imstkComputeGraph.h"
#include <tbb/tbb.h>

namespace imstk
{
class ComputeNodeTask : public tbb::task
{
public:
    ComputeNodeTask(std::shared_ptr<ComputeNode> node) : m_node(node) { }

public:
    task* execute() override
    {
        __TBB_ASSERT(ref_count() == 0, NULL);

        m_node->execute();

        for (size_t i = 0; i < successors.size(); i++)
        {
            if (successors[i]->decrement_ref_count() == 0)
            {
                spawn(*successors[i]);
            }
        }
        return NULL;
    }

public:
    std::shared_ptr<ComputeNode>  m_node = nullptr;
    std::vector<ComputeNodeTask*> successors;
};

void
TbbComputeGraphController::execute()
{
    // Create a Task for every node
    const ComputeNodeVector& nodes = m_graph->getNodes();
    if (nodes.size() == 0)
    {
        return;
    }

    // Create a task for every node
    std::unordered_map<std::shared_ptr<ComputeNode>, ComputeNodeTask*> tasks;
    tasks.reserve(nodes.size());

    for (size_t i = 0; i < nodes.size(); i++)
    {
        std::shared_ptr<ComputeNode> node = nodes[i];
        tasks[node] = new (tbb::task::allocate_root())ComputeNodeTask(node);
    }
    // Increment successor reference counts
    const ComputeNodeAdjList& adjList = m_graph->getAdjList();
    // For every node in graph
    for (size_t i = 0; i < nodes.size(); i++)
    {
        // If it contains outputs
        if (adjList.count(nodes[i]) != 0)
        {
            // For every output
            const ComputeNodeSet& outputNodes = adjList.at(nodes[i]);
            for (ComputeNodeSet::const_iterator it = outputNodes.begin(); it != outputNodes.end(); it++)
            {
                // Lookup the task of the node
                ComputeNodeTask* successor = tasks[*it];
                tasks[nodes[i]]->successors.push_back(successor);
                successor->increment_ref_count();
            }
        }
    }

    ComputeNodeTask* startTask = tasks[m_graph->getSource()];
    ComputeNodeTask* finalTask = tasks[m_graph->getSink()];
    // Extra ref count on the final task
    finalTask->increment_ref_count();
    finalTask->spawn_and_wait_for_all(*startTask);
    finalTask->execute();     // Execute final task explicitly
    tbb::task::destroy(*finalTask);
}
}