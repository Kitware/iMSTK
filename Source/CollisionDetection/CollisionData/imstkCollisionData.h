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

#include "imstkLogger.h"
#include "imstkMath.h"
#include "imstkParallelUtils.h"

namespace imstk
{
class Geometry;

template<class DataElement>
class CDElementVector
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

protected:
    std::vector<DataElement> m_Data;
    ParallelUtils::SpinLock  m_Lock;
};

enum class CollisionElementType
{
    Empty,
    CellVertex,
    CellIndex,
    PointDirection,
    PointIndexDirection,
};

// iMSTK collision data (not contacts) are broken down into the following:
// 1.) Intersecting cells which can be used for contact generation
// 2.) Direct (point, normal, depth) which can be used for contacts
//   - \todo: Consider interpreting PointDirection and Index as CellVertex and CellIndex
//   but give CellVertex and Index a cache-all void* clientData or something for direction, and depth
//
// There are then 3 ways to report a cell:
// 1.) Provide via a single cell id (CellIndexElement)
//    - Not always possible. Some elements don't have ids
// 2.) Provide via a set of vertex ids (CellIndexElement)
//    - Avoids requiring an id for the cell but requires ids for vertices
//    - Useful for cells of cells that may not have ids.
//      ie: edges of triangle, tetrahedron, or face of tet
// 3.) Provide the cell by vertex value (CellVertexElement)
//    - Useful for implicit geometry (cells and verts aren't explicit given in any form)
//
// \todo: Inline initialization can't be used on basic primitive types in a union?
//
struct EmptyElement { };

///
/// \brief Represents a cell by its vertex values
/// Possibly cells may be: point, edge, triangle, quad, or tetrahedron
/// Maximum 4 vertices (tetrahedron is maximum cell it could represent)
///
struct CellVertexElement
{
    Vec3d pts[4] = { Vec3d::Zero(), Vec3d::Zero(), Vec3d::Zero(), Vec3d::Zero() };
    int size;
};
///
/// \brief Represents a cell by its single cell id OR by its N vertex ids
/// which case can be determined by the idCount
/// Possibly cells may be: point, edge, triangle, quad, or tetrahedron
/// maximum 4 ids (tetrahedron by vertex ids is maximum cell it could represent)
///
struct CellIndexElement
{
    int ids[4] = { -1, -1, -1, -1 };
    int idCount;
    CellType cellType;
};

///
/// \brief Direclty gives a point-direction contact
/// as its collision data
///
struct PointDirectionElement
{
    Vec3d pt  = Vec3d::Zero();
    Vec3d dir = Vec3d::Zero();
    double penetrationDepth;
};
///
/// \brief Direclty gives a point-direction contact
/// as its collision data, point given by index
///
struct PointIndexDirectionElement
{
    int ptIndex;
    Vec3d dir = Vec3d::Zero();
    double penetrationDepth;
};

///
/// \brief Union of collision elements. We use a union to avoid polymorphism. There may be many
/// elements and accessing them needs to be quick. Additionally the union keeps them more compact
/// and allows one to keep them on the stack.
///
struct CollisionElement
{
    CollisionElement() : m_element{EmptyElement()}, m_type{CollisionElementType::Empty} { }

    CollisionElement(const EmptyElement& element) : m_element{element}, m_type{CollisionElementType::Empty} { }
    void operator=(const EmptyElement& element)
    {
        m_element.m_EmptyElement = element;
    }

    CollisionElement(const CellVertexElement& element) : m_element{element}, m_type{CollisionElementType::CellVertex} { }
    void operator=(const CellVertexElement& element)
    {
        m_element.m_CellVertexElement = element;
    }

    CollisionElement(const CellIndexElement& element) : m_element{element}, m_type{CollisionElementType::CellIndex} { }
    void operator=(const CellIndexElement& element)
    {
        m_element.m_CellIndexElement = element;
    }

    CollisionElement(const PointDirectionElement& element) : m_element{element}, m_type{CollisionElementType::PointDirection} { }
    void operator=(const PointDirectionElement& element)
    {
        m_element.m_PointDirectionElement = element;
    }

    CollisionElement(const PointIndexDirectionElement& element) : m_element{element}, m_type{CollisionElementType::PointIndexDirection} { }
    void operator=(const PointIndexDirectionElement& element)
    {
        m_element.m_PointIndexDirectionElement = element;
    }

    CollisionElement(const CollisionElement& other)
    {
        m_type = other.m_type;
        switch (m_type)
        {
        case CollisionElementType::Empty:
            break;
        case CollisionElementType::CellVertex:
            m_element.m_CellVertexElement = other.m_element.m_CellVertexElement;
            break;
        case CollisionElementType::CellIndex:
            m_element.m_CellIndexElement = other.m_element.m_CellIndexElement;
            break;
        case CollisionElementType::PointDirection:
            m_element.m_PointDirectionElement = other.m_element.m_PointDirectionElement;
            break;
        case CollisionElementType::PointIndexDirection:
            m_element.m_PointIndexDirectionElement = other.m_element.m_PointIndexDirectionElement;
            break;
        }
    }

    union Element
    {
        EmptyElement m_EmptyElement;
        CellVertexElement m_CellVertexElement;
        CellIndexElement m_CellIndexElement;
        PointDirectionElement m_PointDirectionElement;
        PointIndexDirectionElement m_PointIndexDirectionElement;

        Element() : m_EmptyElement(EmptyElement()) { }
        // Constructors needed here for implicit conversions+assignment between elements and parent struct
        Element(const EmptyElement& ele) : m_EmptyElement(ele) { }
        Element(const CellVertexElement& ele) : m_CellVertexElement(ele) { }
        Element(const CellIndexElement& ele) : m_CellIndexElement(ele) { }
        Element(const PointDirectionElement& ele) : m_PointDirectionElement(ele) { }
        Element(const PointIndexDirectionElement& ele) : m_PointIndexDirectionElement(ele) { }
    } m_element;

    CollisionElementType m_type;
};

///
/// \brief Describes the contact manifold between two geometries
///
class CollisionData
{
public:
    void clearAll()
    {
        elementsA.resize(0);
        elementsB.resize(0);
    }

public:
    CDElementVector<CollisionElement> elementsA;
    CDElementVector<CollisionElement> elementsB;
    std::shared_ptr<Geometry> geomA;
    std::shared_ptr<Geometry> geomB;
};
}