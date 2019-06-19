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

#ifndef imstkGraph_h
#define imstkGraph_h

#include <iostream>
#include <algorithm>

#include "imstkMath.h"

namespace imstk
{
using namespace std;

///
/// \brief data structure to store graph edges
///
struct Edge
{
    size_t src, dest;
};

///
/// \brief class to represent a graph object
///
class Graph
{
public:
    ///
    /// \brief Constructor/destructor
    ///
    Graph(const size_t size){ m_adjList.resize(size); };
    Graph(vector<Edge> edges);
    ~Graph() = default;

    ///
    /// \brief Add edge to the graph
    ///
    void addEdge(const size_t v, const size_t w);

    ///
    /// \brief print adjacency list representation of graph
    ///
    void print() const;

    ///
    /// \brief Assigns colors (starting from 0) to all vertices and prints
    /// the assignment of colors
    ///
    vector<size_t> doGreedyColoring(bool print = false) const;

protected:

    vector<vector<size_t>> m_adjList;    ///< A array of vectors to represent adjacency list
};
}

#endif //imstkGraph_h
