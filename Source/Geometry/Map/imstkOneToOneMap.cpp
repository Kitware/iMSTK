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

#undef min
#undef max
#include <climits>

namespace imstk
{
void
OneToOneMap::compute()
{
    if (!m_master || !m_slave)
    {
        LOG(WARNING) << "OneToOneMap map is being applied without valid geometries";
        return;
    }

    // returns the first matching vertex
    auto findMatchingVertex = [](const std::shared_ptr<PointSet>& masterMesh, const Vec3d& p, size_t& nodeId)
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
                              };

    auto meshMaster = std::dynamic_pointer_cast<PointSet>(m_master);
    auto meshSlave  = std::dynamic_pointer_cast<PointSet>(m_slave);

    LOG_IF(FATAL, (!meshMaster || !meshSlave)) << "Fail to cast from geometry to pointset";

    m_oneToOneMap.clear();
    ParallelUtils::SpinLock lock;
    bool                    bValid = true;

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
OneToOneMap::isValid() const
{
    auto meshMaster = static_cast<PointSet*>(m_master.get());
    auto meshSlave  = static_cast<PointSet*>(m_slave.get());

#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
    LOG_IF(FATAL, (!dynamic_cast<PointSet*>(m_master.get())
                   || !dynamic_cast<PointSet*>(m_slave.get()))) << "Fail to cast from geometry to pointset";
#endif

    auto numVertMaster = meshMaster->getNumVertices();
    auto numVertSlave  = meshSlave->getNumVertices();
    bool bOK           = true;

    ParallelUtils::parallelFor(m_oneToOneMapVector.size(),
        [&](const size_t idx) {
            if (!bOK) // If map is invalid, no need to check further
            {
                return;
            }
            const auto& mapValue = m_oneToOneMapVector[idx];
            if (mapValue.first >= numVertSlave
                && mapValue.second >= numVertMaster)
            {
                LOG(WARNING) << "OneToOneMap map is not valid! Vertex indices out of bounds.";
                bOK = false;
            }
        });

    return bOK;
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
    if (!m_master || !m_slave)
    {
        LOG(WARNING) << "OneToOneMap map is being applied without valid geometries";
        return;
    }

    // Check data
    LOG_IF(FATAL, (m_oneToOneMap.size() != m_oneToOneMapVector.size())) << "Internal data is corrupted";

    auto meshMaster = static_cast<PointSet*>(m_master.get());
    auto meshSlave  = static_cast<PointSet*>(m_slave.get());

#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
    LOG_IF(FATAL, (!dynamic_cast<PointSet*>(m_master.get())
                   || !dynamic_cast<PointSet*>(m_slave.get()))) << "Fail to cast from geometry to pointset";
#endif

    ParallelUtils::parallelFor(m_oneToOneMapVector.size(),
        [&](const size_t idx) {
            const auto& mapValue = m_oneToOneMapVector[idx];
            meshSlave->setVertexPosition(mapValue.first, meshMaster->getVertexPosition(mapValue.second));
        });
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
} // imstk
