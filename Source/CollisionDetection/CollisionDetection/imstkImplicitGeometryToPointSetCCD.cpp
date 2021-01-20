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
#include "imstkImplicitGeometry.h"
#include "imstkMath.h"
#include "imstkPointSet.h"
#include "imstkSignedDistanceField.h"
#include "imstkVecDataArray.h"

namespace imstk
{
ImplicitGeometryToPointSetCCD::ImplicitGeometryToPointSetCCD(std::shared_ptr<ImplicitGeometry> implicitGeomA,
                                                             std::shared_ptr<PointSet>         pointSetB,
                                                             std::shared_ptr<CollisionData>    colData) :
    CollisionDetection(CollisionDetection::Type::PointSetToImplicit, colData),
    m_implicitGeomA(implicitGeomA),
    m_pointSetB(pointSetB)
{
    centralGrad.setFunction(m_implicitGeomA);
    if (m_implicitGeomA->getType() == Geometry::Type::SignedDistanceField)
    {
        centralGrad.setDx(std::dynamic_pointer_cast<SignedDistanceField>(m_implicitGeomA)->getImage()->getSpacing() * 0.9);
    }

    displacements = std::make_shared<VecDataArray<double, 3>>(static_cast<int>(m_pointSetB->getNumVertices()));
    m_pointSetB->setVertexAttribute("displacements", displacements);
}

void
ImplicitGeometryToPointSetCCD::computeCollisionData()
{
    m_colData->clearAll();

    if (!m_pointSetB->hasVertexAttribute("displacements"))
    {
        LOG(WARNING) << "ImplicitGeometry to PointSet CCD can't function without displacements on geometry";
        return;
    }
    const DataArray<double>& displacementsArr = *displacements;

    std::shared_ptr<VecDataArray<double, 3>> vertexData = m_pointSetB->getVertexPositions();
    const VecDataArray<double, 3>&           vertices   = *vertexData;
    ParallelUtils::parallelFor(vertices.size(),
        [&](const size_t idx)
        {
            const Vec3d& pt = vertices[idx];
            const Vec3d displacement = Vec3d(&displacementsArr[idx * 3]);
            const double length      = displacement.norm();

            Vec3d prevPos  = pt;
            Vec3d currPos  = pt;
            double prevVal = m_implicitGeomA->getFunctionValue(currPos);
            double currVal = prevVal;

            // Check if the starting point of the march is inside
            // This could happen if the collision wasn't fully resolved by the solver (common)
            if (currVal < 0.0)
            {
                PositionDirectionCollisionDataElement elem;
                elem.dirAtoB = -centralGrad(currPos).normalized(); // Contact Normal
                elem.nodeIdx = static_cast<uint32_t>(idx);
                elem.penetrationDepth = std::abs(currVal); // Could also reverse march in gradient direction
                elem.posB = currPos;
                m_colData->PDColData.safeAppend(elem);
                return; // No need to find an other root
            }

            // If we didn't start inside the object, march along the displacement
            // smapling to see if enter it
            const double stepRatio  = 0.05;
            const double stepLength = length * stepRatio;
            const Vec3d dir = displacement * (1.0 / length);
            for (double x = 0.0; x < length; x += stepLength)
            {
                const Vec3d diff = dir * x;

                prevPos = currPos;
                currPos = pt + diff;

                prevVal = currVal;
                currVal = m_implicitGeomA->getFunctionValue(currPos);

                if (currVal < 0.0)
                {
                    // Pick midpoint
                    const Vec3d contactPt = (prevPos + currPos) * 0.5;

                    PositionDirectionCollisionDataElement elem;
                    elem.dirAtoB = -centralGrad(contactPt).normalized(); // Contact Normal
                    //elem.dirAtoB = dir;
                    elem.nodeIdx = static_cast<uint32_t>(idx);
                    elem.penetrationDepth = (contactPt - pt).norm();
                    elem.posB = contactPt;
                    m_colData->PDColData.safeAppend(elem);

                    return;
                }
            }
        });
}
}