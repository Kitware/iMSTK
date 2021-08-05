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

#include "imstkImplicitGeometryToPointSetCD.h"
#include "imstkCollisionData.h"
#include "imstkImageData.h"
#include "imstkImplicitGeometry.h"
#include "imstkMath.h"
#include "imstkPointSet.h"
#include "imstkSignedDistanceField.h"

namespace imstk
{
ImplicitGeometryToPointSetCD::ImplicitGeometryToPointSetCD()
{
    setRequiredInputType<ImplicitGeometry>(0);
    setRequiredInputType<PointSet>(1);
}

void
ImplicitGeometryToPointSetCD::computeCollisionDataAB(
    std::shared_ptr<Geometry>          geomA,
    std::shared_ptr<Geometry>          geomB,
    CDElementVector<CollisionElement>& elementsA,
    CDElementVector<CollisionElement>& elementsB)
{
    auto implicitGeom = std::dynamic_pointer_cast<ImplicitGeometry>(geomA);
    auto pointSet     = std::dynamic_pointer_cast<PointSet>(geomB);

    m_centralGrad.setFunction(implicitGeom);
    if (auto sdf = std::dynamic_pointer_cast<SignedDistanceField>(implicitGeom))
    {
        m_centralGrad.setDx(sdf->getImage()->getSpacing() * 0.5);
    }

    std::shared_ptr<VecDataArray<double, 3>> verticesPtr = pointSet->getVertexPositions();
    const VecDataArray<double, 3>&           vertices    = *verticesPtr;
    ParallelUtils::parallelFor(vertices.size(),
        [&](const int i)
        {
            const Vec3d& pt = vertices[i];

            const double signedDistance = implicitGeom->getFunctionValue(pt);
            if (signedDistance < 0.0)
            {
                const Vec3d n = m_centralGrad(pt).normalized(); // Contact Normal

                PointDirectionElement elemA;
                elemA.dir = -n; // Direction to resolve SDF-based object from point
                elemA.pt  = pt;
                elemA.penetrationDepth = std::abs(signedDistance);

                PointIndexDirectionElement elemB;
                elemB.dir     = n; // Direction to resolve point from SDF
                elemB.ptIndex = i;
                elemB.penetrationDepth = std::abs(signedDistance);

                elementsA.safeAppend(elemA);
                elementsB.safeAppend(elemB);
            }
                }, vertices.size() > 100);
}

void
ImplicitGeometryToPointSetCD::computeCollisionDataA(
    std::shared_ptr<Geometry>          geomA,
    std::shared_ptr<Geometry>          geomB,
    CDElementVector<CollisionElement>& elementsA)
{
    auto implicitGeom = std::dynamic_pointer_cast<ImplicitGeometry>(geomA);
    auto pointSet     = std::dynamic_pointer_cast<PointSet>(geomB);

    m_centralGrad.setFunction(implicitGeom);
    if (auto sdf = std::dynamic_pointer_cast<SignedDistanceField>(implicitGeom))
    {
        m_centralGrad.setDx(sdf->getImage()->getSpacing() * 0.5);
    }

    std::shared_ptr<VecDataArray<double, 3>> verticesPtr = pointSet->getVertexPositions();
    const VecDataArray<double, 3>&           vertices    = *verticesPtr;
    ParallelUtils::parallelFor(vertices.size(),
        [&](const int i)
        {
            const Vec3d& pt = vertices[i];

            const double signedDistance = implicitGeom->getFunctionValue(pt);
            if (signedDistance < 0.0)
            {
                const Vec3d n = m_centralGrad(pt).normalized(); // Contact Normal

                PointDirectionElement elemA;
                elemA.dir = -n; // Direction to resolve SDF-based object from point
                elemA.pt  = pt;
                elemA.penetrationDepth = std::abs(signedDistance);

                elementsA.safeAppend(elemA);
            }
                }, vertices.size() > 100);
}

void
ImplicitGeometryToPointSetCD::computeCollisionDataB(
    std::shared_ptr<Geometry>          geomA,
    std::shared_ptr<Geometry>          geomB,
    CDElementVector<CollisionElement>& elementsB)
{
    auto implicitGeom = std::dynamic_pointer_cast<ImplicitGeometry>(geomA);
    auto pointSet     = std::dynamic_pointer_cast<PointSet>(geomB);

    m_centralGrad.setFunction(implicitGeom);
    if (auto sdf = std::dynamic_pointer_cast<SignedDistanceField>(implicitGeom))
    {
        m_centralGrad.setDx(sdf->getImage()->getSpacing() * 0.5);
    }

    std::shared_ptr<VecDataArray<double, 3>> verticesPtr = pointSet->getVertexPositions();
    const VecDataArray<double, 3>&           vertices    = *verticesPtr;
    ParallelUtils::parallelFor(vertices.size(),
        [&](const int i)
        {
            const Vec3d& pt = vertices[i];

            const double signedDistance = implicitGeom->getFunctionValue(pt);
            if (signedDistance < 0.0)
            {
                const Vec3d n = m_centralGrad(pt).normalized(); // Contact Normal

                PointIndexDirectionElement elemB;
                elemB.dir     = n; // Direction to resolve point from SDF
                elemB.ptIndex = i;
                elemB.penetrationDepth = std::abs(signedDistance);

                elementsB.safeAppend(elemB);
            }
                }, vertices.size() > 100);
}
}