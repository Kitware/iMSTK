/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkRbdConstraint.h"
#include "imstkPointSet.h"
#include "imstkVecDataArray.h"

namespace imstk
{
void
RigidBody::setInertiaFromPointSet(std::shared_ptr<PointSet> pointset, const double scale, const bool useBoundingBoxOrigin)
{
    Mat3d results;
    results.setZero();

    Vec3d centroid = Vec3d(0.0, 0.0, 0.0);
    if (useBoundingBoxOrigin)
    {
        Vec3d min, max;
        pointset->computeBoundingBox(min, max);
        centroid = (min + max) * 0.5;
    }
    std::shared_ptr<VecDataArray<double, 3>> verticesPtr = pointset->getVertexPositions(Geometry::DataType::PreTransform);
    const VecDataArray<double, 3>&           vertices    = *verticesPtr;
    for (int i = 0; i < vertices.size(); i++)
    {
        const Vec3d r = vertices[i] - centroid;
        results(0, 0) += r[1] * r[1] + r[2] * r[2];
        results(1, 1) += r[0] * r[0] + r[2] * r[2];
        results(2, 2) += r[0] * r[0] + r[1] * r[1];
        results(1, 0) += -r[0] * r[1];
        results(2, 0) += -r[0] * r[2];
        results(2, 1) += -r[1] * r[2];
    }
    results(0, 2)    = results(2, 0);
    results(0, 1)    = results(1, 0);
    results(1, 2)    = results(2, 1);
    m_intertiaTensor = results * m_mass * scale;
}
} // namespace imstk