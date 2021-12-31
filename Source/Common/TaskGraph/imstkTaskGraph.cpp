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

#include "imstkTaskGraph.h"
#include "imstkLogger.h"

#include <stack>

namespace imstk
{
TaskGraph::TaskGraph(std::string sourceName, std::string sinkName) :
    m_source(std::make_shared<TaskNode>()),
    m_sink(std::make_shared<TaskNode>())
{
    m_source->m_name = sourceName;
    m_sink->m_name   = sinkName;
    addNode(m_source);
    addNode(m_sink);
}

TaskNodeVector::iterator
TaskGraph::findNode(std::string name)
{
    return std::find_if(m_nodes.begin(), m_nodes.end(),
        [&name](const std::shared_ptr<TaskNode>& x) { return x->m_name == name; });
}

TaskNodeVector::const_iterator
TaskGraph::findNode(std::string name) const
{
    return std::find_if(m_nodes.begin(), m_nodes.end(),
        [&name](const std::shared_ptr<TaskNode>& x) { return x->m_name == name; });
}

TaskNodeVector::iterator
TaskGraph::findNode(std::shared_ptr<TaskNode> node)
{
    return std::find(m_nodes.begin(), m_nodes.end(), node);
}

TaskNodeVector::const_iterator
TaskGraph::findNode(std::shared_ptr<TaskNode> node) const
{
    return std::find(m_nodes.begin(), m_nodes.end(), node);
}

bool
TaskGraph::containsNode(std::shared_ptr<TaskNode> node) const
{
    return findNode(node) != endNode();
}

bool
TaskGraph::containsEdge(std::shared_ptr<TaskNode> srcNode, std::shared_ptr<TaskNode> destNode) const
{
    return (m_adjList.count(srcNode) != 0 && m_adjList.at(srcNode).count(destNode) != 0);
}

void
TaskGraph::addEdge(std::shared_ptr<TaskNode> srcNode, std::shared_ptr<TaskNode> destNode)
{
    CHECK(containsNode(srcNode)) << "source node \"" << srcNode->m_name << "\" does not exist in graph";
    CHECK(containsNode(destNode)) << "destination node \"" << destNode->m_name << "\" does not exist in graph";

    m_adjList[srcNode].insert(destNode);
    m_invAdjList[destNode].insert(srcNode);
}

void
TaskGraph::addEdges(const std::vector<std::pair<std::shared_ptr<TaskNode>, std::shared_ptr<TaskNode>>>& edges)
{
    using EdgePair = std::pair<std::shared_ptr<TaskNode>, std::shared_ptr<TaskNode>>;
    std::for_each(edges.begin(), edges.end(), [this](const EdgePair& edge) { addEdge(edge.first, edge.second); });
}

void
TaskGraph::nestGraph(std::shared_ptr<TaskGraph> subgraph, std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink)
{
    CHECK(containsNode(source)) << "Tried to nest a graph using source, but source does not exist in this";
    CHECK(containsNode(sink)) << "Tried to nest a graph using sink, but sink does not exist in this";

    // Copy all of the nodes into this graph (check duplicates)
    for (const auto& node : subgraph->getNodes())
    {
        addNode(node);
    }

    // Copy all the edges into the graph (no need to check for duplicates)
    const TaskNodeAdjList& adjList = subgraph->getAdjList();
    for (TaskNodeAdjList::const_iterator it = adjList.begin(); it != adjList.end(); it++)
    {
        const TaskNodeSet& outputNodes = it->second;
        for (TaskNodeSet::const_iterator jt = outputNodes.begin(); jt != outputNodes.end(); jt++)
        {
            addEdge(it->first, *jt);
        }
    }
    // Add source and sink edges
    addEdge(source, subgraph->getSource());
    addEdge(subgraph->getSink(), sink);
}

void
TaskGraph::removeEdge(std::shared_ptr<TaskNode> srcNode, std::shared_ptr<TaskNode> destNode)
{
    if (m_adjList.count(srcNode) == 0)
    {
        return;
    }
    if (m_adjList[srcNode].count(destNode) == 0)
    {
        return;
    }
    m_adjList[srcNode].erase(destNode);
    m_invAdjList[destNode].erase(srcNode);
    if (m_adjList[srcNode].size() == 0)
    {
        m_adjList.erase(srcNode);
    }
    if (m_invAdjList[destNode].size() == 0)
    {
        m_invAdjList.erase(destNode);
    }
}

bool
TaskGraph::addNode(std::shared_ptr<TaskNode> node)
{
    // If the node already exists return false
    if (!containsNode(node))
    {
        // Put it in this graph
        m_nodes.push_back(node);
        return true;
    }
    else
    {
        return false;
    }
}

void
TaskGraph::addNodes(const std::vector<std::shared_ptr<TaskNode>>& nodes)
{
    std::for_each(nodes.begin(), nodes.end(), [this](const std::shared_ptr<TaskNode>& node) { addNode(node); });
}

std::shared_ptr<TaskNode>
TaskGraph::addFunction(std::string name, std::function<void()> func)
{
    std::shared_ptr<TaskNode> node = std::make_shared<TaskNode>(func, name);
    m_nodes.push_back(node);
    return node;
}

bool
TaskGraph::removeNode(std::shared_ptr<TaskNode> node)
{
    if (!containsNode(node))
    {
        LOG(INFO) << "Tried to remove node " + node->m_name + " from graph but it doesn't contain the node.";
        return false;
    }

    // Erase edges
    TaskNodeSet inputs  = m_invAdjList[node];
    TaskNodeSet outputs = m_adjList[node];
    for (TaskNodeSet::iterator i = inputs.begin(); i != inputs.end(); i++)
    {
        removeEdge(*i, node);
    }
    for (TaskNodeSet::iterator i = outputs.begin(); i != outputs.end(); i++)
    {
        removeEdge(node, *i);
    }

    // Finally remove the node from the graph
    TaskNodeVector::iterator it = findNode(node);
    if (it != endNode())
    {
        m_nodes.erase(it);
    }
    return true;
}

bool
TaskGraph::removeNodeAndRedirect(std::shared_ptr<TaskNode> node)
{
    if (!containsNode(node))
    {
        LOG(INFO) << "Tried to remove node " + node->m_name + " from graph but it doesn't contain the node.";
        return false;
    }

    // Erase edges
    TaskNodeSet inputs  = m_invAdjList[node];
    TaskNodeSet outputs = m_adjList[node];
    for (TaskNodeSet::iterator i = inputs.begin(); i != inputs.end(); i++)
    {
        removeEdge(*i, node);
    }
    for (TaskNodeSet::iterator i = outputs.begin(); i != outputs.end(); i++)
    {
        removeEdge(node, *i);
    }

    // Fix the edges
    for (TaskNodeSet::iterator i = inputs.begin(); i != inputs.end(); i++)
    {
        for (TaskNodeSet::iterator j = outputs.begin(); j != outputs.end(); j++)
        {
            addEdge(*i, *j);
        }
    }

    // Finally remove the node from the graph
    TaskNodeVector::iterator it = findNode(node);
    if (it != endNode())
    {
        m_nodes.erase(it);
    }

    return true;
}

void
TaskGraph::insertAfter(std::shared_ptr<TaskNode> refNode, std::shared_ptr<TaskNode> newNode)
{
    CHECK(containsNode(refNode)) << "Reference Node has to exist in graph for insertAfter.";
    CHECK(!containsNode(newNode)) << "New Node " << newNode->m_name << " already exists in this graph.";

    addNode(newNode);

    // Remove output edges
    TaskNodeSet outputs = m_adjList[refNode]; // Copy (since we are modifying)
    for (TaskNodeSet::iterator i = outputs.begin(); i != outputs.end(); i++)
    {
        removeEdge(refNode, *i);
    }

    // Add refNode->newNode
    addEdge(refNode, newNode);

    // Add newNode->outputs[i]
    for (TaskNodeSet::iterator i = outputs.begin(); i != outputs.end(); i++)
    {
        addEdge(newNode, *i);
    }
}

void
TaskGraph::insertBefore(std::shared_ptr<TaskNode> refNode, std::shared_ptr<TaskNode> newNode)
{
    // Try to add to graph, if already exists, exit
    if (!addNode(newNode))
    {
        return;
    }

    // Remove input edges
    TaskNodeSet inputs = m_invAdjList[refNode]; // Copy (since we are modifying)
    for (TaskNodeSet::iterator i = inputs.begin(); i != inputs.end(); i++)
    {
        removeEdge(*i, refNode);
    }

    // inputs[i]->newNode
    for (TaskNodeSet::iterator i = inputs.begin(); i != inputs.end(); i++)
    {
        addEdge(*i, newNode);
    }

    // refNode->newNode
    addEdge(newNode, refNode);
}

bool
TaskGraph::isReachable(std::shared_ptr<TaskNode> srcNode, std::shared_ptr<TaskNode> destNode)
{
    const TaskNodeAdjList& adjList = getAdjList();

    // Simple BFS
    std::unordered_set<std::shared_ptr<TaskNode>> visitedNodes;

    // It inserts itself as well
    std::queue<std::shared_ptr<TaskNode>> nodeStack;
    nodeStack.push(srcNode);
    while (!nodeStack.empty())
    {
        std::shared_ptr<TaskNode> currNode = nodeStack.front();
        nodeStack.pop();

        // If we've arrived at the desired node
        if (destNode == currNode)
        {
            return true;
        }

        // Add children to stack if not yet visited
        if (adjList.count(currNode) != 0)
        {
            const TaskNodeSet& outputNodes = adjList.at(currNode);
            for (TaskNodeSet::const_iterator j = outputNodes.begin(); j != outputNodes.end(); j++)
            {
                std::shared_ptr<TaskNode> childNode = *j;
                if (visitedNodes.count(childNode) == 0)
                {
                    visitedNodes.insert(childNode);
                    nodeStack.push(childNode);
                }
            }
        }
    }
    return false;
}

void
TaskGraph::clear()
{
    m_nodes.clear();
    clearEdges();
    addNode(m_source);
    addNode(m_sink);
}

//std::shared_ptr<TaskGraph>
//TaskGraph::sum(std::shared_ptr<TaskGraph> graphA, std::shared_ptr<TaskGraph> graphB)
//{
//    std::shared_ptr<TaskGraph> results = std::make_shared<TaskGraph>();
//    // Get rid of the results source/sink
//    results->m_source = nullptr;
//    results->m_sink   = nullptr;
//    results->m_nodes.clear();
//
//    // Sum the nodes
//    TaskNodeVector& nodesA = graphA->getNodes();
//    for (size_t i = 0; i < nodesA.size(); i++)
//    {
//        results->addNode(nodesA[i]);
//    }
//    TaskNodeVector& nodesB = graphB->getNodes();
//    for (size_t i = 0; i < nodesB.size(); i++)
//    {
//        results->addNode(nodesB[i]);
//    }
//
//    // Now sum the edges
//    const TaskNodeAdjList& adjListA = graphA->getAdjList();
//    for (TaskNodeAdjList::const_iterator it = adjListA.begin(); it != adjListA.end(); it++)
//    {
//        const TaskNodeSet& outputNodes = it->second;
//        for (TaskNodeSet::const_iterator jt = outputNodes.begin(); jt != outputNodes.end(); jt++)
//        {
//            results->addEdge(it->first, *jt);
//        }
//    }
//    const TaskNodeAdjList& adjListB = graphB->getAdjList();
//    for (TaskNodeAdjList::const_iterator it = adjListB.begin(); it != adjListB.end(); it++)
//    {
//        const TaskNodeSet& outputNodes = it->second;
//        for (TaskNodeSet::const_iterator jt = outputNodes.begin(); jt != outputNodes.end(); jt++)
//        {
//            results->addEdge(it->first, *jt);
//        }
//    }
//
//    return results;
//}

std::shared_ptr<imstk::TaskNodeList>
TaskGraph::topologicalSort(std::shared_ptr<const TaskGraph> graph)
{
    CHECK(graph != nullptr) << "Graph is nullptr";
    // Compute the number of inputs to each node (we will remove these as we go)
    std::unordered_map<std::shared_ptr<TaskNode>, size_t> numInputs;

    const TaskNodeAdjList& adjList    = graph->getAdjList();
    const TaskNodeAdjList& invAdjList = graph->getInvAdjList();

    for (TaskNodeAdjList::const_iterator i = invAdjList.begin(); i != invAdjList.end(); i++)
    {
        if (invAdjList.count(i->first) != 0)
        {
            numInputs[i->first] = invAdjList.at(i->first).size();
        }
    }

    // Create an edge blacklist for edge removal during algorithm
    std::unordered_map<std::shared_ptr<TaskNode>, std::shared_ptr<TaskNode>> removedEdges;

    auto edgeHasBeenRemoved = [&removedEdges](const std::shared_ptr<TaskNode>& node1, const std::shared_ptr<TaskNode>& node2)
                              {
                                  return (removedEdges.count(node1) != 0) && (removedEdges[node1] == node2);
                              };

    //  Kahns algorithm (BFS/queue)
    //  iterate through all nodes (BFS or DFS) removing edges
    //  nodes are accepted when all input edges have been removed
    std::queue<std::shared_ptr<TaskNode>> sources;
    sources.push(graph->m_source);
    std::shared_ptr<TaskNodeList> results = std::make_shared<TaskNodeList>();

    while (!sources.empty())
    {
        // Remove a node
        std::shared_ptr<TaskNode> node = sources.front();
        sources.pop();

        results->push_back(node);

        // As long as there are children
        if (adjList.count(node) != 0)
        {
            const TaskNodeSet& outputNodes = adjList.at(node);
            for (TaskNodeSet::const_iterator it = outputNodes.begin(); it != outputNodes.end(); it++)
            {
                std::shared_ptr<TaskNode> childNode = *it;
                // If edge is present
                if (!edgeHasBeenRemoved(node, childNode))
                {
                    // Mark edge as removed
                    removedEdges[node] = childNode;

                    // Decrement amount of inputs
                    numInputs[childNode]--;
                    if (numInputs[childNode] <= 0)
                    {
                        sources.push(childNode);
                    }
                }
            }
        }
    }
    return results;
}

std::shared_ptr<TaskGraph>
TaskGraph::resolveCriticalNodes(std::shared_ptr<TaskGraph> graph)
{
    CHECK(graph != nullptr) << "Graph is nullptr";
    std::shared_ptr<TaskGraph> results = std::make_shared<TaskGraph>(*graph);

    const TaskNodeAdjList& adjList = results->getAdjList();
    const TaskNodeVector&  nodes   = results->getNodes();

    // Compute the levels of each node via DFS
    std::unordered_map<std::shared_ptr<TaskNode>, int> depths;
    auto                                               computeDepths =
        [&]()
        {
            std::unordered_set<std::shared_ptr<TaskNode>> visitedNodes;

            // DFS for the dependencies
            std::stack<std::shared_ptr<TaskNode>> nodeStack;
            depths[results->getSource()] = 0;
            nodeStack.push(results->getSource());
            while (!nodeStack.empty())
            {
                std::shared_ptr<TaskNode> currNode  = nodeStack.top();
                int                       currLevel = depths[currNode];
                nodeStack.pop();

                // Add children to stack if not yet visited
                if (adjList.count(currNode) != 0)
                {
                    const TaskNodeSet& outputNodes = adjList.at(currNode);
                    for (TaskNodeSet::const_iterator i = outputNodes.begin(); i != outputNodes.end(); i++)
                    {
                        std::shared_ptr<TaskNode> childNode = *i;
                        if (visitedNodes.count(childNode) == 0)
                        {
                            visitedNodes.insert(childNode);
                            depths[childNode] = currLevel + 1;
                            nodeStack.push(childNode);
                        }
                    }
                }
            }
        };
    computeDepths();

    // Identify the set of critical nodes
    TaskNodeVector critNodes;
    for (size_t i = 0; i < nodes.size(); i++)
    {
        if (nodes[i]->m_isCritical)
        {
            critNodes.push_back(nodes[i]);
        }
    }

    // Compute the critical adjacency list
    // That is, the set of critical nodes that can be reached
    // from a given critical node, think of it as a subgraph
    TaskNodeAdjList critAdjList;
    auto            computeCritList =
        [&]()
        {
            critAdjList.clear();

            // For every critical node
            for (size_t i = 0; i < critNodes.size(); i++)
            {
                std::unordered_set<std::shared_ptr<TaskNode>> visitedNodes;

                // DFS for the dependencies (try to reach another critical)
                std::stack<std::shared_ptr<TaskNode>> nodeStack;
                nodeStack.push(critNodes[i]);
                while (!nodeStack.empty())
                {
                    std::shared_ptr<TaskNode> currNode = nodeStack.top();
                    nodeStack.pop();

                    // If you can reach one critical node from the other then they are adjacent
                    if (currNode->m_isCritical)
                    {
                        critAdjList[critNodes[i]].insert(currNode);
                    }

                    // Add children to stack if not yet visited
                    if (adjList.count(currNode) != 0)
                    {
                        const TaskNodeSet& outputNodes = adjList.at(currNode);
                        for (TaskNodeSet::const_iterator j = outputNodes.begin(); j != outputNodes.end(); j++)
                        {
                            std::shared_ptr<TaskNode> childNode = *j;
                            if (visitedNodes.count(childNode) == 0)
                            {
                                visitedNodes.insert(childNode);
                                nodeStack.push(childNode);
                            }
                        }
                    }
                }
            }
        };
    computeCritList();

    // Now we know which critical nodes depend on each other (we are interested in those that aren't)
    // Because if a critical node depends on another, then it must not be running in parallel to another
    // critical node

    // For every critical pair
    for (size_t i = 0; i < critNodes.size(); i++)
    {
        std::shared_ptr<TaskNode> srcNode = critNodes[i];
        for (size_t j = i + 1; j < critNodes.size(); j++)
        {
            std::shared_ptr<TaskNode> destNode = critNodes[j];
            // If the edge doesn't exist, either way
            if ((critAdjList.count(srcNode) == 0 || critAdjList.at(srcNode).count(destNode) == 0)
                && (critAdjList.count(destNode) == 0 || critAdjList.at(destNode).count(srcNode) == 0))
            {
                // Add an edge between the critical nodes in the direction of levels
                int leveli = depths[srcNode];
                int levelj = depths[destNode];
                // If i is below j, then j->i
                if (leveli > levelj)
                {
                    results->addEdge(destNode, srcNode);
                }
                else
                {
                    results->addEdge(srcNode, destNode);
                }
                computeDepths();
                computeCritList();
            }
        }
    }

    return results;
}

std::shared_ptr<TaskGraph>
TaskGraph::transitiveReduce(std::shared_ptr<TaskGraph> graph)
{
    CHECK(graph != nullptr) << "Graph is nullptr";
    // It's a bad idea to do this method if the graph is cyclic
    if (isCyclic(graph))
    {
        return nullptr;
    }

    std::shared_ptr<TaskGraph> results = std::make_shared<TaskGraph>(*graph);

    // Copy the adjList
    const TaskNodeAdjList adjList = results->getAdjList();

    // For every edge in the graph
    for (TaskNodeAdjList::const_iterator i = adjList.begin(); i != adjList.end(); i++)
    {
        std::shared_ptr<TaskNode> inputNode   = i->first;
        const TaskNodeSet&        outputNodes = i->second;
        for (TaskNodeSet::const_iterator j = outputNodes.begin(); j != outputNodes.end(); j++)
        {
            std::shared_ptr<TaskNode> outputNode = *j;
            // Edge inputNode->outputNode

            // Remove the edge and see if it still reaches
            results->removeEdge(inputNode, outputNode);

            // If there still exists a path inputNode->outputNode, leave it removed
            if (!results->isReachable(inputNode, outputNode))
            {
                results->addEdge(inputNode, outputNode);
            }
        }
    }

    return results;
}

std::shared_ptr<TaskGraph>
TaskGraph::removeRedundantNodes(std::shared_ptr<TaskGraph> graph)
{
    CHECK(graph != nullptr) << "Graph is nullptr";
    std::shared_ptr<TaskGraph> results = std::make_shared<TaskGraph>(*graph);

    const TaskNodeAdjList& adjList    = results->getAdjList();
    const TaskNodeAdjList& invAdjList = results->getInvAdjList();
    TaskNodeVector&        nodes      = results->getNodes();

    for (size_t i = 0; i < nodes.size(); i++)
    {
        std::shared_ptr<TaskNode> node = nodes[i];

        // These can't be removed (following code would break as they have no inputs/outputs)
        if (node == graph->getSource() || node == graph->getSink())
        {
            continue;
        }

        // If node not functional and has single input/output it is removeable
        if (!node->isFunctional())
        {
            // Get copies of the inputs and outputs of the node
            TaskNodeSet inputs  = invAdjList.at(node);
            TaskNodeSet outputs = adjList.at(node);
            if (inputs.size() == 1 && outputs.size() == 1)
            {
                // Remove inputs/outputs of node
                for (TaskNodeSet::iterator j = inputs.begin(); j != inputs.end(); j++)
                {
                    results->removeEdge(*j, node);
                }
                for (TaskNodeSet::iterator j = outputs.begin(); j != outputs.end(); j++)
                {
                    results->removeEdge(node, *j);
                }

                // Fix the edges
                for (TaskNodeSet::iterator j = inputs.begin(); j != inputs.end(); j++)
                {
                    for (TaskNodeSet::iterator k = outputs.begin(); k != outputs.end(); k++)
                    {
                        results->addEdge(*j, *k);
                    }
                }

                // Finally remove the node from the graph
                nodes.erase(nodes.begin() + i);

                // If node was removed, don't advance
                i--;
            }
        }
    }
    return results;
}

std::shared_ptr<TaskGraph>
TaskGraph::removeUnusedNodes(std::shared_ptr<TaskGraph> graph)
{
    CHECK(graph != nullptr) << "Graph is nullptr";
    auto results = std::make_shared<TaskGraph>(*graph);

    // Find the set of nodes not used by any edge
    std::unordered_set<std::shared_ptr<TaskNode>> nodes;
    nodes.reserve(results->m_nodes.size());
    for (auto& i : results->m_adjList)
    {
        nodes.insert(i.first);
        for (auto& j : i.second)
        {
            nodes.insert(j);
        }
    }
    results->m_nodes.resize(nodes.size());
    int iter = 0;
    for (auto& i : nodes)
    {
        results->m_nodes[iter++] = i;
    }
    return results;
}

bool
TaskGraph::isCyclic(std::shared_ptr<TaskGraph> graph)
{
    CHECK(graph != nullptr) << "Graph is nullptr";
    // Brute force, DFS every node to find it again
    const TaskNodeAdjList& adjList = graph->getAdjList();
    const TaskNodeVector&  nodes   = graph->getNodes();
    for (size_t i = 0; i < nodes.size(); i++)
    {
        std::unordered_set<std::shared_ptr<TaskNode>> visitedNodes;

        // DFS for the dependencies (start at children instead of itself)
        std::stack<std::shared_ptr<TaskNode>> nodeStack;
        // Add children to stack if not yet visited
        if (adjList.count(nodes[i]) != 0)
        {
            const TaskNodeSet& outputNodes = adjList.at(nodes[i]);
            for (TaskNodeSet::const_iterator j = outputNodes.begin(); j != outputNodes.end(); j++)
            {
                std::shared_ptr<TaskNode> childNode = *j;
                if (visitedNodes.count(childNode) == 0)
                {
                    visitedNodes.insert(childNode);
                    nodeStack.push(childNode);
                }
            }
        }
        while (!nodeStack.empty())
        {
            std::shared_ptr<TaskNode> currNode = nodeStack.top();
            nodeStack.pop();

            // If we revisit a node then it must be cyclic
            if (nodes[i] == currNode)
            {
                return true;
            }

            // Add children to stack if not yet visited
            if (adjList.count(currNode) != 0)
            {
                const TaskNodeSet& outputNodes = adjList.at(currNode);
                for (TaskNodeSet::const_iterator j = outputNodes.begin(); j != outputNodes.end(); j++)
                {
                    std::shared_ptr<TaskNode> childNode = *j;
                    if (visitedNodes.count(childNode) == 0)
                    {
                        visitedNodes.insert(childNode);
                        nodeStack.push(childNode);
                    }
                }
            }
        }
    }

    return false;
}

std::unordered_map<std::shared_ptr<TaskNode>, std::string>
TaskGraph::getUniqueNodeNames(std::shared_ptr<TaskGraph> graph, bool apply)
{
    CHECK(graph != nullptr) << "Graph is nullptr";
    // Produce non colliding names
    std::unordered_map<std::shared_ptr<TaskNode>, std::string> nodeNames;
    std::unordered_map<std::string, int>                       names;
    const TaskNodeVector&                                      nodes = graph->getNodes();
    for (size_t i = 0; i < nodes.size(); i++)
    {
        nodeNames[nodes[i]] = nodes[i]->m_name;
        names[nodes[i]->m_name]++;
    }
    // Adjust names
    for (std::unordered_map<std::shared_ptr<TaskNode>, std::string>::iterator it = nodeNames.begin();
         it != nodeNames.end(); it++)
    {
        int         nameIter = 0;
        std::string currName = it->second;
        // If we can find a node with this name, increment name counter and try again
        while (names[currName] > 1)
        {
            names[currName]--;
            currName = it->second + std::to_string(nameIter);
            names[currName]++;
            nameIter++;
        }
        nodeNames[it->first] = currName;
    }
    if (apply)
    {
        for (std::shared_ptr<TaskNode> node : nodes)
        {
            node->m_name = nodeNames[node];
        }
    }
    return nodeNames;
}

std::unordered_map<std::shared_ptr<TaskNode>, double>
TaskGraph::getNodeStartTimes(std::shared_ptr<TaskGraph> graph)
{
    CHECK(graph != nullptr) << "Graph is nullptr";
    const TaskNodeAdjList& adjList = graph->getAdjList();

    // Setup a map for total elapsed times at each node
    std::unordered_map<std::shared_ptr<TaskNode>, double> startTimes;

    {
        std::unordered_set<std::shared_ptr<TaskNode>> visitedNodes;

        // BFS down the tree computing total times
        std::stack<std::shared_ptr<TaskNode>> nodeStack;
        startTimes[graph->getSource()] = 0.0;
        nodeStack.push(graph->getSource());
        while (!nodeStack.empty())
        {
            std::shared_ptr<TaskNode> currNode = nodeStack.top();
            nodeStack.pop();

            // Add children to stack if not yet visited
            if (adjList.count(currNode) != 0)
            {
                const TaskNodeSet& outputNodes = adjList.at(currNode);
                for (TaskNodeSet::const_iterator i = outputNodes.begin(); i != outputNodes.end(); i++)
                {
                    std::shared_ptr<TaskNode> childNode = *i;

                    // Determine the time it would take to start this child
                    const double childStartTime = startTimes[currNode] + currNode->m_computeTime;
                    if (childStartTime > startTimes[childNode])
                    {
                        // Accept the longest time as nodes can't continue until all child inputs complete
                        startTimes[childNode] = childStartTime;
                    }
                    // If not visited yet add child to stack
                    if (visitedNodes.count(childNode) == 0)
                    {
                        visitedNodes.insert(childNode);
                        //times[childNode] = times[currNode] + childNode->m_computeTime;
                        nodeStack.push(childNode);
                    }
                }
            }
        }
    }
    return startTimes;
}

TaskNodeList
TaskGraph::getCriticalPath(std::shared_ptr<TaskGraph> graph)
{
    CHECK(graph != nullptr) << "Graph is nullptr";
    std::unordered_map<std::shared_ptr<TaskNode>, double> nodeStartTimes = getNodeStartTimes(graph);

    // Now backtrack to acquire the path of longest duration
    const TaskNodeAdjList&               invAdjList = graph->getInvAdjList();
    std::list<std::shared_ptr<TaskNode>> results;
    {
        std::shared_ptr<TaskNode> currNode = graph->getSink();
        // Starting from the sink, always choose the input with the longer start time
        while (currNode != graph->getSource())
        {
            results.push_front(currNode);
            std::shared_ptr<TaskNode> longestNode = nullptr;
            double                    maxTime     = 0.0;

            // For every parent
            if (invAdjList.count(currNode) != 0)
            {
                const TaskNodeSet& inputNodes = invAdjList.at(currNode);
                for (TaskNodeSet::const_iterator i = inputNodes.begin(); i != inputNodes.end(); i++)
                {
                    std::shared_ptr<TaskNode> parentNode = *i;
                    if (nodeStartTimes[parentNode] >= maxTime)
                    {
                        maxTime     = nodeStartTimes[parentNode];
                        longestNode = parentNode;
                    }
                }
            }

            // Move to the node with the longest time
            currNode = longestNode;
        }
    }
    results.push_front(graph->getSource());
    return results;
}
}