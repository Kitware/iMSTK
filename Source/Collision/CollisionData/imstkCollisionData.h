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

#pragma once

#include <array>
#include <algorithm>

#include "imstkMath.h"
#include "imstkLogUtility.h"
#include "imstkParallelUtils.h"

namespace imstk
{
template<class DataElement>
class CollisionDataBase
{
public:
    ///
    /// \brief operator [] (const accessor)
    ///
    const DataElement& operator[](const size_t idx) const
    {
#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
        LOG_IF(FATAL, (idx >= m_Data.size())) << "Invalid index";
#endif
        return m_Data[idx];
    }

    ///
    /// \brief Thread-safe append a data element
    ///
    void safeAppend(const DataElement& data)
    {
        m_Lock.lock();
        m_Data.push_back(data);
        m_Lock.unlock();
    }

    ///
    /// \brief Append a data element, this is a non thread-safe operation
    ///
    void unsafeAppend(const DataElement& data) { m_Data.push_back(data); }

    ///
    /// \brief Overwrite a data element, this is a non thread-safe operation
    ///
    void setElement(size_t idx, const DataElement& data)
    {
#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
        LOG_IF(FATAL, (idx >= m_Data.size())) << "Invalid index";
#endif
        m_Data[idx] = data;
    }

    ///
    /// \brief Sort the collision data using the provided compare function
    template<class Comp>
    void sort(Comp&& comp) { std::sort(m_Data.begin(), m_Data.end(), comp); }

    ///
    /// \brief Check if the data array is emtpy
    ///
    bool isEmpty() const { return m_Data.empty(); }

    ///
    /// \brief Get the size of the data
    ///
    size_t getSize() const { return m_Data.size(); }

    ///
    /// \brief Resize the data array
    ///
    void resize(size_t newSize) { m_Data.resize(newSize); }

    ///
    /// \brief Clear all data
    ///
    void clear() { m_Data.resize(0); }

private:
    std::vector<DataElement> m_Data;
    ParallelUtils::SpinLock  m_Lock;
};

///
/// \struct PositionDirectionCollisionData
///
/// \brief Point-penetration depth collision data
///
struct PositionDirectionCollisionDataElement
{
    Vec3d posA;
    Vec3d posB;
    Vec3d dirAtoB;
    double penetrationDepth;
};
class PositionDirectionCollisionData : public CollisionDataBase<PositionDirectionCollisionDataElement>
{
};

///
/// \struct MeshToAnalyticalCollisionData
///
/// \brief Mesh to analytical point-penetration depth collision data
///
struct MeshToAnalyticalCollisionDataElement
{
    uint32_t nodeIdx;
    Vec3d penetrationVector;
};
class MeshToAnalyticalCollisionData : public CollisionDataBase<MeshToAnalyticalCollisionDataElement>
{
};

///
/// \struct VertexTriangleCollisionData
///
/// \brief Vertex-triangle collision data
///
struct VertexTriangleCollisionDataElement
{
    uint32_t vertexIdx;
    uint32_t triIdx;
    double closestDistance;
};
class VertexTriangleCollisionData : public CollisionDataBase<VertexTriangleCollisionDataElement>
{
};

///
/// \struct TriangleVertexCollisionData
///
/// \brief Triangle-vertex collision data
///
struct TriangleVertexCollisionDataElement
{
    uint32_t triIdx;
    uint32_t vertexIdx;
    double closestDistance;
};
class TriangleVertexCollisionData : public CollisionDataBase<TriangleVertexCollisionDataElement>
{
};

///
/// \struct EdgeEdgeCollisionData
///
/// \brief Edge-Edge collision data
///
struct EdgeEdgeCollisionDataElement
{
    std::pair<uint32_t, uint32_t> edgeIdA;
    std::pair<uint32_t, uint32_t> edgeIdB;
    float time;
};
class EdgeEdgeCollisionData : public CollisionDataBase<EdgeEdgeCollisionDataElement>
{
};

///
/// \struct PointTetrahedronCollisionData
///
/// \brief Point-tetrahedron collision data
///
struct PointTetrahedronCollisionDataElement
{
    enum CollisionType
    {
        aPenetratingA = 0, // A self-penetration
        aPenetratingB = 1, // vertex is from mesh A, tetrahedron is from mesh B
        bPenetratingA = 2, // vertex is from mesh B, tetrahedron is from mesh A
        bPenetratingB = 3  // B self-penetration
    } collisionType;

    uint32_t vertexIdx;
    uint32_t tetreahedronIdx;
    using WeightsArray = std::array<double, 4>;
    WeightsArray BarycentricCoordinates;
};
class PointTetrahedronCollisionData : public CollisionDataBase<PointTetrahedronCollisionDataElement>
{
};

///
/// \brief The PickingCollisionData struct
///
struct PickingCollisionDataElement
{
    // map of node and point position
    Vec3d ptPos;
    uint32_t nodeIdx;
    bool touchStatus;
};
class PickingCollisionData : public CollisionDataBase<PickingCollisionDataElement>
{
};

///
/// \struct CollisionData
///
/// \brief Class that is the holder of all types of collision data
///
struct CollisionData
{
    void clearAll()
    {
        PDColData.clear();
        VTColData.clear();
        TVColData.clear();
        EEColData.clear();
        MAColData.clear();
        PTColData.clear();
        NodePickData.clear();
    }

    PositionDirectionCollisionData PDColData;           ///< Position Direction collision data
    VertexTriangleCollisionData VTColData;              ///< Vertex Triangle collision data
    TriangleVertexCollisionData TVColData;              ///< Triangle Vertex collision data
    EdgeEdgeCollisionData EEColData;                    ///< Edge Edge collision data
    MeshToAnalyticalCollisionData MAColData;            ///< Mesh to analytical collision data
    PointTetrahedronCollisionData PTColData;            ///< Point Tetrahedron collision data
    PickingCollisionData NodePickData;                  ///< List of points that are picked
};
}
