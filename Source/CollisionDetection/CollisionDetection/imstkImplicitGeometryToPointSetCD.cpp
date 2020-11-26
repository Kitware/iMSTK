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
#include "imstkImplicitGeometry.h"
#include "imstkMath.h"
#include "imstkPointSet.h"

#include "imstkSignedDistanceField.h"
#include "imstkImageData.h"

namespace imstk
{
ImplicitGeometryToPointSetCD::ImplicitGeometryToPointSetCD(std::shared_ptr<ImplicitGeometry> implicitGeomA,
                                                           std::shared_ptr<PointSet>         pointSetB,
                                                           std::shared_ptr<CollisionData>    colData) :
    CollisionDetection(CollisionDetection::Type::PointSetToImplicit, colData),
    m_implicitGeomA(implicitGeomA),
    m_pointSetB(pointSetB)
{
    centralGrad.setFunction(m_implicitGeomA);
    if (m_implicitGeomA->getType() == Geometry::Type::SignedDistanceField)
    {
        centralGrad.setDx(std::dynamic_pointer_cast<SignedDistanceField>(m_implicitGeomA)->getImage()->getSpacing());
    }
}

void
ImplicitGeometryToPointSetCD::computeCollisionData()
{
    m_colData->clearAll();

    const VecDataArray<double, 3>& vertices = *m_pointSetB->getVertexPositions();
    ParallelUtils::parallelFor(vertices.size(),
        [&](const size_t idx)
        {
            const Vec3d& pt = vertices[idx];

            const double signedDistance = m_implicitGeomA->getFunctionValue(pt);
            if (signedDistance < 0.0)
            {
                PositionDirectionCollisionDataElement elem;
                elem.dirAtoB = -centralGrad(pt).normalized(); // Contact Normal
                elem.nodeIdx = static_cast<uint32_t>(idx);
                elem.penetrationDepth = std::abs(signedDistance);
                elem.posB = pt;
                m_colData->PDColData.safeAppend(elem);
            }
        });
}
}