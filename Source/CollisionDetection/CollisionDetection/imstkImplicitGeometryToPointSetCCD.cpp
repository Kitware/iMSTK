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

#include "imstkImplicitGeometryToPointSetCCD.h"
#include "imstkCollisionData.h"
#include "imstkImageData.h"
#include "imstkSurfaceMesh.h"

namespace imstk
{
ImplicitGeometryToPointSetCCD::ImplicitGeometryToPointSetCCD(std::shared_ptr<ImplicitGeometry> implicitGeomA,
                                                             std::shared_ptr<PointSet>         pointSetB,
                                                             std::shared_ptr<CollisionData>    colData) :
    CollisionDetection(CollisionDetection::Type::PointSetToImplicitCCD, colData),
    m_implicitGeomA(implicitGeomA),
    m_pointSetB(pointSetB)
{
    centralGrad.setFunction(m_implicitGeomA);
    if (auto sdf = std::dynamic_pointer_cast<SignedDistanceField>(m_implicitGeomA))
    {
        centralGrad.setDx(sdf->getImage()->getSpacing());
    }

    displacementsPtr = std::make_shared<VecDataArray<double, 3>>(static_cast<int>(m_pointSetB->getNumVertices()));
    m_pointSetB->setVertexAttribute("displacements", displacementsPtr);
}

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
    const double stepRatio  = 0.01; // 100/5=20, this will fail if object moves 20xwidth of the object
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

void
ImplicitGeometryToPointSetCCD::computeCollisionData()
{
    m_colData->clearAll();

    // We are going to try to catch a contact before updating via marching along
    // the displacements of every point in the mesh

    // First we project the mesh to the next timepoint (without collision)

    if (!m_pointSetB->hasVertexAttribute("displacements"))
    {
        LOG(WARNING) << "ImplicitGeometry to PointSet CCD can't function without displacements on geometry";
        return;
    }
    const VecDataArray<double, 3>& displacements = *displacementsPtr;

    std::shared_ptr<VecDataArray<double, 3>> vertexData = m_pointSetB->getVertexPositions();
    const VecDataArray<double, 3>&           vertices   = *vertexData; // Vertices in tentative state

    for (int i = 0; i < vertices.size(); i++)
    {
        const Vec3d& pt = vertices[i];
        const Vec3d& displacement = displacements[i];
        const double limit  = displacement.norm() * m_depthRatioLimit;
        const Vec3d  prevPt = pt - displacement;

        Vec3d  prevPos      = prevPt;
        double prevDist     = m_implicitGeomA->getFunctionValue(prevPos);
        bool   prevIsInside = std::signbit(prevDist);

        Vec3d  currPos      = pt;
        double currDist     = m_implicitGeomA->getFunctionValue(currPos);
        bool   currIsInside = std::signbit(currDist);

        // If both inside
        if (prevIsInside && currIsInside)
        {
            if (prevOuterElementCounter[i] > 0)
            {
                // Static or persistant
                prevOuterElementCounter[i]++;

                const Vec3d start     = prevOuterElement[i]; // The last outside point in its movement history
                const Vec3d end       = pt;
                Vec3d       contactPt = Vec3d::Zero();
                if (findFirstRoot(m_implicitGeomA, start, end, contactPt))
                {
                    PositionDirectionCollisionDataElement elem;
                    elem.dirAtoB = -centralGrad(contactPt).normalized(); // -centralGrad gives Outward facing contact normal
                    elem.nodeIdx = static_cast<uint32_t>(i);
                    elem.penetrationDepth = std::max(0.0, (contactPt - end).dot(elem.dirAtoB));
                    if (elem.penetrationDepth <= limit)
                    {
                        // Could be useful to find the closest point on the shape, as reprojected
                        // contact points could be outside
                        elem.posB = contactPt;
                        m_colData->PDColData.safeAppend(elem);
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
            if (findFirstRoot(m_implicitGeomA, start, end, contactPt))
            {
                PositionDirectionCollisionDataElement elem;
                elem.dirAtoB = -centralGrad(contactPt).normalized(); // -centralGrad gives Outward facing contact normal
                elem.nodeIdx = static_cast<uint32_t>(i);
                elem.penetrationDepth = std::max(0.0, (contactPt - end).dot(elem.dirAtoB));
                if (elem.penetrationDepth <= limit)
                {
                    elem.posB = contactPt;
                    m_colData->PDColData.safeAppend(elem);
                }
                prevOuterElementCounter[i] = 1;
                // Store the previous exterior point
                prevOuterElement[i] = start;
            }
            else
            {
                prevOuterElementCounter[i] = 0;
            }
        }
        else
        {
            prevOuterElementCounter[i] = 0;
        }
    }
}
}