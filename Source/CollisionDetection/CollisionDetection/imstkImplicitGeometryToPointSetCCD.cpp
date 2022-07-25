/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkImplicitGeometryToPointSetCCD.h"
#include "imstkCollisionData.h"
#include "imstkImageData.h"
#include "imstkParallelUtils.h"
#include "imstkSurfaceMesh.h"

namespace imstk
{
static bool
findFirstRoot(std::shared_ptr<ImplicitGeometry> implicitGeomA, const Vec3d& start, const Vec3d& end, Vec3d& root)
{
    const Vec3d displacement = end - start;

    Vec3d  currPos  = start;
    Vec3d  prevPos  = start;
    double currDist = implicitGeomA->getFunctionValue(start);
    //double prevDist = currDist;

    // Root find (could be multiple roots, we want the first, so start march from front)
    // Gradient could be used for SDFs to converge faster but not for levelsets
    const double stepRatio  = 0.01;    // 100/5=20, this will fail if object moves 20xwidth of the object
    const double length     = displacement.norm();
    const double stepLength = length * stepRatio;
    const Vec3d  dir = displacement * (1.0 / length);
    for (double x = stepLength; x < length; x += stepLength)
    {
        prevPos = currPos;
        currPos = start + dir * x;

        //prevDist = currDist;
        currDist = implicitGeomA->getFunctionValue(currPos);

        if (currDist <= 0.0)
        {
            // Pick midpoint
            root = (prevPos + currPos) * 0.5;
            return true;
        }
    }
    return false;
}

ImplicitGeometryToPointSetCCD::ImplicitGeometryToPointSetCCD()
{
    setRequiredInputType<ImplicitGeometry>(0);
    setRequiredInputType<PointSet>(1);
}

void
ImplicitGeometryToPointSetCCD::setupFunctions(std::shared_ptr<ImplicitGeometry> implicitGeom, std::shared_ptr<PointSet> pointSet)
{
    m_centralGrad.setFunction(implicitGeom);
    if (auto sdf = std::dynamic_pointer_cast<SignedDistanceField>(implicitGeom))
    {
        m_centralGrad.setDx(sdf->getImage()->getSpacing());
    }

    // If the point set does not have displacements (or has them but not the right type), add them
    m_displacementsPtr = std::dynamic_pointer_cast<VecDataArray<double, 3>>(pointSet->getVertexAttribute("displacements"));
    if (m_displacementsPtr == nullptr)
    {
        m_displacementsPtr = std::make_shared<VecDataArray<double, 3>>(pointSet->getNumVertices());
        pointSet->setVertexAttribute("displacements", m_displacementsPtr);
        m_displacementsPtr->fill(Vec3d::Zero());
    }
}

void
ImplicitGeometryToPointSetCCD::computeCollisionDataAB(
    std::shared_ptr<Geometry>      geomA,
    std::shared_ptr<Geometry>      geomB,
    std::vector<CollisionElement>& elementsA,
    std::vector<CollisionElement>& elementsB)
{
    std::shared_ptr<ImplicitGeometry> implicitGeom = std::dynamic_pointer_cast<ImplicitGeometry>(geomA);
    std::shared_ptr<PointSet>         pointSet     = std::dynamic_pointer_cast<PointSet>(geomB);
    setupFunctions(implicitGeom, pointSet);

    // We are going to try to catch a contact before updating via marching along
    // the displacements of every point in the mesh

    // First we project the mesh to the next timepoint (without collision)
    const VecDataArray<double, 3>& displacements = *m_displacementsPtr;

    std::shared_ptr<VecDataArray<double, 3>> vertexData = pointSet->getVertexPositions();
    const VecDataArray<double, 3>&           vertices   = *vertexData; // Vertices in tentative state

    for (int i = 0; i < vertices.size(); i++)
    {
        const Vec3d& pt = vertices[i];
        const Vec3d& displacement = displacements[i];
        const double limit  = displacement.norm() * m_depthRatioLimit;
        const Vec3d  prevPt = pt - displacement;

        Vec3d  prevPos      = prevPt;
        double prevDist     = implicitGeom->getFunctionValue(prevPos);
        bool   prevIsInside = std::signbit(prevDist);

        Vec3d  currPos      = pt;
        double currDist     = implicitGeom->getFunctionValue(currPos);
        bool   currIsInside = std::signbit(currDist);

        // If both inside
        if (prevIsInside && currIsInside)
        {
            if (m_prevOuterElementCounter[i] > 0)
            {
                // Static or persistant
                m_prevOuterElementCounter[i]++;

                const Vec3d start     = m_prevOuterElement[i]; // The last outside point in its movement history
                const Vec3d end       = pt;
                Vec3d       contactPt = Vec3d::Zero();
                if (findFirstRoot(implicitGeom, start, end, contactPt))
                {
                    const Vec3d  n     = -m_centralGrad(contactPt).normalized();
                    const double depth = std::max(0.0, (contactPt - end).dot(n));

                    if (depth <= limit)
                    {
                        PointDirectionElement elemA;
                        elemA.dir = n;
                        elemA.pt  = pt;
                        elemA.penetrationDepth = depth;

                        PointIndexDirectionElement elemB;
                        elemB.dir     = -n;
                        elemB.ptIndex = i;
                        elemB.penetrationDepth = depth;

                        elementsA.push_back(elemA);
                        elementsB.push_back(elemB);
                    }
                }
            }
        }
        // If it just entered
        else if (!prevIsInside && currIsInside)
        {
            const Vec3d start     = prevPt;
            const Vec3d end       = pt;
            Vec3d       contactPt = Vec3d::Zero();
            if (findFirstRoot(implicitGeom, start, end, contactPt))
            {
                const Vec3d  n     = -m_centralGrad(contactPt).normalized();
                const double depth = std::max(0.0, (contactPt - end).dot(n));

                if (depth <= limit)
                {
                    PointDirectionElement elemA;
                    elemA.dir = n;
                    elemA.pt  = pt;
                    elemA.penetrationDepth = depth;

                    PointIndexDirectionElement elemB;
                    elemB.dir     = -n;
                    elemB.ptIndex = i;
                    elemB.penetrationDepth = depth;

                    elementsA.push_back(elemA);
                    elementsB.push_back(elemB);
                }
                m_prevOuterElementCounter[i] = 1;
                // Store the previous exterior point
                m_prevOuterElement[i] = start;
            }
            else
            {
                m_prevOuterElementCounter[i] = 0;
            }
        }
        else
        {
            m_prevOuterElementCounter[i] = 0;
        }
    }
}

void
ImplicitGeometryToPointSetCCD::computeCollisionDataA(
    std::shared_ptr<Geometry>      geomA,
    std::shared_ptr<Geometry>      geomB,
    std::vector<CollisionElement>& elementsA)
{
    // Note: Duplicate of AB, but does not add one side to avoid inner loop branching
    std::shared_ptr<ImplicitGeometry> implicitGeom = std::dynamic_pointer_cast<ImplicitGeometry>(geomA);
    std::shared_ptr<PointSet>         pointSet     = std::dynamic_pointer_cast<PointSet>(geomB);
    setupFunctions(implicitGeom, pointSet);

    const VecDataArray<double, 3>& displacements = *m_displacementsPtr;

    std::shared_ptr<VecDataArray<double, 3>> vertexData = pointSet->getVertexPositions();
    const VecDataArray<double, 3>&           vertices   = *vertexData; // Vertices in tentative state

    for (int i = 0; i < vertices.size(); i++)
    {
        const Vec3d& pt = vertices[i];
        const Vec3d& displacement = displacements[i];
        const double limit  = displacement.norm() * m_depthRatioLimit;
        const Vec3d  prevPt = pt - displacement;

        Vec3d  prevPos      = prevPt;
        double prevDist     = implicitGeom->getFunctionValue(prevPos);
        bool   prevIsInside = std::signbit(prevDist);

        Vec3d  currPos      = pt;
        double currDist     = implicitGeom->getFunctionValue(currPos);
        bool   currIsInside = std::signbit(currDist);

        // If both inside
        if (prevIsInside && currIsInside)
        {
            if (m_prevOuterElementCounter[i] > 0)
            {
                // Static or persistant
                m_prevOuterElementCounter[i]++;

                const Vec3d start     = m_prevOuterElement[i]; // The last outside point in its movement history
                const Vec3d end       = pt;
                Vec3d       contactPt = Vec3d::Zero();
                if (findFirstRoot(implicitGeom, start, end, contactPt))
                {
                    const Vec3d  n     = -m_centralGrad(contactPt).normalized();
                    const double depth = std::max(0.0, (contactPt - end).dot(n));

                    if (depth <= limit)
                    {
                        PointDirectionElement elemA;
                        elemA.dir = n;
                        elemA.pt  = pt;
                        elemA.penetrationDepth = depth;

                        elementsA.push_back(elemA);
                    }
                }
            }
        }
        // If it just entered
        else if (!prevIsInside && currIsInside)
        {
            const Vec3d start     = prevPt;
            const Vec3d end       = pt;
            Vec3d       contactPt = Vec3d::Zero();
            if (findFirstRoot(implicitGeom, start, end, contactPt))
            {
                const Vec3d  n     = -m_centralGrad(contactPt).normalized();
                const double depth = std::max(0.0, (contactPt - end).dot(n));

                if (depth <= limit)
                {
                    PointDirectionElement elemA;
                    elemA.dir = n;
                    elemA.pt  = pt;
                    elemA.penetrationDepth = depth;

                    elementsA.push_back(elemA);
                }
                m_prevOuterElementCounter[i] = 1;
                // Store the previous exterior point
                m_prevOuterElement[i] = start;
            }
            else
            {
                m_prevOuterElementCounter[i] = 0;
            }
        }
        else
        {
            m_prevOuterElementCounter[i] = 0;
        }
    }
}

void
ImplicitGeometryToPointSetCCD::computeCollisionDataB(
    std::shared_ptr<Geometry>      geomA,
    std::shared_ptr<Geometry>      geomB,
    std::vector<CollisionElement>& elementsB)
{
    // Note: Duplicate of AB, but does not add one side to avoid inner loop branching
    std::shared_ptr<ImplicitGeometry> implicitGeom = std::dynamic_pointer_cast<ImplicitGeometry>(geomA);
    std::shared_ptr<PointSet>         pointSet     = std::dynamic_pointer_cast<PointSet>(geomB);
    setupFunctions(implicitGeom, pointSet);

    const VecDataArray<double, 3>& displacements = *m_displacementsPtr;

    std::shared_ptr<VecDataArray<double, 3>> vertexData = pointSet->getVertexPositions();
    const VecDataArray<double, 3>&           vertices   = *vertexData; // Vertices in tentative state

    for (int i = 0; i < vertices.size(); i++)
    {
        const Vec3d& pt = vertices[i];
        const Vec3d& displacement = displacements[i];
        const double limit  = displacement.norm() * m_depthRatioLimit;
        const Vec3d  prevPt = pt - displacement;

        Vec3d  prevPos      = prevPt;
        double prevDist     = implicitGeom->getFunctionValue(prevPos);
        bool   prevIsInside = std::signbit(prevDist);

        Vec3d  currPos      = pt;
        double currDist     = implicitGeom->getFunctionValue(currPos);
        bool   currIsInside = std::signbit(currDist);

        // If both inside
        if (prevIsInside && currIsInside)
        {
            if (m_prevOuterElementCounter[i] > 0)
            {
                // Static or persistant
                m_prevOuterElementCounter[i]++;

                const Vec3d start     = m_prevOuterElement[i]; // The last outside point in its movement history
                const Vec3d end       = pt;
                Vec3d       contactPt = Vec3d::Zero();
                if (findFirstRoot(implicitGeom, start, end, contactPt))
                {
                    const Vec3d  n     = -m_centralGrad(contactPt).normalized();
                    const double depth = std::max(0.0, (contactPt - end).dot(n));

                    if (depth <= limit)
                    {
                        PointIndexDirectionElement elemB;
                        elemB.dir     = -n;
                        elemB.ptIndex = i;
                        elemB.penetrationDepth = depth;

                        elementsB.push_back(elemB);
                    }
                }
            }
        }
        // If it just entered
        else if (!prevIsInside && currIsInside)
        {
            const Vec3d start     = prevPt;
            const Vec3d end       = pt;
            Vec3d       contactPt = Vec3d::Zero();
            if (findFirstRoot(implicitGeom, start, end, contactPt))
            {
                const Vec3d  n     = -m_centralGrad(contactPt).normalized();
                const double depth = std::max(0.0, (contactPt - end).dot(n));

                if (depth <= limit)
                {
                    PointIndexDirectionElement elemB;
                    elemB.dir     = -n;
                    elemB.ptIndex = i;
                    elemB.penetrationDepth = depth;

                    elementsB.push_back(elemB);
                }
                m_prevOuterElementCounter[i] = 1;
                // Store the previous exterior point
                m_prevOuterElement[i] = start;
            }
            else
            {
                m_prevOuterElementCounter[i] = 0;
            }
        }
        else
        {
            m_prevOuterElementCounter[i] = 0;
        }
    }
}
} // namespace imstk