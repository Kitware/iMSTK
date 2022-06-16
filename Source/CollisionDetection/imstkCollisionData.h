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
#include "imstkTypes.h"

namespace imstk
{
class Geometry;

enum class CollisionElementType
{
    Empty,
    CellVertex,
    CellIndex,
    PointDirection,
    PointIndexDirection
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
/// Possible cells may be: point, edge, triangle, quad, or tetrahedron
/// Maximum 4 vertices (tetrahedron is maximum cell it could represent)
///
struct CellVertexElement
{
    Vec3d pts[4] = { Vec3d::Zero(), Vec3d::Zero(), Vec3d::Zero(), Vec3d::Zero() };
    int size     = 0;
};
///
/// \brief Represents a cell by a single cell id OR by N vertex ids
/// Which case can be determined by the idCount
/// Possible cells may be: point, edge, triangle, quad, or tetrahedron
/// maximum 4 ids (tetrahedron by vertex ids is maximum cell it could represent)
///
struct CellIndexElement
{
    int ids[4]  = { -1, -1, -1, -1 };
    int idCount = 0;
    CellTypeId cellType = IMSTK_VERTEX;
};

///
/// \brief Direclty gives a point-direction contact
/// as its collision data
///
struct PointDirectionElement
{
    Vec3d pt  = Vec3d::Zero();
    Vec3d dir = Vec3d::Zero();
    double penetrationDepth = 0.0;
};
///
/// \brief Direclty gives a point-direction contact
/// as its collision data, point given by index
///
struct PointIndexDirectionElement
{
    int ptIndex = 0;
    Vec3d dir   = Vec3d::Zero();
    double penetrationDepth = 0.0;
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
        m_type = CollisionElementType::Empty;
    }

    CollisionElement(const CellVertexElement& element) : m_element{element}, m_type{CollisionElementType::CellVertex} { }
    void operator=(const CellVertexElement& element)
    {
        m_element.m_CellVertexElement = element;
        m_type = CollisionElementType::CellVertex;
    }

    CollisionElement(const CellIndexElement& element) : m_element{element}, m_type{CollisionElementType::CellIndex} { }
    void operator=(const CellIndexElement& element)
    {
        m_element.m_CellIndexElement = element;
        m_type = CollisionElementType::CellIndex;
    }

    CollisionElement(const PointDirectionElement& element) : m_element{element}, m_type{CollisionElementType::PointDirection} { }
    void operator=(const PointDirectionElement& element)
    {
        m_element.m_PointDirectionElement = element;
        m_type = CollisionElementType::PointDirection;
    }

    CollisionElement(const PointIndexDirectionElement& element) : m_element{element}, m_type{CollisionElementType::PointIndexDirection} { }
    void operator=(const PointIndexDirectionElement& element)
    {
        m_element.m_PointIndexDirectionElement = element;
        m_type = CollisionElementType::PointIndexDirection;
    }

    CollisionElement(const CollisionElement& other)
    {
        m_type    = other.m_type;
        m_ccdData = other.m_ccdData;
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

    CollisionElement& operator=(const CollisionElement& other)
    {
        m_type    = other.m_type;
        m_ccdData = other.m_ccdData;
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
        return *this;
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
    bool m_ccdData = false;
};

///
/// \brief Describes the contact manifold between two geometries
///
class CollisionData
{
public:
    std::vector<CollisionElement> elementsA;
    std::vector<CollisionElement> elementsB;
    std::shared_ptr<Geometry>     geomA;
    std::shared_ptr<Geometry>     geomB;
    std::shared_ptr<Geometry>     prevGeomA;
    std::shared_ptr<Geometry>     prevGeomB;
};
} // namespace imstk
