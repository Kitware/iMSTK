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

#include "imstkTaskGraphVizWriter.h"
#include "imstkLogger.h"
#include "imstkTaskGraph.h"
#include "imstkColor.h"

namespace imstk
{
void
TaskGraphVizWriter::write()
{
    if (m_inputGraph == nullptr)
    {
        LOG(WARNING) << "No input set, unable to write ComputeGraph";
        return;
    }

    // Compute range of compute times for color function
    double maxTime = std::numeric_limits<double>::min();
    if (m_writeNodeComputeTimesColor)
    {
        for (auto node : m_inputGraph->getNodes())
        {
            if (node->m_computeTime > maxTime)
            {
                maxTime = node->m_computeTime;
            }
        }
    }

    // Hardcoded color function
    std::vector<Color> colorFunc = std::vector<Color>(3);
    colorFunc[0] = Color::Blue;
    colorFunc[1] = Color::Green;
    colorFunc[2] = Color::Red;
    const int colorFuncExtent = static_cast<int>(colorFunc.size() - 1);

    // Compute the critical path (ie: longest path in duration)
    TaskNodeList critPath;
    if (m_highlightCriticalPath)
    {
        critPath = TaskGraph::getCriticalPath(m_inputGraph);
    }
    // Test if edge exists in critical path by linear searching
    auto edgeExists = [&](const std::shared_ptr<TaskNode>& a, const std::shared_ptr<TaskNode>& b)
                      {
                          TaskNodeList::iterator srcNode = std::find(critPath.begin(), critPath.end(), a);
                          // If srcNode was found and the next node is b
                          return (srcNode != critPath.end() && *std::next(srcNode) == b);
                      };

    // Write the file
    {
        std::ofstream file;
        file.open(m_fileName);

        if (!file.is_open() || file.fail())
        {
            return;
        }

        file <<
            "digraph imstkTaskGraph\n"
            "{\n"
            "style=filled;\n"
            "color=lightgrey;\n"
            "edge[arrowhead=vee, arrowtail=inv, arrowsize=.7, color=grey20];\n";

        // Write the node section
        const TaskNodeVector&                                      nodes = m_inputGraph->getNodes();
        std::unordered_map<std::shared_ptr<TaskNode>, std::string> nodeIds;
        for (size_t i = 0; i < nodes.size(); i++)
        {
            const std::string nodeUniqueName = "node" + std::to_string(i);
            nodeIds[nodes[i]] = nodeUniqueName;

            file << "\"" << nodeUniqueName << "\" [";

            // Write label property
            if (m_writeNodeComputeTimesText)
            {
                file << " label=\"" << nodes[i]->m_name << " (" << nodes[i]->m_computeTime << "ms)" << "\"";
            }
            else
            {
                file << " label=\"" << nodes[i]->m_name << '\"';
            }

            // Write style property
            file << " style=filled";

            // Write color property
            if (m_writeNodeComputeTimesColor)
            {
                const double t     = nodes[i]->m_computeTime / maxTime;
                const int    i1    = static_cast<int>(t * colorFuncExtent);
                const int    i2    = std::min(colorFuncExtent, i1 + 1);
                Color        color = Color::lerpRgb(colorFunc[i1], colorFunc[i2], t);
                file << " color=\"#" << color.rgbHex() << "\"";
            }
            else
            {
                if (nodes[i]->m_isCritical)
                {
                    file << " color=\"#8B2610\"";
                }
                else
                {
                    file << " color=cornflowerblue";
                }
            }
            file << "];" << std::endl;
        }

        // Write out all the edges
        const TaskNodeAdjList& adjList = m_inputGraph->getAdjList();
        for (TaskNodeAdjList::const_iterator it = adjList.begin(); it != adjList.end(); it++)
        {
            std::shared_ptr<TaskNode> srcNode     = it->first;
            const TaskNodeSet&        outputNodes = it->second;
            for (TaskNodeSet::const_iterator jt = outputNodes.begin(); jt != outputNodes.end(); jt++)
            {
                std::shared_ptr<TaskNode> destNode = *jt;
                file << '\"' << nodeIds[srcNode] << '\"' << " -> " << '\"' << nodeIds[destNode] << '\"';

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
} // namespace imstk