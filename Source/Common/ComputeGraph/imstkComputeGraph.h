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

#include "imstkComputeNode.h"

#include <list>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace imstk
{
using ComputeNodeVector  = std::vector<std::shared_ptr<ComputeNode>>;
using ComputeNodeList    = std::list<std::shared_ptr<ComputeNode>>;
using ComputeNodeSet     = std::unordered_set<std::shared_ptr<ComputeNode>>;
using ComputeNodeAdjList = std::unordered_map<std::shared_ptr<ComputeNode>, ComputeNodeSet>;

///
/// \brief Base class for compute graphs, a collection of computational nodes
/// edges can exist between graphs. Maintains nodes, adjList, and invAdjList
/// TODO: move algorithms out of the container like STL
///
class ComputeGraph
{
public:
    ComputeGraph(std::string sourceName = "Source", std::string sinkName = "Sink");
    virtual ~ComputeGraph() = default;

public:
    std::shared_ptr<ComputeNode> getSource() { return m_source; }
    std::shared_ptr<ComputeNode> getSink() { return m_sink; }

    ///
    /// \brief Get the nodes belonging to this graph
    ///
    ComputeNodeVector& getNodes() { return m_nodes; }

    ///
    /// \brief Get the edges belonging to this graph
    ///
    ComputeNodeAdjList& getAdjList() { return m_adjList; }

    ///
    /// \brief Get the inverse edges belonging to this graph
    ///
    ComputeNodeAdjList& getInvAdjList() { return m_invAdjList; }

// Node operations
public:
    ///
    /// \brief Linear search for node by name within this graph
    ///
    ComputeNodeVector::iterator findNode(std::string name);
    ComputeNodeVector::const_iterator findNode(std::string name) const;
    ///
    /// \brief Linear search for node within this graph
    ///
    ComputeNodeVector::iterator findNode(std::shared_ptr<ComputeNode> node);
    ComputeNodeVector::const_iterator findNode(std::shared_ptr<ComputeNode> node) const;

    ///
    /// \brief Check if node exists in this graph
    ///
    bool containsNode(std::shared_ptr<ComputeNode> node) const;

    ///
    /// \brief Returns the start of the node container
    ///
    ComputeNodeVector::iterator nodesEnd() { return m_nodes.end(); }
    ComputeNodeVector::const_iterator nodesEnd() const { return m_nodes.end(); }
    ///
    /// \brief Returns the start of the node container
    ///
    ComputeNodeVector::iterator nodesBegin() { return m_nodes.begin(); }
    ComputeNodeVector::const_iterator nodesBegin() const { return m_nodes.begin(); }

    ///
    /// \brief Adds a node to the graph, returns if successful.
    ///  Returns false and fails if node already exists in graph
    ///
    bool addNode(std::shared_ptr<ComputeNode> node);

    ///
    /// \brief Creates a node for the function, adds it to the graph
    ///
    std::shared_ptr<ComputeNode> addFunction(std::string name, std::function<void()> func);

    ///
    /// \brief Removes node from the graph and all relevant edges
    /// Returns false and fails if node is not present in graph
    /// "Expensive" operation as no inverse adjacency list is maintained
    /// full search done for relevant edges
    ///
    bool removeNode(std::shared_ptr<ComputeNode> node);

    ///
    /// \brief Removes node from the graph and all relevant edges
    /// any incoming edges to the node are redirected to all its outputs
    ///
    bool removeNodeAndFix(std::shared_ptr<ComputeNode> node);

    ///
    /// \brief newNode gets placed after refNode & added to the graph. newNode takes on refNode's outputs
    ///
    void insertAfter(std::shared_ptr<ComputeNode> refNode, std::shared_ptr<ComputeNode> newNode);

    ///
    /// \brief newNode gets placed before refNode & added to the graph. newNode takes on refNode's inputs
    ///
    void insertBefore(std::shared_ptr<ComputeNode> refNode, std::shared_ptr<ComputeNode> newNode);

// Edge operations
public:
    ///
    /// \brief Returns whether or not this graph contains the given directed edge
    ///
    bool containsEdge(std::shared_ptr<ComputeNode> srcNode, std::shared_ptr<ComputeNode> destNode);

    ///
    /// \brief Adds a directed edge to the graph (doesn't check if nodes are in graph)
    ///
    void addEdge(std::shared_ptr<ComputeNode> srcNode, std::shared_ptr<ComputeNode> destNode);

    ///
    /// \brief Attachs another ComputeGraph as a subgraph (copies nodes and edges, then connects source->subgraph::source, subgraph::sink->sink),
    /// source and sink must exist in this graph. Also serves as a graph sum between this and subgraph
    ///
    void nestGraph(std::shared_ptr<ComputeGraph> subgraph, std::shared_ptr<ComputeNode> source, std::shared_ptr<ComputeNode> sink);

    ///
    /// \brief Removes an edge from the graph (removes from adjList and invAdjList, cleans)
    ///
    void removeEdge(std::shared_ptr<ComputeNode> srcNode, std::shared_ptr<ComputeNode> destNode);

    ///
    /// \brief Returns true if srcNode reaches destNode
    ///
    bool reaches(std::shared_ptr<ComputeNode> srcNode, std::shared_ptr<ComputeNode> destNode);

public:
    ///
    /// \brief Removes all nodes & edges from the graph. Source and sink nodes maintained.
    ///
    void clear();

    ///
    /// \brief Removes all edges from the graph
    ///
    void clearEdges()
    {
        m_adjList.clear();
        m_invAdjList.clear();
    }

// Graph algorithms, todo: Move into filtering module
public:
    ///
    /// \brief Graph sum, shared references are considered identical nodes, source/sink of results invalidated/nullptr
    ///
    static std::shared_ptr<ComputeGraph> sum(std::shared_ptr<ComputeGraph> graphA, std::shared_ptr<ComputeGraph> graphB);

    ///
    /// \brief Topological sort of all nodes within graph
    ///
    static std::shared_ptr<ComputeNodeList> topologicalSort(std::shared_ptr<ComputeGraph> graph);

    ///
    /// \brief Makes sure no two *critical nodes* run at the same time by establishing an edge between them.
    ///
    static std::shared_ptr<ComputeGraph> resolveCriticalNodes(std::shared_ptr<ComputeGraph> graph);

    ///
    /// \brief Remove redudant edges. Removal is such that all vertices are still reachable and graph goes from source->sink
    /// returns nullptr if failed. Only fails if graph is cyclic.
    ///
    static std::shared_ptr<ComputeGraph> transitiveReduce(std::shared_ptr<ComputeGraph> graph);

    ///
    /// \brief Removes nullptr/nonfunctional ComputeNodes that don't split/join
    ///
    static std::shared_ptr<ComputeGraph> nonFunctionalPrune(std::shared_ptr<ComputeGraph> graph);

    ///
    /// \brief Simplifies the graph in a way that retains functionality
    /// Performs Nonfunctional Prune and Transitive Reduction removing redundant edges and non functional nodes
    ///
    static std::shared_ptr<ComputeGraph> reduce(std::shared_ptr<ComputeGraph> graph)
    {
        return nonFunctionalPrune(transitiveReduce(graph));
    }

    ///
    /// \brief Returns if Graph is cyclic or not
    ///
    static bool isCyclic(std::shared_ptr<ComputeGraph> graph);

    ///
    /// \brief Iterates the names with numeric postfix to generate uniques
    ///
    static std::unordered_map<std::shared_ptr<ComputeNode>, std::string> getUniqueNames(std::shared_ptr<ComputeGraph> graph, bool apply = false);

    ///
    /// \brief Gets the completion times of each node (source = ~0s)
    ///
    static std::unordered_map<std::shared_ptr<ComputeNode>, double> getTimes(std::shared_ptr<ComputeGraph> graph);

    ///
    /// \brief Computes the critical path
    ///
    static ComputeNodeList getCriticalPath(std::shared_ptr<ComputeGraph> graph);

protected:
    ComputeNodeVector  m_nodes;
    ComputeNodeAdjList m_adjList;    ///> This gives the outputs of every node
    ComputeNodeAdjList m_invAdjList; ///> This gives the inputs of every node
    // inv adjlist?

    std::shared_ptr<ComputeNode> m_source = nullptr;
    std::shared_ptr<ComputeNode> m_sink   = nullptr;
};
}