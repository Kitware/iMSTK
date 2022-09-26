/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkStraightNeedle.h"
#include "imstkLineMesh.h"

namespace imstk
{
Vec3d
StraightNeedle::getNeedleDirection() const
{
    return (-m_needleGeom->getRotation().col(1)).normalized();
}

const Vec3d&
StraightNeedle::getNeedleStart() const
{
    return (*m_needleGeom->getVertexPositions())[0];
}

const Vec3d&
StraightNeedle::getNeedleEnd() const
{
    return (*m_needleGeom->getVertexPositions())[1];
}
} // namespace imstk