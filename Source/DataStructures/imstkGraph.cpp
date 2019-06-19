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

#include "imstkGraph.h"

namespace imstk
{
Graph::Graph(vector<Edge> edges)
{
    // add edges to the undirected graph
    for (size_t i = 0; i < edges.size(); i++)
    {
        size_t src = edges[i].src;
        size_t dest = edges[i].dest;

        m_adjList[src].push_back(dest);
        m_adjList[dest].push_back(src);
    }
}

void
Graph::addEdge(const size_t v, const size_t w)
{
    if (v < m_adjList.size() && w < m_adjList.size())
    {
        m_adjList[v].push_back(w);
        m_adjList[w].push_back(v);
    }
    else
    {
        cout << "Vertex id exceeds the graph size: cannot add edge!" << endl;
    }
}

void
Graph::print() const
{
    cout << "Graph: " << "\nTotal nodes: " << m_adjList.size() << "\nAdjacency:" << endl;
    for (size_t i = 0; i < m_adjList.size(); i++)
    {
        cout << "\t[" << i << "] : ";

        for (auto v : m_adjList[i])
        {
            cout << v << " ";
        }
        cout << endl;
    }
}

vector<size_t>
Graph::doGreedyColoring(bool print /*= false*/) const
{
    const auto numNodes = m_adjList.size();
    vector<size_t> result(numNodes, -1);
    vector<bool> available(numNodes, false);

    result[0] = 0;

    // Assign colors to remaining V-1 vertices
    for (auto u = 1; u < numNodes; ++u)
    {
        // Process all adjacent vertices and flag their colors
        // as unavailable
        for (const auto& i : m_adjList[u])
        {
            if (result[i] != -1)
            {
                available[result[i]] = true;
            }
        }

        // Find the first available color
        int cr;
        for (cr = 0; cr < numNodes; cr++)
        {
            if (!available[cr])
            {
                break;
            }
        }
        result[u] = cr; // Assign the found color

        // Reset the values back to false for the next iteration
        for (const auto& i : m_adjList[u])
        {
            if (result[i] != -1)
            {
                available[result[i]] = false;
            }
        }
    }

    // print the result
    if (print)
    {
        std::cout << "Num. of colors: " << *max_element(begin(result), end(result)) + 1 << std::endl;
        for (auto i = 0; i < numNodes; ++i)
        {
            std::cout << "V " << i << "-C " << result[i] << " | ";
        }
        std::cout << std::endl;
    }

    return std::move(result);
}
}