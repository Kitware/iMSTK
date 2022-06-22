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

#pragma once

#include <cstdlib>
#include <unordered_set>
#include <vector>

namespace imstk
{
///
/// \brief class to represent a graph object
///
class Graph
{
using edgeType = std::unordered_set<size_t>;
using graphColorsType = std::pair<std::vector<unsigned short>, unsigned short>;
public:
    enum class ColoringMethod
    {
        Greedy,
        WelshPowell
    };

    Graph(const size_t size) { m_adjList.resize(size); }
    virtual ~Graph() = default;

    ///
    /// \brief Add edge to the graph
    ///
    void addEdge(const size_t v, const size_t w);

    ///
    /// \brief Get edges surrounding a node
    ///
    void getEdges(const size_t v, edgeType& edges) const;

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
    /// \brief Colorize using the given method and prints the assignment of colors
    /// \return Vertex colors and number of colors
    ///
    graphColorsType doColoring(ColoringMethod method = ColoringMethod::WelshPowell,
                               bool           print = false) const;

protected:
    ///
    /// \brief Colorize using greedy algorithm and print the assignment of colors
    /// \return Vertex colors and number of colors
    ///
    graphColorsType doColoringGreedy(bool print = false) const;

    ///
    /// \brief Colorize using Welsh-Powell algorithm and print the assignment of colors
    /// \return Vertex colors and number of colors
    ///
    graphColorsType doColoringWelshPowell(bool print = false) const;

    std::vector<edgeType> m_adjList;    ///< A array of std::vectors to represent adjacency list
    ColoringMethod m_ColoringMethod = ColoringMethod::WelshPowell;
};
} // namespace imstk
