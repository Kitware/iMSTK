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

#include <unordered_set>
#include <iostream>
#include <algorithm>

#include "imstkMath.h"

namespace imstk
{
///
/// \brief class to represent a graph object
///
class Graph
{
public:
    enum class ColoringMethod
    {
        Greedy,
        WelshPowell
    };

    ///
    /// \brief Constructor/destructor
    ///
    Graph(const size_t size){ m_adjList.resize(size); }
    ~Graph() = default;

    ///
    /// \brief Add edge to the graph
    ///
    void addEdge(const size_t v, const size_t w);

    ///
    /// \brief Get size of the graph
    ///
    size_t size() const { return m_adjList.size(); }

    ///
    /// \brief print adjacency list representation of graph
    ///
    void print() const;

    ///
    /// \brief Set the default colorizing method
    ///
    void setDefaultColoringMethod(ColoringMethod method) { m_ColoringMethod = method; }

    ///
    /// \brief Colorize using the default method and prints the assignment of colors
    /// \return Vertex colors and number of colors
    ///
    std::pair<std::vector<unsigned short>, unsigned short>
    doColoring(bool print = false) const { return doColoring(m_ColoringMethod, print); }

    ///
    /// \brief Colorize using the given method and prints the assignment of colors
    /// \return Vertex colors and number of colors
    ///
    std::pair<std::vector<unsigned short>, unsigned short>
    doColoring(ColoringMethod method, bool print = false) const;

protected:
    ///
    /// \brief Colorize using greedy algorithm and print the assignment of colors
    /// \return Vertex colors and number of colors
    ///
    std::pair<std::vector<unsigned short>, unsigned short>
    doColoringGreedy(bool print = false) const;

    ///
    /// \brief Colorize using Welsh-Powell algorithm and print the assignment of colors
    /// \return Vertex colors and number of colors
    ///
    std::pair<std::vector<unsigned short>, unsigned short>
    doColoringWelshPowell(bool print = false) const;

    std::vector<std::unordered_set<size_t>> m_adjList;    ///< A array of std::vectors to represent adjacency list
    ColoringMethod m_ColoringMethod = ColoringMethod::WelshPowell;
};
}

#endif //imstkGraph_h
