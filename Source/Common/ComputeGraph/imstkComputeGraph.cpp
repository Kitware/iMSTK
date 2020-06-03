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

#include "imstkComputeGraph.h"
#include "imstkComputeNode.h"
#include "imstkLogger.h"
#include <queue>
#include <stack>
#include <set>

namespace imstk
{
ComputeGraph::ComputeGraph(std::string sourceName, std::string sinkName) :
    m_source(std::make_shared<ComputeNode>()),
    m_sink(std::make_shared<ComputeNode>())
{
    m_source->m_name = sourceName;
    m_sink->m_name   = sinkName;
    addNode(m_source);
    addNode(m_sink);
}

ComputeNodeVector::iterator
ComputeGraph::findNode(std::string name)
{
    return std::find_if(m_nodes.begin(), m_nodes.end(),
        [&name](const std::shared_ptr<ComputeNode>& x) { return x->m_name == name; });
}

ComputeNodeVector::const_iterator
ComputeGraph::findNode(std::string name) const
{
    return std::find_if(m_nodes.begin(), m_nodes.end(),
        [&name](const std::shared_ptr<ComputeNode>& x) { return x->m_name == name; });
}

ComputeNodeVector::iterator
ComputeGraph::findNode(std::shared_ptr<ComputeNode> node)
{
    return std::find(m_nodes.begin(), m_nodes.end(), node);
}

ComputeNodeVector::const_iterator
ComputeGraph::findNode(std::shared_ptr<ComputeNode> node) const
{
    return std::find(m_nodes.begin(), m_nodes.end(), node);
}

bool
ComputeGraph::containsNode(std::shared_ptr<ComputeNode> node) const
{
    return findNode(node) != nodesEnd();
}

bool
ComputeGraph::containsEdge(std::shared_ptr<ComputeNode> srcNode, std::shared_ptr<ComputeNode> destNode)
{
    return (m_adjList.count(srcNode) != 0 && m_adjList[srcNode].count(destNode) != 0);
}

void
ComputeGraph::addEdge(std::shared_ptr<ComputeNode> srcNode, std::shared_ptr<ComputeNode> destNode)
{
    m_adjList[srcNode].insert(destNode);
    m_invAdjList[destNode].insert(srcNode);
}

void
ComputeGraph::nestGraph(std::shared_ptr<ComputeGraph> subgraph, std::shared_ptr<ComputeNode> source, std::shared_ptr<ComputeNode> sink)
{
    // Ensure source and sink are in the graph
    if (findNode(source) == nodesEnd())
    {
        LOG(WARNING) << "Tried to nest a graph using source, but source does not exist in this";
        return;
    }
    if (findNode(sink) == nodesEnd())
    {
        LOG(WARNING) << "Tried to nest a graph using sink, but sink does not exist in this";
        return;
    }

    // Copy all of the nodes into this graph (check duplicates)
    for (ComputeNodeVector::iterator it = subgraph->getNodes().begin(); it != subgraph->getNodes().end(); it++)
    {
        addNode(*it);
    }
    // Copy all the edges into the graph (no need to check for duplicates)
    const ComputeNodeAdjList& adjList = subgraph->getAdjList();
    for (ComputeNodeAdjList::const_iterator it = adjList.begin(); it != adjList.end(); it++)
    {
        const ComputeNodeSet& outputNodes = it->second;
        for (ComputeNodeSet::const_iterator jt = outputNodes.begin(); jt != outputNodes.end(); jt++)
        {
            addEdge(it->first, *jt);
        }
    }
    // Add source and sink edges
    addEdge(source, subgraph->getSource());
    addEdge(subgraph->getSink(), sink);
}

void
ComputeGraph::removeEdge(std::shared_ptr<ComputeNode> srcNode, std::shared_ptr<ComputeNode> destNode)
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
ComputeGraph::addNode(std::shared_ptr<ComputeNode> node)
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

std::shared_ptr<ComputeNode>
ComputeGraph::addFunction(std::string name, std::function<void()> func)
{
    std::shared_ptr<ComputeNode> node = std::make_shared<ComputeNode>(func, name);
    m_nodes.push_back(node);
    return node;
}

bool
ComputeGraph::removeNode(std::shared_ptr<ComputeNode> node)
{
    if (!containsNode(node))
    {
        LOG(INFO) << "Tried to remove node " + node->m_name + " from graph but it doesn't contain the node.";
        return false;
    }

    // Erase edges
    ComputeNodeSet inputs = m_invAdjList[node];
    ComputeNodeSet outputs = m_adjList[node];
    for (ComputeNodeSet::iterator i = inputs.begin(); i != inputs.end(); i++)
    {
        removeEdge(*i, node);
    }
    for (ComputeNodeSet::iterator i = outputs.begin(); i != outputs.end(); i++)
    {
        removeEdge(node, *i);
    }

    // Finally remove the node from the graph
    ComputeNodeVector::iterator it = findNode(node);
    if (it != nodesEnd())
    {
        m_nodes.erase(it);
    }
    return true;
}

bool
ComputeGraph::removeNodeAndFix(std::shared_ptr<ComputeNode> node)
{
    if (!containsNode(node))
    {
        LOG(INFO) << "Tried to remove node " + node->m_name + " from graph but it doesn't contain the node.";
        return false;
    }

    // Erase edges
    ComputeNodeSet inputs = m_invAdjList[node];
    ComputeNodeSet outputs = m_adjList[node];
    for (ComputeNodeSet::iterator i = inputs.begin(); i != inputs.end(); i++)
    {
        removeEdge(*i, node);
    }
    for (ComputeNodeSet::iterator i = outputs.begin(); i != outputs.end(); i++)
    {
        removeEdge(node, *i);
    }

    // Fix the edges
    for (ComputeNodeSet::iterator i = inputs.begin(); i != inputs.end(); i++)
    {
        for (ComputeNodeSet::iterator j = outputs.begin(); j != outputs.end(); j++)
        {
            addEdge(*i, *j);
        }
    }

    // Finally remove the node from the graph
    ComputeNodeVector::iterator it = findNode(node);
    if (it != nodesEnd())
    {
        m_nodes.erase(it);
    }

    return true;
}

void
ComputeGraph::insertAfter(std::shared_ptr<ComputeNode> refNode, std::shared_ptr<ComputeNode> newNode)
{
    // Try to add to graph, if already exists, exit
    if (!addNode(newNode))
    {
        return;
    }

    // Remove output edges
    ComputeNodeSet outputs = m_adjList[refNode]; // Copy (since we are modifying)
    for (ComputeNodeSet::iterator i = outputs.begin(); i != outputs.end(); i++)
    {
        removeEdge(refNode, *i);
    }

    // Add refNode->newNode
    addEdge(refNode, newNode);

    // Add newNode->outputs[i]
    for (ComputeNodeSet::iterator i = outputs.begin(); i != outputs.end(); i++)
    {
        addEdge(newNode, *i);
    }
}

void
ComputeGraph::insertBefore(std::shared_ptr<ComputeNode> refNode, std::shared_ptr<ComputeNode> newNode)
{
    // Try to add to graph, if already exists, exit
    if (!addNode(newNode))
    {
        return;
    }

    // Remove input edges
    ComputeNodeSet inputs = m_invAdjList[refNode]; // Copy (since we are modifying)
    for (ComputeNodeSet::iterator i = inputs.begin(); i != inputs.end(); i++)
    {
        removeEdge(*i, refNode);
    }

    // inputs[i]->newNode
    for (ComputeNodeSet::iterator i = inputs.begin(); i != inputs.end(); i++)
    {
        addEdge(*i, newNode);
    }

    // refNode->newNode
    addEdge(newNode, refNode);
}

bool
ComputeGraph::reaches(std::shared_ptr<ComputeNode> srcNode, std::shared_ptr<ComputeNode> destNode)
{
    const ComputeNodeAdjList& adjList = getAdjList();

    // Simple BFS
    std::unordered_set<std::shared_ptr<ComputeNode>> visitedNodes;

    // It inserts itself as well
    std::queue<std::shared_ptr<ComputeNode>> nodeStack;
    nodeStack.push(srcNode);
    while (!nodeStack.empty())
    {
        std::shared_ptr<ComputeNode> currNode = nodeStack.front();
        nodeStack.pop();

        // If we've arrived at the desired node
        if (destNode == currNode)
        {
            return true;
        }

        // Add children to stack if not yet visited
        if (adjList.count(currNode) != 0)
        {
            const ComputeNodeSet& outputNodes = adjList.at(currNode);
            for (ComputeNodeSet::const_iterator j = outputNodes.begin(); j != outputNodes.end(); j++)
            {
                std::shared_ptr<ComputeNode> childNode = *j;
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
ComputeGraph::clear()
{
    m_nodes.clear();
    clearEdges();
    addNode(m_source);
    addNode(m_sink);
}

std::shared_ptr<ComputeGraph>
ComputeGraph::sum(std::shared_ptr<ComputeGraph> graphA, std::shared_ptr<ComputeGraph> graphB)
{
    std::shared_ptr<ComputeGraph> results = std::make_shared<ComputeGraph>();
    // Get rid of the results source/sink
    results->m_source = nullptr;
    results->m_sink   = nullptr;
    results->m_nodes.clear();

    // Sum the nodes
    ComputeNodeVector& nodesA = graphA->getNodes();
    for (size_t i = 0; i < nodesA.size(); i++)
    {
        results->addNode(nodesA[i]);
    }
    ComputeNodeVector& nodesB = graphB->getNodes();
    for (size_t i = 0; i < nodesB.size(); i++)
    {
        results->addNode(nodesB[i]);
    }

    // Now sum the edges
    const ComputeNodeAdjList& adjListA = graphA->getAdjList();
    for (ComputeNodeAdjList::const_iterator it = adjListA.begin(); it != adjListA.end(); it++)
    {
        const ComputeNodeSet& outputNodes = it->second;
        for (ComputeNodeSet::const_iterator jt = outputNodes.begin(); jt != outputNodes.end(); jt++)
        {
            results->addEdge(it->first, *jt);
        }
    }
    const ComputeNodeAdjList& adjListB = graphB->getAdjList();
    for (ComputeNodeAdjList::const_iterator it = adjListB.begin(); it != adjListB.end(); it++)
    {
        const ComputeNodeSet& outputNodes = it->second;
        for (ComputeNodeSet::const_iterator jt = outputNodes.begin(); jt != outputNodes.end(); jt++)
        {
            results->addEdge(it->first, *jt);
        }
    }

    return results;
}

std::shared_ptr<ComputeNodeList>
ComputeGraph::topologicalSort(std::shared_ptr<ComputeGraph> graph)
{
    // Compute the number of inputs to each node (we will remove these as we go)
    std::unordered_map<std::shared_ptr<ComputeNode>, size_t> numInputs;

    const ComputeNodeAdjList& adjList = graph->getAdjList();
    const ComputeNodeAdjList& invAdjList = graph->getInvAdjList();

    for (ComputeNodeAdjList::const_iterator i = invAdjList.begin(); i != invAdjList.end(); i++)
    {
        numInputs[i->first] = invAdjList.size();
    }

    // Create an edge blacklist for edge removal during algorithm
    std::unordered_map<std::shared_ptr<ComputeNode>, std::shared_ptr<ComputeNode>> removedEdges;

    auto edgeHasBeenRemoved = [&](const std::shared_ptr<ComputeNode>& node1, const std::shared_ptr<ComputeNode>& node2)
                              {
                                  return (removedEdges.count(node1) != 0) && (removedEdges[node1] == node2);
                              };

    // Kahns algorithm (BFS/queue)
    //  iterate through all nodes (BFS or DFS) removing edges
    //  nodes are accepted when all input edges have been removed
    std::queue<std::shared_ptr<ComputeNode>> sources;
    sources.push(graph->m_source);
    std::shared_ptr<ComputeNodeList> results = std::make_shared<ComputeNodeList>();

    while (!sources.empty())
    {
        // Remove a node
        std::shared_ptr<ComputeNode> node = sources.front();
        sources.pop();

        results->push_back(node);

        // As long as there are children
        if (adjList.count(node) != 0)
        {
            const ComputeNodeSet& outputNodes = adjList.at(node);
            for (ComputeNodeSet::const_iterator it = outputNodes.begin(); it != outputNodes.end(); it++)
            {
                std::shared_ptr<ComputeNode> childNode = *it;
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

std::shared_ptr<ComputeGraph>
ComputeGraph::resolveCriticalNodes(std::shared_ptr<ComputeGraph> graph)
{
    std::shared_ptr<ComputeGraph> results = std::make_shared<ComputeGraph>(*graph);

    const ComputeNodeAdjList& adjList = graph->getAdjList();
    const ComputeNodeVector& nodes = graph->getNodes();

    // Compute the levels of each node via DFS
    std::unordered_map<std::shared_ptr<ComputeNode>, int> depths;
    {
        std::unordered_set<std::shared_ptr<ComputeNode>> visitedNodes;

        // DFS for the dependencies
        std::stack<std::shared_ptr<ComputeNode>> nodeStack;
        depths[graph->getSource()] = 0;
        nodeStack.push(graph->getSource());
        while (!nodeStack.empty())
        {
            std::shared_ptr<ComputeNode> currNode = nodeStack.top();
            int currLevel = depths[currNode];
            nodeStack.pop();

            // Add children to stack if not yet visited
            if (adjList.count(currNode) != 0)
            {
                const ComputeNodeSet& outputNodes = adjList.at(currNode);
                for (ComputeNodeSet::const_iterator i = outputNodes.begin(); i != outputNodes.end(); i++)
                {
                    std::shared_ptr<ComputeNode> childNode = *i;
                    if (visitedNodes.count(childNode) == 0)
                    {
                        visitedNodes.insert(childNode);
                        depths[childNode] = currLevel + 1;
                        nodeStack.push(childNode);
                    }
                }
            }
        }
    }

    // Identify the set of critical nodes
    ComputeNodeVector critNodes;
    for (size_t i = 0; i < nodes.size(); i++)
    {
        if (nodes[i]->m_critical)
            critNodes.push_back(nodes[i]);
    }

    // Compute the critical adjacency list
    ComputeNodeAdjList critAdjList;
    for (size_t i = 0; i < critNodes.size(); i++)
    {
        std::unordered_set<std::shared_ptr<ComputeNode>> visitedNodes;

        // DFS for the dependencies
        std::stack<std::shared_ptr<ComputeNode>> nodeStack;
        nodeStack.push(critNodes[i]);
        while (!nodeStack.empty())
        {
            std::shared_ptr<ComputeNode> currNode = nodeStack.top();
            nodeStack.pop();

            // If you can reach one critical node from the other then they are adjacent
            if (currNode->m_critical)
            {
                critAdjList[critNodes[i]].insert(currNode);
            }

            // Add children to stack if not yet visited
            if (adjList.count(currNode) != 0)
            {
                const ComputeNodeSet& outputNodes = adjList.at(currNode);
                for (ComputeNodeSet::const_iterator j = outputNodes.begin(); j != outputNodes.end(); j++)
                {
                    std::shared_ptr<ComputeNode> childNode = *j;
                    if (visitedNodes.count(childNode) == 0)
                    {
                        visitedNodes.insert(childNode);
                        nodeStack.push(childNode);
                    }
                }
            }
        }
    }

    // Now we know which critical nodes depend on each other (we are interested in those that aren't)
    // For every critical pair
    for (size_t i = 0; i < critNodes.size(); i++)
    {
        std::shared_ptr<ComputeNode> srcNode = critNodes[i];
        for (size_t j = i + 1; j < critNodes.size(); j++)
        {
            std::shared_ptr<ComputeNode> destNode = critNodes[j];
            // If the edge doesn't exist, either way
            if ((critAdjList.count(srcNode) == 0 || critAdjList.at(srcNode).count(destNode) == 0) &&
                (critAdjList.count(destNode) == 0 || critAdjList.at(destNode).count(srcNode) == 0))
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
            }
        }
    }

    return results;
}

std::shared_ptr<ComputeGraph>
ComputeGraph::transitiveReduce(std::shared_ptr<ComputeGraph> graph)
{
    // It's a bad idea to do this method if the graph is cyclic
    if (isCyclic(graph))
        return nullptr;

    std::shared_ptr<ComputeGraph> results = std::make_shared<ComputeGraph>(*graph);

    // Copy the adjList
    const ComputeNodeAdjList adjList = results->getAdjList();

    // For every edge in the graph
    for (ComputeNodeAdjList::const_iterator i = adjList.begin(); i != adjList.end(); i++)
    {
        std::shared_ptr<ComputeNode> inputNode = i->first;
        const ComputeNodeSet& outputNodes = i->second;
        for (ComputeNodeSet::const_iterator j = outputNodes.begin(); j != outputNodes.end(); j++)
        {
            std::shared_ptr<ComputeNode> outputNode = *j;
            // Edge inputNode->outputNode

            // Remove the edge and see if it still reaches
            results->removeEdge(inputNode, outputNode);

            // If there still exists a path inputNode->outputNode, leave it removed
            if (!results->reaches(inputNode, outputNode))
            {
                results->addEdge(inputNode, outputNode);
            }
        }
    }

    return results;
}

std::shared_ptr<ComputeGraph>
ComputeGraph::nonFunctionalPrune(std::shared_ptr<ComputeGraph> graph)
{
    std::shared_ptr<ComputeGraph> results = std::make_shared<ComputeGraph>(*graph);

    const ComputeNodeAdjList& adjList = results->getAdjList();
    const ComputeNodeAdjList& invAdjList = results->getInvAdjList();
    ComputeNodeVector& nodes = results->getNodes();

    for (size_t i = 0; i < nodes.size(); i++)
    {
        std::shared_ptr<ComputeNode> node = nodes[i];

        // These can't be removed (following code would break as they have no inputs/outputs)
        if (node == graph->getSource() || node == graph->getSink())
        {
            continue;
        }

        // If node not functional and has single input/output it is removeable
        if (!node->isFunctional())
        {
            // Get copies of the inputs and outputs of the node
            ComputeNodeSet inputs = invAdjList.at(node);
            ComputeNodeSet outputs = adjList.at(node);
            if (inputs.size() == 1 && outputs.size() == 1)
            {
                // Remove inputs/outputs of node
                for (ComputeNodeSet::iterator j = inputs.begin(); j != inputs.end(); j++)
                {
                    results->removeEdge(*j, node);
                }
                for (ComputeNodeSet::iterator j = outputs.begin(); j != outputs.end(); j++)
                {
                    results->removeEdge(node, *j);
                }

                // Fix the edges
                for (ComputeNodeSet::iterator j = inputs.begin(); j != inputs.end(); j++)
                {
                    for (ComputeNodeSet::iterator k = outputs.begin(); k != outputs.end(); k++)
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

bool
ComputeGraph::isCyclic(std::shared_ptr<ComputeGraph> graph)
{
    // Brute force, DFS every node to find it again
    const ComputeNodeAdjList& adjList = graph->getAdjList();
    const ComputeNodeVector& nodes = graph->getNodes();
    for (size_t i = 0; i < nodes.size(); i++)
    {
        std::unordered_set<std::shared_ptr<ComputeNode>> visitedNodes;

        // DFS for the dependencies (start at children instead of itself)
        std::stack<std::shared_ptr<ComputeNode>> nodeStack;
        // Add children to stack if not yet visited
        if (adjList.count(nodes[i]) != 0)
        {
            const ComputeNodeSet& outputNodes = adjList.at(nodes[i]);
            for (ComputeNodeSet::const_iterator j = outputNodes.begin(); j != outputNodes.end(); j++)
            {
                std::shared_ptr<ComputeNode> childNode = *j;
                if (visitedNodes.count(childNode) == 0)
                {
                    visitedNodes.insert(childNode);
                    nodeStack.push(childNode);
                }
            }
        }
        while (!nodeStack.empty())
        {
            std::shared_ptr<ComputeNode> currNode = nodeStack.top();
            nodeStack.pop();

            // If we revisit a node then it must be cyclic
            if (nodes[i] == currNode)
            {
                return true;
            }

            // Add children to stack if not yet visited
            if (adjList.count(currNode) != 0)
            {
                const ComputeNodeSet& outputNodes = adjList.at(currNode);
                for (ComputeNodeSet::const_iterator j = outputNodes.begin(); j != outputNodes.end(); j++)
                {
                    std::shared_ptr<ComputeNode> childNode = *j;
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

std::unordered_map<std::shared_ptr<ComputeNode>, std::string>
ComputeGraph::getUniqueNames(std::shared_ptr<ComputeGraph> graph, bool apply)
{
    // Produce non colliding names
    std::unordered_map<std::shared_ptr<ComputeNode>, std::string> nodeNames;
    std::unordered_map<std::string, int> names;
    const ComputeNodeVector& nodes = graph->getNodes();
    for (size_t i = 0; i < nodes.size(); i++)
    {
        nodeNames[nodes[i]] = nodes[i]->m_name;
        names[nodes[i]->m_name]++;
    }
    // Adjust names
    for (std::unordered_map<std::shared_ptr<ComputeNode>, std::string>::iterator it = nodeNames.begin();
        it != nodeNames.end(); it++)
    {
        int nameIter = 0;
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
        for (std::shared_ptr<ComputeNode> node : nodes)
        {
            node->m_name = nodeNames[node];
        }
    }
    return nodeNames;
}

std::unordered_map<std::shared_ptr<ComputeNode>, double>
ComputeGraph::getTimes(std::shared_ptr<ComputeGraph> graph)
{
    const ComputeNodeAdjList& adjList = graph->getAdjList();

    // Setup a map for total elapsed times at each node
    std::unordered_map<std::shared_ptr<ComputeNode>, double> times;

    {
        std::unordered_set<std::shared_ptr<ComputeNode>> visitedNodes;

        // BFS down the tree computing total times
        std::stack<std::shared_ptr<ComputeNode>> nodeStack;
        times[graph->getSource()] = 0.0;
        nodeStack.push(graph->getSource());
        while (!nodeStack.empty())
        {
            std::shared_ptr<ComputeNode> currNode = nodeStack.top();
            nodeStack.pop();

            // Add children to stack if not yet visited
            if (adjList.count(currNode) != 0)
            {
                const ComputeNodeSet& outputNodes = adjList.at(currNode);
                for (ComputeNodeSet::const_iterator i = outputNodes.begin(); i != outputNodes.end(); i++)
                {
                    std::shared_ptr<ComputeNode> childNode = *i;
                    if (visitedNodes.count(childNode) == 0)
                    {
                        visitedNodes.insert(childNode);
                        times[childNode] = times[currNode] + childNode->m_elapsedTime;
                        nodeStack.push(childNode);
                    }
                }
            }
        }
    }
    return times;
}

ComputeNodeList
ComputeGraph::getCriticalPath(std::shared_ptr<ComputeGraph> graph)
{
    std::unordered_map<std::shared_ptr<ComputeNode>, double> times = getTimes(graph);

    // Now backtrack to acquire the path of longest duration
    const ComputeNodeAdjList& invAdjList = graph->getInvAdjList();
    std::list<std::shared_ptr<ComputeNode>> results;
    {
        std::shared_ptr<ComputeNode> currNode = graph->getSink();
        // Starting from the sink, choose the input with the longest total elapsed time
        while (currNode != graph->getSource())
        {
            results.push_front(currNode);
            std::shared_ptr<ComputeNode> longestNode = nullptr;
            double maxTime = 0.0;

            // For every parent
            if (invAdjList.count(currNode) != 0)
            {
                const ComputeNodeSet& inputNodes = invAdjList.at(currNode);
                for (ComputeNodeSet::const_iterator i = inputNodes.begin(); i != inputNodes.end(); i++)
                {
                    std::shared_ptr<ComputeNode> parentNode = *i;
                    if (times[parentNode] >= maxTime)
                    {
                        maxTime = times[parentNode];
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