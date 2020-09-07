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

#include "imstkRbdConstraint.h"
#include "imstkPointSet.h"

namespace imstk
{
namespace expiremental
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
    const StdVectorOfVec3d& vertices = pointset->getVertexPositions(Geometry::DataType::PreTransform);
    for (size_t i = 0; i < vertices.size(); i++)
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
}
}