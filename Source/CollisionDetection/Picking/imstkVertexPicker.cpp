/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkVertexPicker.h"
#include "imstkImplicitGeometry.h"
#include "imstkPointSet.h"
#include "imstkVecDataArray.h"

namespace imstk
{
VertexPicker::VertexPicker()
{
    setRequiredInputType<PointSet>(0);
}

void
VertexPicker::requestUpdate()
{
    auto pointSetToPick = std::dynamic_pointer_cast<PointSet>(getInput(0));

    // Use implicit functions available in the geometries to sample if in or out of the shape
    std::shared_ptr<VecDataArray<double, 3>> verticesPtr = pointSetToPick->getVertexPositions();
    VecDataArray<double, 3>&                 vertices    = *verticesPtr;
    for (int i = 0; i < vertices.size(); i++)
    {
        const double signedDist = m_pickGeometry->getFunctionValue(vertices[i]);

        // If inside the primitive
        // \todo: come back to this
        if (signedDist <= 0.0)
        {
            PickData data;
            data.ids[0]   = i;
            data.idCount  = 1;
            data.cellType = IMSTK_VERTEX;
            m_results.push_back(data);
        }
    }
}
} // namespace imstk