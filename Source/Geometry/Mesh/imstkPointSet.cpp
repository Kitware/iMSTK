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

#include "imstkPointSet.h"
#include "imstkGraph.h"

namespace imstk
{
void
PointSet::initialize(const StdVectorOfVec3d& vertices)
{
    this->setInitialVertexPositions(vertices);
    this->setVertexPositions(vertices);
}

void
PointSet::clear()
{
    m_initialVertexPositions.clear();
    m_vertexPositions.clear();
    m_vertexPositionsPostTransform.clear();
}

void
PointSet::print() const
{
    Geometry::print();
    LOG(INFO) << "Number of vertices: " << this->getNumVertices();
    LOG(INFO) << "Vertex positions:";
    for (auto& verts : m_vertexPositions)
    {
        LOG(INFO) << verts.x() << ", " << verts.y() << ", " << verts.z();
    }
}

void
PointSet::computeBoundingBox(Vec3d& min, Vec3d& max, const double percent) const
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
        min = min - range * (percent / 100);
        max = max + range * (percent / 100);
    }
}

void
PointSet::setInitialVertexPositions(const StdVectorOfVec3d& vertices)
{
    if (m_originalNumVertices == 0)
    {
        m_initialVertexPositions = vertices;
        m_originalNumVertices    = vertices.size();
        m_maxNumVertices         = (size_t)(m_originalNumVertices * m_loadFactor);
        m_vertexPositions.reserve(m_maxNumVertices);
    }
    else
    {
        LOG(WARNING) << "Already set initial vertices";
    }
}

const StdVectorOfVec3d&
PointSet::getInitialVertexPositions() const
{
    return m_initialVertexPositions;
}

const Vec3d&
PointSet::getInitialVertexPosition(const size_t& vertNum) const
{
    return m_initialVertexPositions.at(vertNum);
}

void
PointSet::setVertexPositions(const StdVectorOfVec3d& vertices)
{
    if (vertices.size() <= m_maxNumVertices)
    {
        m_vertexPositions  = vertices;
        m_dataModified     = true;
        m_transformApplied = false;
        this->updatePostTransformData();
    }
    else
    {
        LOG(WARNING) << "Vertices not set, exceeded maximum number of vertices";
    }
}

const StdVectorOfVec3d&
PointSet::getVertexPositions(DataType type /* = DataType::PostTransform */)
{
    if (type == DataType::PostTransform)
    {
        this->updatePostTransformData();
        return m_vertexPositionsPostTransform;
    }
    return m_vertexPositions;
}

void
PointSet::setVertexPosition(const size_t& vertNum, const Vec3d& pos)
{
    m_vertexPositions.at(vertNum) = pos;
    m_dataModified     = true;
    m_transformApplied = false;
}

const Vec3d&
PointSet::getVertexPosition(const size_t& vertNum, DataType type)
{
    return this->getVertexPositions(type).at(vertNum);
}

void
PointSet::setVertexDisplacements(const StdVectorOfVec3d& diff)
{
    assert(diff.size() == m_vertexPositions.size());
    for (size_t i = 0; i < m_vertexPositions.size(); ++i)
    {
        m_vertexPositions[i] = m_initialVertexPositions[i] + diff[i];
    }
    m_dataModified     = true;
    m_transformApplied = false;
}

void
PointSet::setVertexDisplacements(const Vectord& u)
{
    assert(u.size() == 3 * m_vertexPositions.size());
    size_t dofId = 0;
    for (size_t i = 0; i < m_vertexPositions.size(); ++i)
    {
        m_vertexPositions[i] = m_initialVertexPositions[i] + Vec3d(u(dofId), u(dofId + 1), u(dofId + 2));
        dofId += 3;
    }
    m_dataModified     = true;
    m_transformApplied = false;
}

void
PointSet::translateVertices(const Vec3d& t)
{
    for (size_t i = 0; i < m_vertexPositions.size(); ++i)
    {
        m_vertexPositions[i] += t;
    }
    m_dataModified     = true;
    m_transformApplied = false;
}

void
PointSet::setPointDataMap(const std::map<std::string, StdVectorOfVectorf>& pointData)
{
    m_pointDataMap = pointData;
}

const std::map<std::string, StdVectorOfVectorf>&
PointSet::getPointDataMap() const
{
    return m_pointDataMap;
}

void
PointSet::setPointDataArray(const std::string& arrayName, const StdVectorOfVectorf& arrayData)
{
    if (arrayData.size() != this->getNumVertices())
    {
        LOG(WARNING) << "Specified array should have " << this->getNumVertices()
                     << " tuples, has " << arrayData.size();
        return;
    }
    m_pointDataMap[arrayName] = arrayData;
}

const StdVectorOfVectorf*
PointSet::getPointDataArray(const std::string& arrayName) const
{
    auto it = m_pointDataMap.find(arrayName);
    if (it == m_pointDataMap.end())
    {
        LOG(WARNING) << "No array with such name holds any point data.";
        return nullptr;
    }
    return &(it->second);
}

bool
PointSet::hasPointDataArray(const std::string& arrayName) const
{
    auto it = m_pointDataMap.find(arrayName);
    if (it == m_pointDataMap.end())
    {
        return false;
    }
    return true;
}

size_t
PointSet::getNumVertices() const
{
    return m_vertexPositions.size();
}

void
PointSet::applyTranslation(const Vec3d t)
{
    for (size_t i = 0; i < m_vertexPositions.size(); ++i)
    {
        m_vertexPositions[i]        += t;
        m_initialVertexPositions[i] += t;
    }
    m_dataModified     = true;
    m_transformApplied = false;
}

void
PointSet::applyRotation(const Mat3d r)
{
    for (size_t i = 0; i < m_vertexPositions.size(); ++i)
    {
        m_vertexPositions[i]        = r * m_vertexPositions[i];
        m_initialVertexPositions[i] = r * m_initialVertexPositions[i];
    }
    m_dataModified     = true;
    m_transformApplied = false;
}

void
PointSet::applyScaling(const double s)
{
    for (size_t i = 0; i < m_vertexPositions.size(); ++i)
    {
        m_vertexPositions[i]        = s * m_vertexPositions[i];
        m_initialVertexPositions[i] = s * m_initialVertexPositions[i];
    }
    m_dataModified     = true;
    m_transformApplied = false;
}

void
PointSet::updatePostTransformData()
{
    if (m_transformApplied)
    {
        return;
    }

    if (m_vertexPositionsPostTransform.size() != m_vertexPositions.size())
    {
        m_vertexPositionsPostTransform.resize(m_vertexPositions.size());
    }
    for (size_t i = 0; i < m_vertexPositions.size(); ++i)
    {
        // NOTE: Right now scaling is appended on top of the rigid transform
        // for scaling around the mesh center, and not concatenated within
        // the transform, for ease of use.
        m_vertexPositionsPostTransform[i] = m_transform * (m_vertexPositions[i] * m_scaling);
    }
    m_transformApplied = true;
}

void
PointSet::setLoadFactor(double loadFactor)
{
    m_loadFactor     = loadFactor;
    m_maxNumVertices = (size_t)(m_originalNumVertices * m_loadFactor);
    m_vertexPositions.reserve(m_maxNumVertices);
}

double
PointSet::getLoadFactor()
{
    return m_loadFactor;
}

size_t
PointSet::getMaxNumVertices()
{
    return m_maxNumVertices;
}

std::shared_ptr<Graph>
PointSet::getMeshGraph()
{
    LOG(WARNING) << "The graph of a point set has no edges";

    return std::make_shared<Graph>(this->getNumVertices());
}
} // imstk
