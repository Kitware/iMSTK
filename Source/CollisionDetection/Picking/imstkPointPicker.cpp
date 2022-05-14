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

#include "imstkPointPicker.h"
#include "imstkCollisionUtils.h"
#include "imstkLineMesh.h"
#include "imstkOrientedBox.h"
#include "imstkPlane.h"
#include "imstkSphere.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkVecDataArray.h"

#include <set>

namespace imstk
{
void
PointPicker::requestUpdate()
{
    // Use for sorting
    // \todo: Could also parameterize by distance to avoid recomputation
    //  if dealing with many intersections
    auto pred = [&](const PickData& a, const PickData& b)
                {
                    const double sqrDistA = (a.pickPoint - m_rayStart).squaredNorm();
                    const double sqrDistB = (b.pickPoint - m_rayStart).squaredNorm();
                    return sqrDistA < sqrDistB;
                };
    std::set<PickData, decltype(pred)> resultSet(pred);

    std::shared_ptr<Geometry> geomToPick = getInput(0);
    geomToPick->updatePostTransformData();
    if (auto surfMeshToPick = std::dynamic_pointer_cast<SurfaceMesh>(geomToPick))
    {
        std::shared_ptr<VecDataArray<double, 3>> verticesPtr = surfMeshToPick->getVertexPositions();
        const VecDataArray<double, 3>&           vertices    = *verticesPtr;
        std::shared_ptr<VecDataArray<int, 3>>    indicesPtr  = surfMeshToPick->getTriangleIndices();
        const VecDataArray<int, 3>&              indices     = *indicesPtr;

        // Brute force
        // For every cell
        for (int i = 0; i < indices.size(); i++)
        {
            const Vec3i& cell = indices[i];
            const Vec3d& a    = vertices[cell[0]];
            const Vec3d& b    = vertices[cell[1]];
            const Vec3d& c    = vertices[cell[2]];
            const Vec3d  n    = (b - a).cross(c - a).normalized();
            Vec3d        iPt  = Vec3d::Zero();
            if (CollisionUtils::testRayToPlane(m_rayStart, m_rayDir, vertices[cell[0]], n, iPt))
            {
                const Vec3d uvw = baryCentric(iPt, a, b, c);
                if (uvw[0] >= 0.0 && uvw[1] >= 0.0 && uvw[2] >= 0.0) // Check if within triangle
                {
                    resultSet.insert({ { i }, 1, IMSTK_TRIANGLE, iPt });
                }
            }
        }
    }
    else if (auto tetMeshToPick = std::dynamic_pointer_cast<TetrahedralMesh>(geomToPick))
    {
        // Current implementation just based off the triangle faces
        static int faces[4][3] = { { 0, 1, 2 }, { 1, 2, 3 }, { 0, 2, 3 }, { 0, 1, 3 } };

        std::shared_ptr<VecDataArray<double, 3>> verticesPtr = tetMeshToPick->getVertexPositions();
        const VecDataArray<double, 3>&           vertices    = *verticesPtr;
        std::shared_ptr<VecDataArray<int, 4>>    indicesPtr  = tetMeshToPick->getTetrahedraIndices();
        const VecDataArray<int, 4>&              indices     = *indicesPtr;

        // For every tet
        for (int i = 0; i < indices.size(); i++)
        {
            const Vec4i& tet = indices[i];

            // For every face
            for (int j = 0; j < 4; j++)
            {
                // Find intersection point and add constraints
                const Vec3d& a = vertices[tet[faces[j][0]]];
                const Vec3d& b = vertices[tet[faces[j][1]]];
                const Vec3d& c = vertices[tet[faces[j][2]]];

                Vec3d iPt;
                if (CollisionUtils::testRayToPlane(m_rayStart, m_rayDir, a,
                    (b - a).cross(c - a).normalized(), iPt))
                {
                    const Vec3d uvw = baryCentric(iPt, a, b, c);
                    if (uvw[0] >= 0.0 && uvw[1] >= 0.0 && uvw[2] >= 0.0) // Check if within triangle
                    {
                        resultSet.insert({ { i }, 1, IMSTK_TETRAHEDRON, iPt });
                    }
                }
            }
        }
    }
    else if (auto lineMeshToPick = std::dynamic_pointer_cast<LineMesh>(geomToPick))
    {
        // Requires a thickness
        LOG(FATAL) << "LineMesh picking not implemented yet";
    }
    else if (auto sphereToPick = std::dynamic_pointer_cast<Sphere>(geomToPick))
    {
        Vec3d iPt = Vec3d::Zero();
        if (CollisionUtils::testRayToSphere(m_rayStart, m_rayDir,
            sphereToPick->getPosition(), sphereToPick->getRadius(), iPt))
        {
            resultSet.insert({ { 0 }, 1, IMSTK_VERTEX, iPt });
            // \todo: Exit point
        }
    }
    else if (auto planeToPick = std::dynamic_pointer_cast<Plane>(geomToPick))
    {
        Vec3d iPt = Vec3d::Zero();
        if (CollisionUtils::testRayToPlane(m_rayStart, m_rayDir,
            planeToPick->getPosition(), planeToPick->getNormal(), iPt))
        {
            resultSet.insert({ { 0 }, 1, IMSTK_VERTEX, iPt });
        }
    }
    //else if (auto capsuleToPick = std::dynamic_pointer_cast<Capsule>(geomToPick))
    //{
    //}
    else if (auto obbToPick = std::dynamic_pointer_cast<OrientedBox>(geomToPick))
    {
        Mat4d worldToBox = mat4dTranslate(obbToPick->getPosition()) *
                           mat4dRotation(obbToPick->getOrientation());
        Vec2d t = Vec2d::Zero(); // Entry and exit t
        if (CollisionUtils::testRayToObb(m_rayStart, m_rayDir,
            worldToBox.inverse(), obbToPick->getExtents(), t))
        {
            resultSet.insert({ { 0 }, 1, IMSTK_VERTEX, m_rayStart + m_rayDir * t[0] });
            resultSet.insert({ { 1 }, 1, IMSTK_VERTEX, m_rayStart + m_rayDir * t[1] });
        }
    }
    else if (auto implicitGeom = std::dynamic_pointer_cast<ImplicitGeometry>(geomToPick))
    {
        // Implicit primitives such as capsule should get caught here if the
        // above analytic solution is not provided. SDFs as well
        // Only works with bounded geometries

        // Find the intersection point on the oriented box
        Vec3d min, max;
        implicitGeom->computeBoundingBox(min, max);
        const Vec3d  center     = (min + max) * 0.5;
        const Vec3d  extents    = (max - min) * 0.5; // Half the size
        const double size       = extents.norm() * 2.0;
        const double stepLength = size / 50.0;

        const Mat4d boxTransform = mat4dTranslate(center);
        Vec2d       tPt = Vec2d::Zero(); // Entry and exit t
        if (CollisionUtils::testRayToObb(m_rayStart, m_rayDir, boxTransform.inverse(), extents, tPt))
        {
            // If it hit, start iterating from this point on the box in the implicit geometry
            Vec3d iPt = m_rayStart + m_rayDir * tPt[0];

            // For implicit geometry this isn't always signed distance
            double currDist = IMSTK_DOUBLE_MAX;
            Vec3d  currPt   = iPt;
            double prevDist = IMSTK_DOUBLE_MAX;
            Vec3d  prevPt   = iPt;
            for (int i = 0; i < 50; i++)
            {
                // Push back state
                prevDist = currDist;
                prevPt   = currPt;

                // Compute new pt
                const double t = static_cast<double>(i) / 50.0;
                currPt   = iPt + t * m_rayDir * stepLength;
                currDist = implicitGeom->getFunctionValue(currPt);

                // If the sign changed
                if (std::signbit(currDist) != std::signbit(prevDist))
                {
                    // Use midpoint of results
                    iPt = (currPt + prevPt) * 0.5;
                    resultSet.insert({ { 0 }, 1, IMSTK_VERTEX, iPt });
                }
            }
        }
    }
    else
    {
        LOG(FATAL) << "Tried to point pick with an unsupported geometry " << geomToPick->getTypeName();
        return;
    }

    // Only select the first hit that is within max distance
    const double maxSqrDist = m_maxDist * m_maxDist;
    const bool   useMaxDist = (m_maxDist != -1.0);
    if (m_useFirstHit)
    {
        // Start at max, unless you get under this it won't select
        double   minSqrDist = useMaxDist ? maxSqrDist : IMSTK_DOUBLE_MAX;
        PickData results;
        bool     resultsFound = false;
        for (const auto& pickData : resultSet)
        {
            // Possibly parameterize all by t and use that here instead
            const double sqrDist = (pickData.pickPoint - m_rayStart).squaredNorm();
            if (sqrDist <= minSqrDist)
            {
                results      = pickData;
                resultsFound = true;
                minSqrDist   = sqrDist;
            }
        }

        if (resultsFound)
        {
            m_results.resize(1);
            m_results[0] = results;
        }
    }
    else
    {
        m_results = std::vector<PickData>();
        for (auto pickData : resultSet)
        {
            const double sqrDist = (pickData.pickPoint - m_rayStart).squaredNorm();
            if (!useMaxDist || sqrDist <= maxSqrDist)
            {
                m_results.push_back(pickData);
            }
        }
    }
}
} // namespace imstk