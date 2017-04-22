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
    this->setInitialVertexPositions(vertices);
    this->setVertexPositions(vertices);
}

void
Mesh::clear()
{
    m_initialVertexPositions.clear();
    m_vertexPositions.clear();
    m_vertexPositionsPostTransform.clear();
}

void
Mesh::print() const
{
    Geometry::print();
    LOG(INFO) << "Number of vertices: " << this->getNumVertices();
    LOG(INFO) << "Vertex positions:";
    for (auto &verts : m_vertexPositions)
    {
        LOG(INFO) << verts.x() << ", " << verts.y() << ", " << verts.z();
    }
}

void
Mesh::computeBoundingBox(Vec3d& min, Vec3d& max, const double percent) const
{
    min = Vec3d(MAX_D, MAX_D, MAX_D);
    max = Vec3d(-MAX_D, -MAX_D, -MAX_D);

    for (auto& pos : m_vertexPositions)
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
Mesh::setInitialVertexPositions(const StdVectorOfVec3d& vertices)
{
    m_initialVertexPositions = vertices;
}

const StdVectorOfVec3d&
Mesh::getInitialVertexPositions() const
{
    return m_initialVertexPositions;
}

const Vec3d&
Mesh::getInitialVertexPosition(const size_t& vertNum) const
{
    return m_initialVertexPositions.at(vertNum);
}

void
Mesh::setVertexPositions(const StdVectorOfVec3d& vertices)
{
    m_vertexPositions = vertices;
    m_dataModified = true;
    m_transformApplied = false;
}

const StdVectorOfVec3d&
Mesh::getVertexPositions(DataType type /* = DataType::PostTransform */)
{
    if (type == DataType::PostTransform)
    {
        this->updatePostTransformData();
        return m_vertexPositionsPostTransform;
    }
    return m_vertexPositions;
}

void
Mesh::setVertexPosition(const size_t& vertNum, const Vec3d& pos)
{
    m_vertexPositions.at(vertNum) = pos;
    m_dataModified = true;
    m_transformApplied = false;
}

const Vec3d&
Mesh::getVertexPosition(const size_t& vertNum, DataType type)
{
    return this->getVertexPositions(type).at(vertNum);
}

void
Mesh::setVertexDisplacements(const StdVectorOfVec3d& diff)
{
    assert(diff.size() == m_vertexPositions.size());
    for (size_t i = 0; i < m_vertexPositions.size(); ++i)
    {
        m_vertexPositions[i] = m_initialVertexPositions[i] + diff[i];
    }
    m_dataModified = true;
    m_transformApplied = false;
}

void
Mesh::setVertexDisplacements(const Vectord& u)
{
    assert(u.size() == 3 * m_vertexPositions.size());
    size_t dofId = 0;
    for (size_t i = 0; i < m_vertexPositions.size(); ++i)
    {
        m_vertexPositions[i] = m_initialVertexPositions[i] + Vec3d(u(dofId), u(dofId + 1), u(dofId + 2));
        dofId += 3;
    }
    m_dataModified = true;
    m_transformApplied = false;
}

void Mesh::translateVertices(const Vec3d& t)
{
    for (size_t i = 0; i < m_vertexPositions.size(); ++i)
    {
        m_vertexPositions[i] += t;
    }
    m_dataModified = true;
    m_transformApplied = false;
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

const StdVectorOfVectorf* const
Mesh::getPointDataArray(const std::string& arrayName) const
{
    auto it = m_pointDataMap.find(arrayName);
    if (it == m_pointDataMap.end())
    {
        LOG(WARNING) << "No array with such name holds any point data.";
        return nullptr;
    }
    return &(it->second);
}

const size_t
Mesh::getNumVertices() const
{
    return m_initialVertexPositions.size();
}


void
Mesh::applyTranslation(const Vec3d t)
{
    for (size_t i = 0; i < m_vertexPositions.size(); ++i)
    {
        m_vertexPositions[i] += t;
    }
    m_dataModified = true;
    m_transformApplied = false;
}

void
Mesh::applyRotation(const Mat3d r)
{
    for (size_t i = 0; i < m_vertexPositions.size(); ++i)
    {
        m_vertexPositions[i] = r * m_vertexPositions[i];
    }
    m_dataModified = true;
    m_transformApplied = false;
}

void
Mesh::applyScaling(const double s)
{
    for (size_t i = 0; i < m_vertexPositions.size(); ++i)
    {
        m_vertexPositions[i] = s * m_vertexPositions[i];
    }
    m_dataModified = true;
    m_transformApplied = false;
}

void
Mesh::updatePostTransformData()
{
    if (m_transformApplied)
    {
        return;
    }

    if (m_vertexPositionsPostTransform.size() != m_vertexPositions.size())
    {
        m_vertexPositionsPostTransform.clear();
        m_vertexPositionsPostTransform.resize(m_vertexPositions.size());
    }
    for (size_t i = 0; i < m_vertexPositions.size(); ++i)
    {
        // NOTE: Right now scaling is appended on top of the rigid transform
        // for scaling around the mesh center, and not concatenated within
        // the transform, for ease of use.
        m_vertexPositionsPostTransform[i] = m_transform * (m_vertexPositions[i]* m_scaling);
    }
    m_transformApplied = true;
}

} // imstk
