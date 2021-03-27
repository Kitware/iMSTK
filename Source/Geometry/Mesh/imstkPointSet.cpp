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
#include "imstkParallelUtils.h"
#include "imstkLogger.h"
#include "imstkVecDataArray.h"

namespace imstk
{
PointSet::PointSet(const std::string& name) : Geometry(name),
    m_initialVertexPositions(std::make_shared<VecDataArray<double, 3>>()),
    m_vertexPositions(std::make_shared<VecDataArray<double, 3>>())
{
}

void
PointSet::initialize(std::shared_ptr<VecDataArray<double, 3>> vertices)
{
    // Copy data to initial
    this->setInitialVertexPositions(std::make_shared<VecDataArray<double, 3>>(*vertices));

    // Use in place as current vertices
    this->setVertexPositions(vertices);
}

void
PointSet::clear()
{
    if (m_initialVertexPositions != nullptr)
    {
        m_initialVertexPositions->clear();
    }
    if (m_vertexPositions != nullptr)
    {
        m_vertexPositions->clear();
    }
    for (auto i : m_vertexAttributes)
    {
        i.second->clear();
    }
}

void
PointSet::print() const
{
    Geometry::print();
    LOG(INFO) << "Number of vertices: " << this->getNumVertices();
    LOG(INFO) << "Vertex positions:";
    for (auto& verts : *m_vertexPositions)
    {
        LOG(INFO) << "\t" << verts.x() << ", " << verts.y() << ", " << verts.z();
    }
    for (auto i : m_vertexAttributes)
    {
        LOG(INFO) << i.first;
        //i.second->print();
    }
}

void
PointSet::computeBoundingBox(Vec3d& lowerCorner, Vec3d& upperCorner, const double paddingPercent)
{
    updatePostTransformData();
    ParallelUtils::findAABB(*m_vertexPositions, lowerCorner, upperCorner);
    if (paddingPercent > 0.0)
    {
        const Vec3d range = upperCorner - lowerCorner;
        lowerCorner = lowerCorner - range * (paddingPercent / 100.0);
        upperCorner = upperCorner + range * (paddingPercent / 100.0);
    }
}

void
PointSet::setInitialVertexPositions(std::shared_ptr<VecDataArray<double, 3>> vertices)
{
    m_initialVertexPositions = vertices;
}

Vec3d&
PointSet::getInitialVertexPosition(const size_t vertNum)
{
#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
    LOG_IF(FATAL, (vertNum >= m_initialVertexPositions->size())) << "Invalid index";
#endif
    return (*m_initialVertexPositions)[vertNum];
}

void
PointSet::setVertexPositions(std::shared_ptr<VecDataArray<double, 3>> vertices)
{
    m_vertexPositions  = vertices;
    m_transformApplied = false;

    this->updatePostTransformData();
}

std::shared_ptr<VecDataArray<double, 3>>
PointSet::getVertexPositions(DataType type) const
{
    if (type == DataType::PostTransform)
    {
        this->updatePostTransformData();
        return m_vertexPositions;
    }
    return m_initialVertexPositions;
}

void
PointSet::setVertexPosition(const size_t vertNum, const Vec3d& pos)
{
#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
    LOG_IF(FATAL, (vertNum >= m_vertexPositions->size())) << "Invalid index";
#endif
    (*m_vertexPositions)[vertNum] = pos;
    m_transformApplied = false;
    this->updatePostTransformData();
}

const Vec3d&
PointSet::getVertexPosition(const size_t vertNum, DataType type) const
{
#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
    LOG_IF(FATAL, (vertNum >= getVertexPositions()->size())) << "Invalid index";
#endif
    return (*this->getVertexPositions(type))[vertNum];
}

Vec3d&
PointSet::getVertexPosition(const size_t vertNum, DataType type)
{
#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
    LOG_IF(FATAL, (vertNum >= getVertexPositions()->size())) << "Invalid index";
#endif
    return (*this->getVertexPositions(type))[vertNum];
}

size_t
PointSet::getNumVertices() const
{
    return m_vertexPositions->size();
}

void
PointSet::applyTransform(const Mat4d& m)
{
    VecDataArray<double, 3>& initVertices = *m_initialVertexPositions;
    VecDataArray<double, 3>& vertices     = *m_vertexPositions;

    ParallelUtils::parallelFor(initVertices.size(),
        [&](const size_t i)
        {
            initVertices[i] = (m * Vec4d(initVertices[i][0], initVertices[i][1], initVertices[i][2], 1.0)).head<3>();
            vertices[i]     = initVertices[i];
        });
    m_transformApplied = false;
    this->updatePostTransformData();
}

void
PointSet::updatePostTransformData() const
{
    if (m_transformApplied)
    {
        return;
    }

    const VecDataArray<double, 3>& initVertices = *m_initialVertexPositions;
    VecDataArray<double, 3>&       vertices     = *m_vertexPositions;

    if (initVertices.size() != vertices.size())
    {
        vertices.resize(initVertices.size());
    }

    ParallelUtils::parallelFor(vertices.size(),
        [&](const size_t i)
        {
            vertices[i] = (m_transform * Vec4d(initVertices[i][0], initVertices[i][1], initVertices[i][2], 1.0)).head<3>();
        });
    m_transformApplied = true;
}

void
PointSet::setLoadFactor(const double loadFactor)
{
    m_loadFactor     = loadFactor;
    m_maxNumVertices = static_cast<size_t>(m_originalNumVertices * m_loadFactor);
    m_vertexPositions->reserve(static_cast<int>(m_maxNumVertices));
}

bool
PointSet::hasVertexAttribute(const std::string& arrayName) const
{
    return (m_vertexAttributes.find(arrayName) != m_vertexAttributes.end());
}

void
PointSet::setVertexAttribute(const std::string& arrayName, std::shared_ptr<AbstractDataArray> arr)
{
    m_vertexAttributes[arrayName] = arr;
}

std::shared_ptr<AbstractDataArray>
PointSet::getVertexAttribute(const std::string& arrayName) const
{
    auto it = m_vertexAttributes.find(arrayName);
    if (it == m_vertexAttributes.end())
    {
        return nullptr;
    }
    return it->second;
}

void
PointSet::setVertexScalars(const std::string& arrayName, std::shared_ptr<AbstractDataArray> scalars)
{
    m_activeVertexScalars = arrayName;
    m_vertexAttributes[arrayName] = scalars;
}

void
PointSet::setVertexScalars(const std::string& arrayName)
{
    if (hasVertexAttribute(arrayName))
    {
        m_activeVertexScalars = arrayName;
    }
}

std::shared_ptr<AbstractDataArray>
PointSet::getVertexScalars() const
{
    if (hasVertexAttribute(m_activeVertexScalars))
    {
        return m_vertexAttributes.at(m_activeVertexScalars);
    }
    else
    {
        return nullptr;
    }
}

void
PointSet::setVertexNormals(const std::string& arrayName, std::shared_ptr<VecDataArray<double, 3>> normals)
{
    m_activeVertexNormals = arrayName;
    m_vertexAttributes[arrayName] = normals;
}

void
PointSet::setVertexNormals(const std::string& arrayName)
{
    if (hasVertexAttribute(arrayName))
    {
        m_activeVertexNormals = arrayName;
    }
}

std::shared_ptr<VecDataArray<double, 3>>
PointSet::getVertexNormals() const
{
    if (hasVertexAttribute(m_activeVertexNormals))
    {
        return std::dynamic_pointer_cast<VecDataArray<double, 3>>(m_vertexAttributes.at(m_activeVertexNormals));
    }
    else
    {
        return nullptr;
    }
}

void
PointSet::setVertexTangents(const std::string& arrayName, std::shared_ptr<VecDataArray<float, 3>> tangents)
{
    m_activeVertexTangents = arrayName;
    m_vertexAttributes[arrayName] = tangents;
}

void
PointSet::setVertexTangents(const std::string& arrayName)
{
    if (hasVertexAttribute(arrayName))
    {
        m_activeVertexTangents = arrayName;
    }
}

std::shared_ptr<VecDataArray<float, 3>>
PointSet::getVertexTangents() const
{
    if (hasVertexAttribute(m_activeVertexTangents))
    {
        return std::dynamic_pointer_cast<VecDataArray<float, 3>>(m_vertexAttributes.at(m_activeVertexTangents));
    }
    else
    {
        return nullptr;
    }
}

void
PointSet::setVertexTCoords(const std::string& arrayName, std::shared_ptr<VecDataArray<float, 2>> tcoords)
{
    m_activeVertexTCoords = arrayName;
    m_vertexAttributes[arrayName] = tcoords;
}

void
PointSet::setVertexTCoords(const std::string& arrayName)
{
    if (hasVertexAttribute(arrayName))
    {
        m_activeVertexTCoords = arrayName;
    }
}

std::shared_ptr<VecDataArray<float, 2>>
PointSet::getVertexTCoords() const
{
    if (hasVertexAttribute(m_activeVertexTCoords))
    {
        return std::dynamic_pointer_cast<VecDataArray<float, 2>>(m_vertexAttributes.at(m_activeVertexTCoords));
    }
    else
    {
        return nullptr;
    }
}
} // imstk
