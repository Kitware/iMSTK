/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCompositeImplicitGeometry.h"

namespace imstk
{
double
CompositeImplicitGeometry::getFunctionValue(const Vec3d& pos) const
{
    // Initialize everything to outside
    double distA = std::numeric_limits<double>::max();
    for (auto i = geometries.begin(); i != geometries.end(); i++)
    {
        const GeometryBoolPair& pair  = *i;
        double                  distB = pair.first->getFunctionValue(pos);
        switch (pair.second)
        {
        case GeometryBoolType::Difference:
            distA = std::max(distA, -distB);
            break;
        case GeometryBoolType::Intersection:
            distA = std::max(distA, distB);
            break;
        case GeometryBoolType::Union:
            distA = std::min(distA, distB);
            break;
        }
    }
    return distA;
}
} // namespace imstk