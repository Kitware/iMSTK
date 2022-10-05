/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCellPicker.h"
#include "imstkAbstractCellMesh.h"
#include "imstkCollisionDetectionAlgorithm.h"
#include "imstkPointSet.h"

namespace imstk
{
CellPicker::CellPicker()
{
    setRequiredInputType<Geometry>(0);
}

void
CellPicker::requestUpdate()
{
    auto geomToPick = std::dynamic_pointer_cast<Geometry>(getInput(0));

    // Perform collision detection (not for the manifold but just the intersected elements)
    // Note: collision detection differs from intersection detection in that CD produces
    // a manifold, not the intersected elements. So not all CDs would work.
    CHECK(m_colDetect != nullptr) << "Trying to vertex pick but no collision detection provided";
    m_colDetect->setInputGeometryA(geomToPick);
    m_colDetect->setInputGeometryB(m_pickGeometry);
    m_colDetect->update();

    // When CD reports point contacts we can assume all its connected cells are
    // intersecting
    if (auto cellMesh = std::dynamic_pointer_cast<AbstractCellMesh>(geomToPick))
    {
        cellMesh->computeVertexToCellMap();
    }
    // Used to resolve duplicates
    std::unordered_map<int, PickData> resultsMap;

    const std::vector<CollisionElement>& elementsA = m_colDetect->getCollisionData()->elementsA;
    for (size_t i = 0; i < elementsA.size(); i++)
    {
        // A is always the geometry to pick
        const CollisionElement& colElemA = elementsA[i];
        if (colElemA.m_type == CollisionElementType::CellIndex)
        {
            PickData data;
            std::copy(colElemA.m_element.m_CellIndexElement.ids,
                colElemA.m_element.m_CellIndexElement.ids + 4, data.ids);
            data.idCount  = colElemA.m_element.m_CellIndexElement.idCount;
            data.cellType = colElemA.m_element.m_CellIndexElement.cellType;
            data.cellId   = colElemA.m_element.m_CellIndexElement.parentId;
            resultsMap[data.cellId] = data;
        }
        else if (colElemA.m_type == CollisionElementType::PointIndexDirection)
        {
            const int vertexId = colElemA.m_element.m_PointIndexDirectionElement.ptIndex;

            // Get the connected cells to the vertex that is intersecting
            auto                                        cellMesh = std::dynamic_pointer_cast<AbstractCellMesh>(geomToPick);
            const std::vector<std::unordered_set<int>>& vertexToCellMap = cellMesh->getVertexToCellMap();
            for (auto cellId : vertexToCellMap[vertexId])
            {
                PickData data;
                data.ids[0]   = cellId;
                data.idCount  = 1;
                data.cellType = IMSTK_EDGE;
                m_results.push_back(data);
                resultsMap[cellId] = data;
            }
        }
        else if (colElemA.m_type == CollisionElementType::PointDirection)
        {
            // A cell is not picked but point specified
            PickData data;
            data.idCount  = 0;
            data.cellType = IMSTK_VERTEX;
            // Yeilds the point on body
            data.pickPoint = colElemA.m_element.m_PointDirectionElement.pt +
                             colElemA.m_element.m_PointDirectionElement.dir *
                             colElemA.m_element.m_PointDirectionElement.penetrationDepth;
            m_results.push_back(data);
        }
        else if (colElemA.m_type == CollisionElementType::PointDirection)
        {
            // A cell is not picked but point specified
            PickData data;
            data.idCount  = 0;
            data.cellType = IMSTK_VERTEX;
            // Yeilds the point on body
            data.pickPoint = colElemA.m_element.m_PointDirectionElement.pt +
                             colElemA.m_element.m_PointDirectionElement.dir *
                             colElemA.m_element.m_PointDirectionElement.penetrationDepth;
            m_results.push_back(data);
        }
    }
    for (auto pickData : resultsMap)
    {
        m_results.push_back(pickData.second);
    }
}
} // namespace imstk