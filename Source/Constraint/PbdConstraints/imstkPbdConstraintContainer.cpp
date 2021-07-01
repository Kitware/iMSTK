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

#include "imstkPbdConstraintContainer.h"
#include "imstkGraph.h"

#include <unordered_map>

namespace imstk
{
void
PbdConstraintContainer::addConstraint(std::shared_ptr<PbdConstraint> constraint)
{
    m_constraintLock.lock();
    m_constraints.push_back(constraint);
    m_constraintLock.unlock();
}

void
PbdConstraintContainer::removeConstraint(std::shared_ptr<PbdConstraint> constraint)
{
    m_constraintLock.lock();
    iterator i = std::find(m_constraints.begin(), m_constraints.end(), constraint);
    if (i != m_constraints.end())
    {
        m_constraints.erase(i);
    }
    m_constraintLock.unlock();
}

void
PbdConstraintContainer::removeConstraints(std::shared_ptr<std::unordered_set<size_t>> vertices)
{
    // Remove constraints that contain the given vertices
    auto removeConstraintFunc = [&](std::shared_ptr<PbdConstraint> constraint)
                                {
                                    for (auto i : constraint->getVertexIds())
                                    {
                                        if (vertices->find(i) != vertices->end())
                                        {
                                            return true;
                                        }
                                    }
                                    return false;
                                };

    m_constraintLock.lock();
    m_constraints.erase(std::remove_if(m_constraints.begin(), m_constraints.end(), removeConstraintFunc),
        m_constraints.end());

    // Also remove partitioned constraints
    for (auto& pc : m_partitionedConstraints)
    {
        pc.erase(std::remove_if(pc.begin(), pc.end(), removeConstraintFunc), pc.end());
    }

    m_constraintLock.unlock();
}

PbdConstraintContainer::iterator
PbdConstraintContainer::eraseConstraint(iterator iter)
{
    m_constraintLock.lock();
    iterator newIter = m_constraints.erase(iter);
    m_constraintLock.unlock();
    return newIter;
}

PbdConstraintContainer::const_iterator
PbdConstraintContainer::eraseConstraint(const_iterator iter)
{
    m_constraintLock.lock();
    const_iterator newIter = m_constraints.erase(iter);
    m_constraintLock.unlock();
    return newIter;
}

void
PbdConstraintContainer::partitionConstraints(const int partitionedThreshold)
{
    // Form the map { vertex : list_of_constraints_involve_vertex }
    std::vector<std::shared_ptr<PbdConstraint>>& allConstraints = m_constraints;

    //std::cout << "---------partitionConstraints: " << allConstraints.size() << std::endl;

    std::unordered_map<size_t, std::vector<size_t>> vertexConstraints;
    for (size_t constrIdx = 0; constrIdx < allConstraints.size(); ++constrIdx)
    {
        const auto& constr = allConstraints[constrIdx];
        for (const auto& vIds : constr->getVertexIds())
        {
            vertexConstraints[vIds].push_back(constrIdx);
        }
    }

    // Add edges to the constraint graph
    // Each edge represent a shared vertex between two constraints
    Graph constraintGraph(allConstraints.size());
    for (const auto& kv : vertexConstraints)
    {
        const auto& constraints = kv.second;     // the list of constraints for a vertex
        for (size_t i = 0; i < constraints.size(); ++i)
        {
            for (size_t j = i + 1; j < constraints.size(); ++j)
            {
                constraintGraph.addEdge(constraints[i], constraints[j]);
            }
        }
    }
    vertexConstraints.clear();

    // do graph coloring for the constraint graph
    const auto coloring = constraintGraph.doColoring(Graph::ColoringMethod::WelshPowell);
    // const auto  coloring = constraintGraph.doColoring(Graph::ColoringMethod::Greedy);
    const auto& partitionIndices = coloring.first;
    const auto  numPartitions    = coloring.second;
    assert(partitionIndices.size() == allConstraints.size());

    std::vector<std::vector<std::shared_ptr<PbdConstraint>>>& partitionedConstraints = m_partitionedConstraints;
    partitionedConstraints.resize(0);
    partitionedConstraints.resize(static_cast<size_t>(numPartitions));

    for (size_t constrIdx = 0; constrIdx < partitionIndices.size(); ++constrIdx)
    {
        const auto partitionIdx = partitionIndices[constrIdx];
        partitionedConstraints[partitionIdx].push_back(allConstraints[constrIdx]);
    }

    // If a partition has size smaller than the partition threshold, then move its constraints back
    // These constraints will be processed sequentially
    // Because small size partitions yield bad performance upon running in parallel
    allConstraints.resize(0);
    for (const auto& constraints : partitionedConstraints)
    {
        if (constraints.size() < partitionedThreshold)
        {
            for (size_t constrIdx = 0; constrIdx < constraints.size(); ++constrIdx)
            {
                allConstraints.push_back(std::move(constraints[constrIdx]));
            }
        }
    }

    // Remove all empty partitions
    size_t writeIdx = 0;
    for (size_t readIdx = 0; readIdx < partitionedConstraints.size(); ++readIdx)
    {
        if (partitionedConstraints[readIdx].size() >= partitionedThreshold)
        {
            if (readIdx != writeIdx)
            {
                partitionedConstraints[writeIdx] = std::move(partitionedConstraints[readIdx]);
            }
            ++writeIdx;
        }
    }
    partitionedConstraints.resize(writeIdx);

    // Print
    /*if (print)
    {
        size_t numConstraints = 0;
        int    idx = 0;
        for (const auto& constraints : partitionedConstraints)
        {
            std::cout << "Partition # " << idx++ << " | # nodes: " << constraints.size() << std::endl;
            numConstraints += constraints.size();
        }
        std::cout << "Sequential processing # nodes: " << allConstraints.size() << std::endl;
        numConstraints += allConstraints.size();
        std::cout << "Total constraints: " << numConstraints << " | Graph size: "
            << constraintGraph.size() << std::endl;
    }*/
}
}