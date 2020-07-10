///*=========================================================================
//
//   Library: iMSTK
//
//   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
//   & Imaging in Medicine, Rensselaer Polytechnic Institute.
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0.txt
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.
//
//=========================================================================*/
//
//#include "imstkSurfaceMeshTextureProjection.h"
//#include "imstkSurfaceMesh.h"
//
//namespace imstk
//{
//static bool intersectPlaneLine(const Vec3d& n, const Vec3d& planePt, const Vec3d& raySrc, const Vec3d& rayDir, Vec3d& intersectionPt)
//{
//    float denom = n.dot(rayDir);
//    // Line is tangent, or nearly
//    if (std::abs(denom) < 0.000001f)
//        return false;
//
//    float t = (planePt - raySrc).dot(n) / denom;
//    // Starts outside of plane
//   /* if (t < 0.0f)
//        return false;*/
//    intersectionPt = raySrc + t * rayDir;
//    return true;
//}
//
//static Vec3d baryCentric(const Vec3d& p, const Vec3d& a, const Vec3d& b, const Vec3d& c)
//{
//    Vec3d v0 = b - a;
//    Vec3d v1 = c - a;
//    Vec3d v2 = p - a;
//    const double d00 = v0.dot(v0);
//    const double d01 = v0.dot(v1);
//    const double d11 = v1.dot(v1);
//    const double d20 = v2.dot(v0);
//    const double d21 = v2.dot(v1);
//    const double invDenom = 1.0f / (d00 * d11 - d01 * d01);
//    const double v = (d11 * d20 - d01 * d21) * invDenom;
//    const double w = (d00 * d21 - d01 * d20) * invDenom;
//    const double u = 1.0f - v - w;
//    return Vec3d(u, v, w);
//}
//
//SurfaceMeshTextureProjection::SurfaceMeshTextureProjection()
//{
//    setNumberOfInputPorts(2);
//    setNumberOfOutputPorts(1);
//    setOutput(std::make_shared<SurfaceMesh>());
//}
//
//void SurfaceMeshTextureProjection::setSourceMesh(std::shared_ptr<SurfaceMesh> srcMesh) { setInput(srcMesh, 0); }
//
//void SurfaceMeshTextureProjection::setDestMesh(std::shared_ptr<SurfaceMesh> destMesh) { setInput(destMesh, 1); }
//
//void SurfaceMeshTextureProjection::requestUpdate()
//{
//    std::shared_ptr<SurfaceMesh> sourceMesh = std::dynamic_pointer_cast<SurfaceMesh>(getInput(0));
//    const StdVectorOfVec3d& srcVertices = sourceMesh->getVertexPositions();
//    const std::vector<SurfaceMesh::TriangleArray>& srcFaces = sourceMesh->getTrianglesVertices();
//    const StdVectorOfVectorf& srcTCoords = *sourceMesh->getPointDataArray(sourceMesh->getDefaultTCoords());
//
//    std::shared_ptr<SurfaceMesh> destMesh = std::dynamic_pointer_cast<SurfaceMesh>(getInput(1));
//    std::shared_ptr<SurfaceMesh> results = std::make_shared<SurfaceMesh>(destMesh);
//    results->computeVertexNormals();
//    const StdVectorOfVec3d& destVertices = results->getVertexPositions();
//    const StdVectorOfVec3d& destVertexNormals = results->getVertexNormals();
//    StdVectorOfVectorf& destTCoords = *results->getPointDataArray(results->getDefaultTCoords());
//
//
//    // For every vertex of dest
//    for (size_t i = 0; i < destVertices.size(); i++)
//    {
//        // Emit a ray along the normal
//        const Vec3d& pt = destVertices[i];
//        const Vec3d& dir = destVertexNormals[i];
//
//        // Test intersection with every face of other SurfaceMesh
//        for (size_t j = 0; j < srcFaces.size(); j++)
//        {
//            const size_t i1 = srcFaces[j][0];
//            const size_t i2 = srcFaces[j][1];
//            const size_t i3 = srcFaces[j][2];
//            const Vec3d& p1 = srcVertices[i1];
//            const Vec3d& p2 = srcVertices[i2];
//            const Vec3d& p3 = srcVertices[i3];
//            const Vec3d n = (p2 - p1).cross(p2 - p3).normalized();
//
//            Vec3d iPt;
//
//            // For every face of sourceMesh
//            if (intersectPlaneLine(n, pt, pt, dir, iPt))
//            {
//                Vec3d bCoords = baryCentric(iPt, p1, p2, p3);
//                if (bCoords[0] >= 0.0f && bCoords[1] >= 0.0f && bCoords[2] >= 0.0f)
//                {
//                    // Intersecting
//                    // Now sample the texture coordinates using barycentric interpolation
//
//                    const Vec2d& tCoord1 = destTCoords[i1];
//                    const Vec2d& tCoord2 = destTCoords[i2];
//                    const Vec2d& tCoord3 = destTCoords[i3];
//
//                    Vec2d results = tCoord1 * bCoords[0] + tCoord1 * bCoords[1] + tCoord2 * bCoords[2];
//                }
//            }
//        }
//    }
//
//    setOutput(results);
//}
//}