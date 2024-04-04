/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkPointwiseMap.h"
#include "imstkParallelUtils.h"
#include "imstkLogger.h"
#include "imstkPointSet.h"

namespace imstk
{
PointwiseMap::PointwiseMap()
{
    setRequiredInputType<PointSet>(0);
    setRequiredInputType<PointSet>(1);
}

PointwiseMap::PointwiseMap(
    std::shared_ptr<Geometry> parent,
    std::shared_ptr<Geometry> child)
{
    setParentGeometry(parent);
    setChildGeometry(child);

    setRequiredInputType<PointSet>(0);
    setRequiredInputType<PointSet>(1);
}

void
PointwiseMap::compute()
{
    if (!areInputsValid())
    {
        LOG(WARNING) << "PointwiseMap failed to run, inputs not satisfied";
        return;
    }

    m_oneToOneMap.clear();
    computeMap(m_oneToOneMap);

    // Copy data from map to vector for parallel processing
    m_oneToOneMapVector.clear();
    for (const auto& kv : m_oneToOneMap)
    {
        m_oneToOneMapVector.push_back({ kv.first, kv.second });
    }
}

void
PointwiseMap::computeMap(std::unordered_map<int, int>& tetVertToSurfVertMap)
{
    tetVertToSurfVertMap.clear();

    if (!areInputsValid())
    {
        LOG(WARNING) << "PointwiseMap failed to run, inputs not satisfied";
        return;
    }

    auto meshParent = std::dynamic_pointer_cast<PointSet>(getParentGeometry());
    auto meshChild  = std::dynamic_pointer_cast<PointSet>(getChildGeometry());

    std::shared_ptr<VecDataArray<double, 3>> parentVerticesPtr = meshParent->getVertexPositions();
    const VecDataArray<double, 3>&           parentVertices    = *parentVerticesPtr;
    std::shared_ptr<VecDataArray<double, 3>> childVerticesPtr  = meshChild->getVertexPositions();
    const VecDataArray<double, 3>&           childVertices     = *childVerticesPtr;

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
PointwiseMap::findMatchingVertex(const VecDataArray<double, 3>& parentVertices, const Vec3d& p)
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
PointwiseMap::setMap(const std::unordered_map<int, int>& sourceMap)
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
PointwiseMap::requestUpdate()
{
    auto meshParent = std::dynamic_pointer_cast<PointSet>(getParentGeometry());
    auto meshChild  = std::dynamic_pointer_cast<PointSet>(getChildGeometry());

    // Check data
    CHECK(m_oneToOneMap.size() == m_oneToOneMapVector.size()) << "Internal data is corrupted";
    if (m_oneToOneMap.size() == 0)
    {
        return;
    }

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
PointwiseMap::getParentVertexId(const int childVertexId) const
{
    auto citer = m_oneToOneMap.find(childVertexId);
    return (citer != m_oneToOneMap.end()) ? citer->second : -1;
}
} // namespace imstk