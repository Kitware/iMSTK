/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

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
