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

#include "imstkOneToOneMap.h"
#include "imstkParallelUtils.h"
#include "imstkLogger.h"
#include "imstkPointSet.h"

namespace imstk
{
OneToOneMap::OneToOneMap()
{
    setRequiredInputType<PointSet>(0);
    setRequiredInputType<PointSet>(1);
}
OneToOneMap::OneToOneMap(
    std::shared_ptr<Geometry> parent,
    std::shared_ptr<Geometry> child)
{
    setParentGeometry(parent);
    setChildGeometry(child);

    setRequiredInputType<PointSet>(0);
    setRequiredInputType<PointSet>(1);
}

void
OneToOneMap::compute()
{
    if (!areInputsValid())
    {
        LOG(WARNING) << "OneToOneMap failed to run, inputs not satisfied";
        return;
    }

    m_oneToOneMap.clear();
    computeMap(m_oneToOneMap);

    // Copy data from map to vector for parallel processing
    m_oneToOneMapVector.clear();
    for (auto kv : m_oneToOneMap)
    {
        m_oneToOneMapVector.push_back({ kv.first, kv.second });
    }
}

void
OneToOneMap::computeMap(std::unordered_map<int, int>& tetVertToSurfVertMap)
{
    tetVertToSurfVertMap.clear();

    if (!areInputsValid())
    {
        LOG(WARNING) << "OneToOneMap failed to run, inputs not satisfied";
        return;
    }

    auto meshParent = std::dynamic_pointer_cast<PointSet>(getParentGeometry());
    auto meshChild = std::dynamic_pointer_cast<PointSet>(getChildGeometry());

    std::shared_ptr<VecDataArray<double, 3>> parentVerticesPtr = meshParent->getInitialVertexPositions();
    const VecDataArray<double, 3>& parentVertices = *parentVerticesPtr;
    std::shared_ptr<VecDataArray<double, 3>> childVerticesPtr = meshChild->getInitialVertexPositions();
    const VecDataArray<double, 3>& childVertices = *childVerticesPtr;

    // For every vertex on the child, find corresponding one on the parent
    ParallelUtils::SpinLock lock;
    ParallelUtils::parallelFor(meshChild->getNumVertices(),
        [&](const int nodeId)
        {
            // Find the enclosing or closest tetrahedron
            int matchingNodeId = findMatchingVertex(parentVertices, childVertices[nodeId]);
            if (matchingNodeId == -1)
            {
                return;
            }

            // Add to the map
            // Note: This replaces the map if one with <nodeId> already exists
            lock.lock();
            tetVertToSurfVertMap[nodeId] = matchingNodeId; // child index -> parent index
            lock.unlock();
        });
}

int
OneToOneMap::findMatchingVertex(const VecDataArray<double, 3>& parentVertices, const Vec3d& p)
{
    for (int idx = 0; idx < parentVertices.size(); idx++)
    {
        if (p.isApprox(parentVertices[idx], m_epsilon))
        {
            return idx;
        }
    }
    return -1;
}

void
OneToOneMap::setMap(const std::unordered_map<int, int>& sourceMap)
{
    m_oneToOneMap = sourceMap;

    // Copy data from map to vector for parallel processing
    m_oneToOneMapVector.resize(0);
    for (auto kv : m_oneToOneMap)
    {
        m_oneToOneMapVector.push_back({ kv.first, kv.second });
    }
}

void
OneToOneMap::requestUpdate()
{
    auto meshParent = std::dynamic_pointer_cast<PointSet>(getParentGeometry());
    auto meshChild = std::dynamic_pointer_cast<PointSet>(getChildGeometry());

    // Check data
    CHECK(m_oneToOneMap.size() == m_oneToOneMapVector.size()) << "Internal data is corrupted";

    std::shared_ptr<VecDataArray<double, 3>> childVerticesPtr  = meshChild->getVertexPositions();
    VecDataArray<double, 3>&                 childVertices     = *childVerticesPtr;
    std::shared_ptr<VecDataArray<double, 3>> parentVerticesPtr = meshParent->getVertexPositions();
    const VecDataArray<double, 3>&           parentVertices    = *parentVerticesPtr;
    ParallelUtils::parallelFor(m_oneToOneMapVector.size(),
        [&](const size_t idx)
        {
            const auto& mapValue = m_oneToOneMapVector[idx];
            childVertices[mapValue.first] = parentVertices[mapValue.second];
        });
    meshChild->postModified();

    setOutput(meshChild);
}

int
OneToOneMap::getMapIdx(const int idx) const
{
    auto citer = m_oneToOneMap.find(idx);
    if (citer != m_oneToOneMap.end())
    {
        return citer->second;
    }
    else
    {
        return -1;
    }
}
} // namespace imstk