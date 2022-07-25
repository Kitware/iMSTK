/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkImplicitGeometryToPointSetCD.h"
#include "imstkCollisionData.h"
#include "imstkImageData.h"
#include "imstkImplicitGeometry.h"
#include "imstkMath.h"
#include "imstkParallelUtils.h"
#include "imstkPointSet.h"
#include "imstkSignedDistanceField.h"

namespace imstk
{
ImplicitGeometryToPointSetCD::ImplicitGeometryToPointSetCD()
{
    setRequiredInputType<ImplicitGeometry>(0);
    setRequiredInputType<PointSet>(1);
    m_centralGrad.setDx(Vec3d(0.001, 0.001, 0.001));
}

void
ImplicitGeometryToPointSetCD::computeCollisionDataAB(
    std::shared_ptr<Geometry>      geomA,
    std::shared_ptr<Geometry>      geomB,
    std::vector<CollisionElement>& elementsA,
    std::vector<CollisionElement>& elementsB)
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
    ParallelUtils::SpinLock                  lock;
    ParallelUtils::parallelFor(vertices.size(),
        [&](const int i)
        {
            const Vec3d& pt = vertices[i];

            const double signedDistance = implicitGeom->getFunctionValue(pt);
            if (signedDistance < 0.0)
            {
                const Vec3d n      = m_centralGrad(pt).normalized(); // Contact Normal
                const double depth = std::abs(signedDistance);

                PointDirectionElement elemA;
                elemA.dir = -n; // Direction to resolve SDF-based object from point
                elemA.pt  = pt + n * depth;
                elemA.penetrationDepth = depth;

                PointIndexDirectionElement elemB;
                elemB.dir     = n; // Direction to resolve point from SDF
                elemB.ptIndex = i;
                elemB.penetrationDepth = depth;

                lock.lock();
                elementsA.push_back(elemA);
                elementsB.push_back(elemB);
                lock.unlock();
            }
        }, vertices.size() > 100);
}

void
ImplicitGeometryToPointSetCD::computeCollisionDataA(
    std::shared_ptr<Geometry>      geomA,
    std::shared_ptr<Geometry>      geomB,
    std::vector<CollisionElement>& elementsA)
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
    ParallelUtils::SpinLock                  lock;
    ParallelUtils::parallelFor(vertices.size(),
        [&](const int i)
        {
            const Vec3d& pt = vertices[i];

            const double signedDistance = implicitGeom->getFunctionValue(pt);
            if (signedDistance < 0.0)
            {
                const Vec3d n      = m_centralGrad(pt).normalized(); // Contact Normal
                const double depth = std::abs(signedDistance);

                PointDirectionElement elemA;
                elemA.dir = -n; // Direction to resolve SDF-based object from point
                elemA.pt  = pt + n * depth;
                elemA.penetrationDepth = depth;

                lock.lock();
                elementsA.push_back(elemA);
                lock.unlock();
            }
        }, vertices.size() > 100);
}

void
ImplicitGeometryToPointSetCD::computeCollisionDataB(
    std::shared_ptr<Geometry>      geomA,
    std::shared_ptr<Geometry>      geomB,
    std::vector<CollisionElement>& elementsB)
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
    ParallelUtils::SpinLock                  lock;
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

                lock.lock();
                elementsB.push_back(elemB);
                lock.unlock();
            }
        }, vertices.size() > 100);
}
} // namespace imstk