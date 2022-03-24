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

#include "imstkTaskNode.h"

#include <list>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace imstk
{
struct TaskNodeHash
{
    std::size_t operator()(const std::shared_ptr<TaskNode>& node) const
    {
        return node->getGlobalId();
    }
};
struct TaskNodeEq
{
    bool operator()(const std::shared_ptr<TaskNode>& a,
                    const std::shared_ptr<TaskNode>& b) const
    {
        return a->getGlobalId() == b->getGlobalId();
    }
};

using TaskNodeVector  = std::vector<std::shared_ptr<TaskNode>>;
using TaskNodeList    = std::list<std::shared_ptr<TaskNode>>;
using TaskNodeSet     = std::unordered_set<std::shared_ptr<TaskNode>, TaskNodeHash, TaskNodeEq>;
using TaskNodeAdjList = std::unordered_map<std::shared_ptr<TaskNode>, TaskNodeSet, TaskNodeHash, TaskNodeEq>;
using TaskNodeNameMap = std::unordered_map<std::shared_ptr<TaskNode>, std::string, TaskNodeHash, TaskNodeEq>;

///
/// \class TaskGraph
///
/// \brief Base class for TaskGraph, a collection of TaskNode's. Maintains nodes,
/// adjacencyList, and invAdjacencyList.
///
class TaskGraph
{
public:
    TaskGraph(std::string sourceName = "Source", std::string sinkName = "Sink");
    virtual ~TaskGraph() = default;

public:
    std::shared_ptr<TaskNode> getSource() const { return m_source; }
    std::shared_ptr<TaskNode> getSink() const { return m_sink; }

    ///
    /// \brief Get the nodes belonging to this graph
    /// HS This is bad, there are algorithms that change the nodes of the graph from the outside
    ///
    TaskNodeVector& getNodes() { return m_nodes; }

    ///
    /// \brief Get the edges belonging to this graph
    ///
    const TaskNodeAdjList& getAdjList() const { return m_adjList; }

    ///
    /// \brief Get the inverse edges belonging to this graph
    ///
    const TaskNodeAdjList& getInvAdjList() const { return m_invAdjList; }

// Node operations
public:
    ///
    /// \brief Linear search for node by name within this graph
    ///
    TaskNodeVector::iterator findNode(std::string name);
    TaskNodeVector::const_iterator findNode(std::string name) const;
    ///
    /// \brief Linear search for node within this graph
    ///
    TaskNodeVector::iterator findNode(std::shared_ptr<TaskNode> node);
    TaskNodeVector::const_iterator findNode(std::shared_ptr<TaskNode> node) const;

    ///
    /// \brief Check if node exists in this graph
    ///
    bool containsNode(std::shared_ptr<TaskNode> node) const;

    ///
    /// \brief Returns the start of the node container
    ///
    TaskNodeVector::iterator endNode() { return m_nodes.end(); }
    TaskNodeVector::const_iterator endNode() const { return m_nodes.end(); }
    ///
    /// \brief Returns the start of the node container
    ///
    TaskNodeVector::iterator beginNode() { return m_nodes.begin(); }
    TaskNodeVector::const_iterator beginNode() const { return m_nodes.begin(); }

    ///
    /// \brief Adds a node to the graph, returns if successful.
    /// Returns false and fails if node already exists in graph
    ///
    bool addNode(std::shared_ptr<TaskNode> node);

    ///
    /// \brief Adds a series of nodes at the same time
    /// Use with {} initializer for easier graph construction
    void addNodes(const std::vector<std::shared_ptr<TaskNode>>& nodes);

    ///
    /// \brief Creates a node for the function, adds it to the graph
    ///
    std::shared_ptr<TaskNode> addFunction(std::string name, std::function<void()> func);

    ///
    /// \brief Removes node from the graph and all relevant edges
    /// Returns false and fails if node is not present in graph
    ///
    bool removeNode(std::shared_ptr<TaskNode> node);

    ///
    /// \brief Removes node from the graph and all relevant edges.
    /// Any incoming edges to the node are redirected to all its outputs
    ///
    bool removeNodeAndRedirect(std::shared_ptr<TaskNode> node);

    ///
    /// \brief newNode gets placed after refNode & added to the graph. newNode takes on refNode's outputs
    ///
    void insertAfter(std::shared_ptr<TaskNode> refNode, std::shared_ptr<TaskNode> newNode);

    ///
    /// \brief newNode gets placed before refNode & added to the graph. newNode takes on refNode's inputs
    ///
    void insertBefore(std::shared_ptr<TaskNode> refNode, std::shared_ptr<TaskNode> newNode);

// Edge operations
public:
    ///
    /// \brief Returns whether or not this graph contains the given directed edge
    ///
    bool containsEdge(std::shared_ptr<TaskNode> srcNode, std::shared_ptr<TaskNode> destNode) const;

    ///
    /// \brief Adds a directed edge to the graph both the source and target have to exist in the graph
    ///
    void addEdge(std::shared_ptr<TaskNode> srcNode, std::shared_ptr<TaskNode> destNode);

    /// \brief Adds a series of directed edges {source, target} to the graph
    /// both the source and target of the edge have to exist in the graph
    ///
    void addEdges(const std::vector<std::pair<std::shared_ptr<TaskNode>, std::shared_ptr<TaskNode>>>& edges);

    ///
    /// \brief Attaches another TaskGraph as a subgraph (copies nodes and edges, then connects source->subgraph::source, subgraph::sink->sink),
    /// source and sink must exist in this graph. Also serves as a graph sum between this and subgraph
    ///
    void nestGraph(std::shared_ptr<TaskGraph> subgraph, std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink);

    ///
    /// \brief Removes an edge from the graph (removes from adjList and invAdjList, cleans)
    ///
    void removeEdge(std::shared_ptr<TaskNode> srcNode, std::shared_ptr<TaskNode> destNode);

    ///
    /// \brief Returns true if srcNode reaches destNode
    ///
    bool isReachable(std::shared_ptr<TaskNode> srcNode, std::shared_ptr<TaskNode> destNode);

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
    //static std::shared_ptr<TaskGraph> sum(std::shared_ptr<TaskGraph> graphA, std::shared_ptr<TaskGraph> graphB);

    ///
    /// \brief Topological sort of all nodes within graph
    ///
    static std::shared_ptr<TaskNodeList> topologicalSort(std::shared_ptr<TaskGraph> graph);

    ///
    /// \brief Remove redundant edges. Removal is such that all vertices are still reachable and graph goes from source->sink
    /// returns nullptr if failed. Only fails if graph is cyclic.
    ///
    static std::shared_ptr<TaskGraph> transitiveReduce(std::shared_ptr<TaskGraph> graph);

    ///
    /// \brief Removes nullptr/nonfunctional TaskNode's that don't split/join
    ///
    static std::shared_ptr<TaskGraph> removeRedundantNodes(std::shared_ptr<TaskGraph> graph);

    ///
    /// \brief Simplifies the graph in a way that retains functionality
    /// Performs transitive reduction then redundant node removal to remove redundant nodes and edges
    ///
    static std::shared_ptr<TaskGraph> reduce(std::shared_ptr<TaskGraph> graph)
    {
        std::shared_ptr<TaskGraph> reducedGraph = transitiveReduce(graph);
        if (reducedGraph != nullptr)
        {
            return removeRedundantNodes(reducedGraph);
        }
        else
        {
            return nullptr;
        }
    }

    static std::shared_ptr<TaskGraph> removeUnusedNodes(std::shared_ptr<TaskGraph> graph);

    ///
    /// \brief Returns if Graph is cyclic or not
    ///
    static bool isCyclic(std::shared_ptr<TaskGraph> graph);

    ///
    /// \brief Nodes may not have unique names. Iterates the names with numeric postfix to generate uniques
    ///
    static TaskNodeNameMap getUniqueNodeNames(std::shared_ptr<TaskGraph> graph, bool apply = false);

    ///
    /// \brief Gets the completion times of each node (source = ~0s)
    ///
    static std::unordered_map<std::shared_ptr<TaskNode>, double, TaskNodeHash, TaskNodeEq> getNodeStartTimes(std::shared_ptr<TaskGraph> graph);

    ///
    /// \brief Computes the critical path
    ///
    static TaskNodeList getCriticalPath(std::shared_ptr<TaskGraph> graph);

protected:
    TaskNodeVector  m_nodes;
    TaskNodeAdjList m_adjList;    ///> This gives the outputs of every node
    TaskNodeAdjList m_invAdjList; ///> This gives the inputs of every node

    std::shared_ptr<TaskNode> m_source = nullptr;
    std::shared_ptr<TaskNode> m_sink   = nullptr;
};
} // namespace imstk
