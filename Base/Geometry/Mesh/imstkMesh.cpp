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

#include "imstkMesh.h"

namespace imstk
{

void
Mesh::initialize(const StdVectorOfVec3d& vertices)
{
    this->setInitialVerticesPositions(vertices);
    this->setVerticesPositions(vertices);
}

void
Mesh::clear()
{
    m_initialVerticesPositions.clear();
    m_verticesPositions.clear();
    m_verticesDisplacements.clear();
}

void
Mesh::print() const
{
    Geometry::print();
    LOG(INFO) << "Number of vertices: " << this->getNumVertices();
    LOG(INFO) << "Vertex positions:";
    for (auto &verts : m_verticesPositions)
    {
        LOG(INFO) << verts.x() << ", " << verts.y() << ", " << verts.z();
    }
}

void
Mesh::computeBoundingBox(Vec3d& min, Vec3d& max, const double percent) const
{
    min = Vec3d(MAX_D, MAX_D, MAX_D);
    max = Vec3d(-MAX_D, -MAX_D, -MAX_D);

    for (auto& pos : m_verticesPositions)
    {
        for (int i = 0; i < 3; ++i)
        {
            min[i] = std::min(min[i], pos[i]);
            max[i] = std::max(max[i], pos[i]);
        }
    }

    if (percent == 0.0)
    {
        return;
    }
    else
    {
        Vec3d range = max - min;
        min = min - range*(percent / 100);
        max = max + range*(percent / 100);
    }
}

void
Mesh::setInitialVerticesPositions(const StdVectorOfVec3d& vertices)
{
    m_initialVerticesPositions = vertices;
    m_verticesDisplacements= vertices;
}

const StdVectorOfVec3d&
Mesh::getInitialVerticesPositions() const
{
    return m_initialVerticesPositions;
}

const Vec3d&
Mesh::getInitialVertexPosition(const size_t& vertNum) const
{
    return m_initialVerticesPositions.at(vertNum);
}

void
Mesh::setVerticesPositions(const StdVectorOfVec3d& vertices)
{
    m_verticesPositions = vertices;
}

const StdVectorOfVec3d&
Mesh::getVertexPositions() const
{
    return m_verticesPositions;
}

void
Mesh::setVerticePosition(const size_t& vertNum, const Vec3d& pos)
{
    m_verticesPositions.at(vertNum) = pos;
}

const Vec3d&
Mesh::getVertexPosition(const size_t& vertNum) const
{
    return m_verticesPositions.at(vertNum);
}

void
Mesh::setVerticesDisplacements(const StdVectorOfVec3d& diff)
{
    m_verticesDisplacements = diff;
}

void
Mesh::setVerticesDisplacements(const Vectord& u)
{
    size_t dofId = 0;
    for (auto &vDisp : m_verticesDisplacements)
    {
        vDisp = Vec3d(u(dofId), u(dofId + 1), u(dofId + 2));
        dofId += 3;
    }

    for (size_t i = 0; i < m_verticesPositions.size(); ++i)
    {
        m_verticesPositions[i] = m_initialVerticesPositions[i] + m_verticesDisplacements[i];
    }
}

const StdVectorOfVec3d&
Mesh::getVerticesDisplacements() const
{
    return m_verticesDisplacements;
}

const Vec3d&
Mesh::getVerticeDisplacement(const size_t& vertNum) const
{
    return m_verticesDisplacements.at(vertNum);
}

void
Mesh::setPointDataMap(const std::map<std::string, StdVectorOfVectorf>& pointData)
{
    m_pointDataMap = pointData;
}

const std::map<std::string, StdVectorOfVectorf>&
Mesh::getPointDataMap() const
{
    return m_pointDataMap;
}

void
Mesh::setPointDataArray(const std::string& arrayName, const StdVectorOfVectorf& arrayData)
{
    if ( arrayData.size() != this->getNumVertices())
    {
        LOG(WARNING) << "Specified array should have " << this->getNumVertices()
                     << " tuples, has " << arrayData.size();
        return;
    }
    m_pointDataMap[arrayName] = arrayData;
}

const StdVectorOfVectorf&
Mesh::getPointDataArray(const std::string& arrayName) const
{
    if (!m_pointDataMap.count(arrayName))
    {
        LOG(WARNING) << "No array with such name holds any point data.";
        return StdVectorOfVectorf();
    }
    return m_pointDataMap.at(arrayName);
}

const size_t
Mesh::getNumVertices() const
{
    return m_initialVerticesPositions.size();
}

} // imstk
