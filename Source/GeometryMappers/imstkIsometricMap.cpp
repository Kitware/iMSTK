/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkIsometricMap.h"
#include "imstkGeometry.h"
#include "imstkLogger.h"

namespace imstk
{
void
IsometricMap::requestUpdate()
{
    // Decompose when using offset
    if (m_useOffset)
    {
        Vec3d        position    = Vec3d::Zero();
        Mat3d        orientation = Mat3d::Identity();
        Vec3d        scale       = Vec3d::Ones();
        const Mat4d& transform   = getParentGeometry()->getTransform();
        mat4dTRS(transform, position, orientation, scale);

        getChildGeometry()->setTransform(
            mat4dTranslate(position + orientation * m_localOffset) *
            mat4dRotation(orientation));
    }
    else
    {
        getChildGeometry()->setTransform(getParentGeometry()->getTransform());
    }
}
} // namespace imstk