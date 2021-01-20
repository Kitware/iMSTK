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
    CHECK(m_master != nullptr && m_slave != nullptr) << "OneToOneMap map is being applied without valid geometries";

    auto meshMaster = std::dynamic_pointer_cast<PointSet>(m_master);
    auto meshSlave  = std::dynamic_pointer_cast<PointSet>(m_slave);

    CHECK(meshMaster != nullptr && meshSlave != nullptr) << "Fail to cast from geometry to pointset";

    m_oneToOneMap.clear();
    ParallelUtils::SpinLock lock;

    bool bValid = true;
    ParallelUtils::parallelFor(meshSlave->getNumVertices(),
        [&](const size_t nodeId)
        {
            if (!bValid) // If map is invalid, no need to check further
            {
                return;
            }
            // Find the enclosing or closest tetrahedron
            size_t matchingNodeId;
            if (!findMatchingVertex(meshMaster, meshSlave->getVertexPosition(nodeId), matchingNodeId))
            {
                LOG(WARNING) << "Could not find matching node for the node " << nodeId;
                bValid = false;
                return;
            }

            // Add to the map
            // Note: This replaces the map if one with <nodeId> already exists
            lock.lock();
            m_oneToOneMap[nodeId] = matchingNodeId;
            lock.unlock();
        });

    if (!bValid)
    {
        m_oneToOneMap.clear();
        return;
    }

    // Copy data from map to vector for parallel processing
    m_oneToOneMapVector.resize(0);
    for (auto kv: m_oneToOneMap)
    {
        m_oneToOneMapVector.push_back({ kv.first, kv.second });
    }
}

bool
OneToOneMap::findMatchingVertex(const std::shared_ptr<PointSet>& masterMesh, const Vec3d& p, size_t& nodeId)
{
    for (size_t idx = 0; idx < masterMesh->getNumVertices(); ++idx)
    {
        if (masterMesh->getInitialVertexPosition(idx) == p)
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
    auto meshMaster = std::dynamic_pointer_cast<PointSet>(m_master);
    auto meshSlave  = std::dynamic_pointer_cast<PointSet>(m_slave);

#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
    CHECK(dynamic_cast<PointSet*>(m_master.get()) && dynamic_cast<PointSet*>(m_slave.get())) <<
        "Failed to cast from geometry to pointset";
#endif

    const size_t numVertMaster = meshMaster->getNumVertices();
    const size_t numVertSlave  = meshSlave->getNumVertices();

    bool valid = true;
    ParallelUtils::parallelFor(m_oneToOneMapVector.size(),
        [&](const size_t idx) {
            if (!valid) // If map is invalid, no need to check further
            {
                return;
            }
            const auto& mapValue = m_oneToOneMapVector[idx];
            if (mapValue.first >= numVertSlave
                && mapValue.second >= numVertMaster)
            {
                valid = false;
            }
        });

    // check conformity
    if (valid)
    {
        ParallelUtils::parallelFor(meshSlave->getNumVertices(), [&](const size_t nodeId)
            {
                const Vec3d& p    = meshSlave->getVertexPosition(nodeId);
                bool matchFound = false;
                for (size_t idx = 0; idx < meshMaster->getNumVertices(); ++idx)
                {
                    if (meshMaster->getInitialVertexPosition(idx) == p)
                    {
                        matchFound = true;
                        break;
                    }
                }
                if (!matchFound)
                {
                    valid = false;
                }
        });
    }

    return valid;
}

void
OneToOneMap::setMap(const std::map<size_t, size_t>& sourceMap)
{
    m_oneToOneMap = sourceMap;

    // Copy data from map to vector for parallel processing
    m_oneToOneMapVector.resize(0);
    for (auto kv: m_oneToOneMap)
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
    CHECK(m_master != nullptr && m_slave != nullptr) << "OneToOneMap map is being applied without valid geometries";

    // Check data
    CHECK(m_oneToOneMap.size() == m_oneToOneMapVector.size()) << "Internal data is corrupted";

    auto meshMaster = static_cast<PointSet*>(m_master.get());
    auto meshSlave  = static_cast<PointSet*>(m_slave.get());

#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
    CHECK(dynamic_cast<PointSet*>(m_master.get()) && dynamic_cast<PointSet*>(m_slave.get())) <<
        "Failed to cast from geometry to pointset";
#endif

    VecDataArray<double, 3>&       slaveVertices  = *meshSlave->getVertexPositions();
    const VecDataArray<double, 3>& masterVertices = *meshMaster->getVertexPositions();
    ParallelUtils::parallelFor(m_oneToOneMapVector.size(),
        [&](const size_t idx) {
            const auto& mapValue = m_oneToOneMapVector[idx];
            slaveVertices[mapValue.first] = masterVertices[mapValue.second];
        });
    meshSlave->modified();
}

void
OneToOneMap::print() const
{
    // Print Type
    GeometryMap::print();

    // Print the one-to-one map
    LOG(INFO) << "[slaveVertId, masterVertexId]\n";
    for (auto const& mapValue : m_oneToOneMap)
    {
        LOG(INFO) << "[" << mapValue.first << ", " << mapValue.second << "]\n";
    }
}

void
OneToOneMap::setMaster(std::shared_ptr<Geometry> master)
{
    if (!master->isMesh())
    {
        LOG(WARNING) << "The geometry provided is not a mesh!\n";
        return;
    }
    GeometryMap::setMaster(master);
}

void
OneToOneMap::setSlave(std::shared_ptr<Geometry> slave)
{
    if (!slave->isMesh())
    {
        LOG(WARNING) << "The geometry provided is not a mesh!\n";
        return;
    }
    GeometryMap::setSlave(slave);
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
