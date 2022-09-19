/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "NeedleObject.h"
#include "imstkPointSet.h"

using namespace imstk;

const Vec3d
NeedleObject::getNeedleAxes() const
{
    auto                                     colGeom     = std::dynamic_pointer_cast<PointSet>(getCollidingGeometry());
    std::shared_ptr<VecDataArray<double, 3>> verticesPtr = colGeom->getVertexPositions();
    const VecDataArray<double, 3>&           vertices    = *verticesPtr;
    CHECK(vertices.size() == 2) << "Assumes straight single segment needle";
    return (vertices[0] - vertices[1]).normalized();
}