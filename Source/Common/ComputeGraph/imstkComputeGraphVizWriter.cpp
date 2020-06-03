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

#include "imstkComputeGraphVizWriter.h"
#include "imstkComputeGraph.h"
#include "imstkLogger.h"
#include <fstream>

namespace imstk
{
void
ComputeGraphVizWriter::write()
{
    if (m_inputGraph == nullptr)
    {
        LOG(WARNING) << "No input set, unable to write ComputeGraph";
        return;
    }

    // This little writer fails when nodes are named the same, some more advanced
    // method would have to be used to rename nodes that are the same with consistent
    // numbering down the tree
    std::ofstream file;
    file.open(m_fileName);

    if (!file.is_open() || file.fail())
    {
        return;
    }

    file <<
        "digraph imstkDependency\n"
        "{\n"
        "style=filled;\n"
        "color=lightgrey;\n"
        "node [style=filled,color=cornflowerblue];\n"
        "edge[arrowhead=vee, arrowtail=inv, arrowsize=.7, color=grey20]\n";

    // Ensure unique names
    std::unordered_map<std::shared_ptr<ComputeNode>, std::string> nodeNames = ComputeGraph::getUniqueNames(m_inputGraph);

    // Compute the critical path (ie: longest path in duration)
    ComputeNodeList critPath;
    if (m_highlightCriticalPath)
    {
        critPath = ComputeGraph::getCriticalPath(m_inputGraph);
    }
    auto edgeExists = [&](const std::shared_ptr<ComputeNode>& a, const std::shared_ptr<ComputeNode>& b)
                      {
                          ComputeNodeList::iterator srcNode = std::find(critPath.begin(), critPath.end(), a);
                          // If srcNode was found and the next node is b
                          return (srcNode != critPath.end() && *std::next(srcNode) == b);
                      };

    // Compute completion times of each node
    if (m_writeTimes)
    {
        std::unordered_map<std::shared_ptr<ComputeNode>, double> times = ComputeGraph::getTimes(m_inputGraph);
        for (auto i : nodeNames)
        {
            nodeNames[i.first] = i.second + "(" + std::to_string(times[i.first]) + "ms)";
        }
    }

    // Write out all the edges
    const ComputeNodeAdjList& adjList = m_inputGraph->getAdjList();
    for (ComputeNodeAdjList::const_iterator it = adjList.begin(); it != adjList.end(); it++)
    {
        std::shared_ptr<ComputeNode> srcNode     = it->first;
        const ComputeNodeSet&        outputNodes = it->second;
        for (ComputeNodeSet::const_iterator jt = outputNodes.begin(); jt != outputNodes.end(); jt++)
        {
            std::shared_ptr<ComputeNode> destNode = *jt;
            file << '\"' << nodeNames[srcNode] << '\"' << " -> " << '\"' << nodeNames[destNode] << '\"';

            if (m_highlightCriticalPath && edgeExists(srcNode, destNode))
            {
                file << "[color=red]";
            }
            file << std::endl;
        }
    }
    file << "}\n";
    file.close();
}
}