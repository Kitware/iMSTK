/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCellPicker.h"
#include "imstkCollisionDetectionAlgorithm.h"
#include "imstkPointSet.h"

namespace imstk
{
CellPicker::CellPicker()
{
    setRequiredInputType<PointSet>(0);
}

void
CellPicker::requestUpdate()
{
    auto pointSetToPick = std::dynamic_pointer_cast<PointSet>(getInput(0));

    // Perform collision detection (not for the manifold but just the intersected elements)
    // Note: collision detection differs from intersection detection in that CD produces
    // a manifold, not the intersected elements. So not all CDs would work.
    CHECK(m_colDetect != nullptr) << "Trying to vertex pick but no collision detection provided";
    m_colDetect->setInputGeometryA(pointSetToPick);
    m_colDetect->setInputGeometryB(m_pickGeometry);
    m_colDetect->update();

    const std::vector<CollisionElement>& elementsA = m_colDetect->getCollisionData()->elementsA;
    //const std::vector<CollisionElement>& elementsB = m_colDetect->getCollisionData()->elementsB;
    for (size_t i = 0; i < elementsA.size(); i++)
    {
        // A is the mesh, B is the analytic geometry
        const CollisionElement& colElemA = elementsA[i];
        //const CollisionElement& colElemB = elementsB[i];

        if (colElemA.m_type == CollisionElementType::CellIndex)
        {
            PickData data;
            std::copy(colElemA.m_element.m_CellIndexElement.ids,
                colElemA.m_element.m_CellIndexElement.ids + 4, data.ids);
            data.idCount  = colElemA.m_element.m_CellIndexElement.idCount;
            data.cellType = colElemA.m_element.m_CellIndexElement.cellType;
            m_results.push_back(data);
        }
        else if (colElemA.m_type == CollisionElementType::PointIndexDirection)
        {
            PickData data;
            data.ids[0]   = colElemA.m_element.m_PointIndexDirectionElement.ptIndex;
            data.idCount  = 1;
            data.cellType = IMSTK_VERTEX;
            m_results.push_back(data);
        }
    }
}
} // namespace imstk