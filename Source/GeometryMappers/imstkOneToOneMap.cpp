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
void
OneToOneMap::compute()
{
    CHECK(m_parentGeom != nullptr && m_childGeom != nullptr) << "OneToOneMap map is being applied without valid geometries";

    auto meshParent = std::dynamic_pointer_cast<PointSet>(m_parentGeom);
    auto meshChild  = std::dynamic_pointer_cast<PointSet>(m_childGeom);

    CHECK(meshParent != nullptr && meshChild != nullptr) << "Fail to cast from geometry to pointset";

    m_oneToOneMap.clear();
    ParallelUtils::SpinLock lock;

    std::shared_ptr<VecDataArray<double, 3>> parentVerticesPtr = meshParent->getInitialVertexPositions();
    const VecDataArray<double, 3>&           parentVertices    = *parentVerticesPtr;
    std::shared_ptr<VecDataArray<double, 3>> childVerticesPtr  = meshChild->getInitialVertexPositions();
    const VecDataArray<double, 3>&           childVertices     = *childVerticesPtr;

    // For every vertex on the child, find corresponding one on the parent
    ParallelUtils::parallelFor(meshChild->getNumVertices(),
        [&](const size_t nodeId)
        {
            // Find the enclosing or closest tetrahedron
            size_t matchingNodeId;
            if (!findMatchingVertex(parentVertices, childVertices[nodeId], matchingNodeId))
            {
                return;
            }

            // Add to the map
            // Note: This replaces the map if one with <nodeId> already exists
            lock.lock();
            m_oneToOneMap[nodeId] = matchingNodeId; // child index -> parent index
            lock.unlock();
        });

    // Copy data from map to vector for parallel processing
    m_oneToOneMapVector.resize(0);
    for (auto kv : m_oneToOneMap)
    {
        m_oneToOneMapVector.push_back({ kv.first, kv.second });
    }
}

bool
OneToOneMap::findMatchingVertex(const VecDataArray<double, 3>& parentVertices, const Vec3d& p, size_t& nodeId)
{
    const double eps2 = m_epsilon * m_epsilon;
    for (int idx = 0; idx < parentVertices.size(); ++idx)
    {
        if ((parentVertices[idx] - p).squaredNorm() < eps2)
        {
            nodeId = idx;
            return true;
        }
    }
    return false;
}

bool
OneToOneMap::isValid() const
{
    return true;
}

void
OneToOneMap::setMap(const std::map<size_t, size_t>& sourceMap)
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
OneToOneMap::apply()
{
    // Check Map active
    if (!m_isActive)
    {
        LOG(WARNING) << "OneToOneMap map is not active";
        return;
    }

    // Check geometries
    CHECK(m_parentGeom != nullptr && m_childGeom != nullptr) << "OneToOneMap map is being applied without valid geometries";

    // Check data
    CHECK(m_oneToOneMap.size() == m_oneToOneMapVector.size()) << "Internal data is corrupted";

    auto meshParent = std::dynamic_pointer_cast<PointSet>(m_parentGeom);
    auto meshChild  = std::dynamic_pointer_cast<PointSet>(m_childGeom);

    CHECK(meshParent != nullptr && meshChild != nullptr) << "Failed to cast from Geometry to PointSet";

    VecDataArray<double, 3>&       childVertices  = *meshChild->getVertexPositions();
    const VecDataArray<double, 3>& parentVertices = *meshParent->getVertexPositions();
    ParallelUtils::parallelFor(m_oneToOneMapVector.size(),
        [&](const size_t idx) {
            const auto& mapValue = m_oneToOneMapVector[idx];
            childVertices[mapValue.first] = parentVertices[mapValue.second];
        });
    meshChild->postModified();
}

void
OneToOneMap::print() const
{
    // Print Type
    GeometryMap::print();

    // Print the one-to-one map
    LOG(INFO) << "[childVertId, parentVertexId]\n";
    for (auto const& mapValue : m_oneToOneMap)
    {
        LOG(INFO) << "[" << mapValue.first << ", " << mapValue.second << "]\n";
    }
}

void
OneToOneMap::setParentGeometry(std::shared_ptr<Geometry> parent)
{
    auto pointSet = std::dynamic_pointer_cast<PointSet>(parent);
    if (parent == nullptr)
    {
        LOG(WARNING) << "The geometry provided is not a PointSet!\n";
        return;
    }
    GeometryMap::setParentGeometry(parent);
}

void
OneToOneMap::setChildGeometry(std::shared_ptr<Geometry> child)
{
    auto pointSet = std::dynamic_pointer_cast<PointSet>(child);
    if (pointSet == nullptr)
    {
        LOG(WARNING) << "The geometry provided is not a PointSet!\n";
        return;
    }
    GeometryMap::setChildGeometry(child);
}

size_t
OneToOneMap::getMapIdx(const size_t& idx)
{
#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
    CHECK(m_oneToOneMap.find(idx) != m_oneToOneMap.end()) << "Invalid source index";
#endif
    return m_oneToOneMap[idx];
}
} // imstk