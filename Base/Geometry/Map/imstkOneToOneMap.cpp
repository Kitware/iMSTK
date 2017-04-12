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
    auto findMatchingVertex = [](std::shared_ptr<Mesh> masterMesh, const Vec3d& p) -> size_t
    {
        for (size_t nodeId = 0; nodeId < masterMesh->getNumVertices(); ++nodeId)
        {
            if (masterMesh->getInitialVertexPosition(nodeId) == p)
            {
                return nodeId;
            }
        }
        return -1;
    };

    auto meshMaster = std::dynamic_pointer_cast<Mesh>(m_master);
    auto meshSlave = std::dynamic_pointer_cast<Mesh>(m_slave);

    m_oneToOneMap.clear();
    for (size_t nodeId = 0; nodeId < meshSlave->getNumVertices(); ++nodeId)
    {
        // Find the enclosing or closest tetrahedron
        size_t matchingNodeId = findMatchingVertex(meshMaster, meshSlave->getVertexPosition(nodeId));

        if (matchingNodeId < 0)
        {
            LOG(WARNING) << "Could not find matching node for the node " << nodeId;
            continue;
        }

        // add to the map
        // Note: This replaces the map if one with <nodeId> already exists
        m_oneToOneMap[nodeId] = matchingNodeId;
    }
}

bool
OneToOneMap::isValid() const
{
    auto meshMaster = std::dynamic_pointer_cast<Mesh> (m_master);
    auto meshSlave = std::dynamic_pointer_cast<Mesh> (m_slave);

    auto numVertMaster = meshMaster->getNumVertices();
    auto numVertSlave = meshSlave->getNumVertices();

    for (auto const& mapValue : m_oneToOneMap)
    {
        if (mapValue.first >= 0 && mapValue.first < numVertSlave &&
            mapValue.second >= 0 && mapValue.second < numVertMaster)
        {
            continue;
        }
        else
        {
            LOG(WARNING) << "OneToOneMap map is not valid! Vertex indices out of bounds.";
            return false;
        }
    }
    return true;
}

void
OneToOneMap::setMap(const std::map<size_t, size_t>& sourceMap)
{
    this->m_oneToOneMap = sourceMap;
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

    auto meshMaster = std::dynamic_pointer_cast<Mesh>(m_master);
    auto meshSlave = std::dynamic_pointer_cast<Mesh>(m_slave);

    for (auto const& mapValue : m_oneToOneMap)
    {
        meshSlave->setVertexPosition(mapValue.first, meshMaster->getVertexPosition(mapValue.second));
    }
}

void
OneToOneMap::print() const
{
    // Print Type
    GeometryMap::print();

    // Print the one-to-one map
    LOG(INFO) << "[slaveVertId, masterVertexId]\n";
    for (auto const& mapValue : this->m_oneToOneMap)
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
