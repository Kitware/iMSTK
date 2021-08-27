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

#include "imstkCollisionDataDebugObject.h"
#include "imstkLineMesh.h"
#include "imstkSurfaceMesh.h"

namespace imstk
{
void
CollisionDataDebugObject::addCollisionElement(std::shared_ptr<PointSet> pointSet, const CollisionElement& elem)
{
    VecDataArray<double, 3>  dummy;
    VecDataArray<double, 3>& vertices = dummy;
    if (pointSet != nullptr)
    {
        vertices = *pointSet->getVertexPositions();
    }
    if (elem.m_type == CollisionElementType::CellIndex)
    {
        const int      idCount = elem.m_element.m_CellIndexElement.idCount;
        const CellType type    = elem.m_element.m_CellIndexElement.cellType;
        if (type == IMSTK_VERTEX)
        {
            addPoint(vertices[elem.m_element.m_CellIndexElement.ids[0]]);
        }
        else if (type == IMSTK_EDGE)
        {
            Vec2i cell;
            if (idCount == 1)
            {
                auto indices = std::dynamic_pointer_cast<LineMesh>(pointSet)->getLinesIndices();
                cell = (*indices)[elem.m_element.m_CellIndexElement.ids[0]];
            }
            else
            {
                cell = Vec2i(
                    elem.m_element.m_CellIndexElement.ids[0],
                    elem.m_element.m_CellIndexElement.ids[1]);
            }
            addLine(vertices[cell[0]], vertices[cell[1]]);
        }
        else if (type == IMSTK_TRIANGLE)
        {
            Vec3i cell;
            if (idCount == 1)
            {
                auto indices = std::dynamic_pointer_cast<SurfaceMesh>(pointSet)->getTriangleIndices();
                cell = (*indices)[elem.m_element.m_CellIndexElement.ids[0]];
            }
            else
            {
                cell = Vec3i(
                    elem.m_element.m_CellIndexElement.ids[0],
                    elem.m_element.m_CellIndexElement.ids[1],
                    elem.m_element.m_CellIndexElement.ids[2]);
            }
            addTriangle(vertices[cell[0]], vertices[cell[1]], vertices[cell[2]]);
        }
        // No visualizations for tetrahedrons
    }
    else if (elem.m_type == CollisionElementType::CellVertex)
    {
        const int vertexCount = elem.m_element.m_CellVertexElement.size;
        if (vertexCount == 1)
        {
            addPoint(elem.m_element.m_CellVertexElement.pts[0]);
        }
        else if (vertexCount == 2)
        {
            addLine(
                elem.m_element.m_CellVertexElement.pts[0],
                elem.m_element.m_CellVertexElement.pts[1]);
        }
        else if (vertexCount == 3)
        {
            addTriangle(
                elem.m_element.m_CellVertexElement.pts[0],
                elem.m_element.m_CellVertexElement.pts[1],
                elem.m_element.m_CellVertexElement.pts[2]);
        }
        // No visualizations for tetrahedrons
    }
    else if (elem.m_type == CollisionElementType::PointIndexDirection)
    {
        addPoint(vertices[elem.m_element.m_PointIndexDirectionElement.ptIndex]);
        addArrow(vertices[elem.m_element.m_PointIndexDirectionElement.ptIndex],
            vertices[elem.m_element.m_PointIndexDirectionElement.ptIndex] +
            elem.m_element.m_PointIndexDirectionElement.dir * elem.m_element.m_PointIndexDirectionElement.penetrationDepth);
    }
    else if (elem.m_type == CollisionElementType::PointDirection)
    {
        addPoint(elem.m_element.m_PointDirectionElement.pt);
        addArrow(elem.m_element.m_PointDirectionElement.pt,
            elem.m_element.m_PointDirectionElement.pt +
            elem.m_element.m_PointDirectionElement.dir * elem.m_element.m_PointDirectionElement.penetrationDepth);
    }
}

void
CollisionDataDebugObject::printContactInfo(const CollisionElement& elem)
{
    if (elem.m_type == CollisionElementType::CellVertex)
    {
        const int vertexCount = elem.m_element.m_CellVertexElement.size;
        printf(" CellVertexElement\n");
        printf("  # Vertices: %d\n", vertexCount);
        for (int i = 0; i < vertexCount; i++)
        {
            printf("  ptIndex: (%f, %f, %f)\n",
                elem.m_element.m_CellVertexElement.pts[i][0],
                elem.m_element.m_CellVertexElement.pts[i][1],
                elem.m_element.m_CellVertexElement.pts[i][2]);
        }
    }
    else if (elem.m_type == CollisionElementType::CellIndex)
    {
        printf(" CellIndexElement\n");
        printf("  Cell Type: %d\n", elem.m_element.m_CellIndexElement.cellType);
        printf("  # ids: %d\n", elem.m_element.m_CellIndexElement.idCount);
        for (int i = 0; i < elem.m_element.m_CellIndexElement.idCount; i++)
        {
            printf("  Cell Index: %d\n", elem.m_element.m_CellIndexElement.ids[i]);
        }
    }
    else if (elem.m_type == CollisionElementType::PointDirection)
    {
        printf(" PointDirectionElement\n");
        printf("  pt: (%f, %f, %f)\n",
            elem.m_element.m_PointDirectionElement.pt[0],
            elem.m_element.m_PointDirectionElement.pt[1],
            elem.m_element.m_PointDirectionElement.pt[2]);
        printf("  dir: (%f, %f, %f)\n",
            elem.m_element.m_PointDirectionElement.dir[0],
            elem.m_element.m_PointDirectionElement.dir[1],
            elem.m_element.m_PointDirectionElement.dir[2]);
        printf("  penetrationDepth: %f\n", elem.m_element.m_PointDirectionElement.penetrationDepth);
    }
    else if (elem.m_type == CollisionElementType::PointIndexDirection)
    {
        printf(" PointIndexDirectionElement\n");
        printf("  ptIndex: %d\n", elem.m_element.m_PointIndexDirectionElement.ptIndex);
        printf("  dir: (%f, %f, %f)\n",
            elem.m_element.m_PointIndexDirectionElement.dir[0],
            elem.m_element.m_PointIndexDirectionElement.dir[1],
            elem.m_element.m_PointIndexDirectionElement.dir[2]);
        printf("  penetrationDepth: %f\n", elem.m_element.m_PointIndexDirectionElement.penetrationDepth);
    }
}

void
CollisionDataDebugObject::debugUpdate()
{
    if (m_cdData == nullptr)
    {
        return;
    }

    if (m_printContacts)
    {
        printf("# A Contacts: %d\n", static_cast<int>(m_cdData->elementsA.size()));
        printf("# B Contacts: %d\n", static_cast<int>(m_cdData->elementsB.size()));
    }

    // Clear the debug object geometry
    clear();

    std::shared_ptr<PointSet> pointSetA = std::dynamic_pointer_cast<PointSet>(m_cdData->geomA);
    for (int i = 0; i < static_cast<int>(m_cdData->elementsA.size()); i++)
    {
        const CollisionElement& elem = m_cdData->elementsA[i];
        addCollisionElement(pointSetA, elem);
        if (m_printContacts)
        {
            printf("Contact A %d\n", i);
            printContactInfo(elem);
        }
    }
    std::shared_ptr<PointSet> pointSetB = std::dynamic_pointer_cast<PointSet>(m_cdData->geomB);
    for (int i = 0; i < static_cast<int>(m_cdData->elementsB.size()); i++)
    {
        const CollisionElement& elem = m_cdData->elementsB[i];
        addCollisionElement(pointSetB, elem);
        if (m_printContacts)
        {
            printf("Contact B %d\n", i);
            printContactInfo(elem);
        }
    }
}
}