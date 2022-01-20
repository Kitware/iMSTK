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

#include "imstkSurfaceMeshTextureProject.h"
#include "imstkLogger.h"
#include "imstkParallelUtils.h"
#include "imstkSurfaceMesh.h"
#include "imstkVecDataArray.h"

namespace imstk
{
static Vec3d
closestPointOnTriangle(const Vec3d& p, const Vec3d& a, const Vec3d& b, const Vec3d& c, int& caseType)
{
    const Vec3d ab = b - a;
    const Vec3d ac = c - a;
    const Vec3d ap = p - a;

    const double d1 = ab.dot(ap);
    const double d2 = ac.dot(ap);
    if (d1 <= 0.0 && d2 <= 0.0)
    {
        caseType = 0;
        return a; // barycentric coordinates (1,0,0)
    }

    // Check if P in vertex region outside B
    const Vec3d  bp = p - b;
    const double d3 = ab.dot(bp);
    const double d4 = ac.dot(bp);
    if (d3 >= 0.0 && d4 <= d3)
    {
        caseType = 1;
        return b; // barycentric coordinates (0,1,0)
    }
    // Check if P in edge region of AB, if so return projection of P onto AB
    const double vc = d1 * d4 - d3 * d2;
    if (vc <= 0.0 && d1 >= 0.0 && d3 <= 0.0)
    {
        caseType = 3;
        double v = d1 / (d1 - d3);
        return a + v * ab; // barycentric coordinates (1-v,v,0)
    }

    // Check if P in vertex region outside C
    const Vec3d  cp = p - c;
    const double d5 = ab.dot(cp);
    const double d6 = ac.dot(cp);
    if (d6 >= 0.0 && d5 <= d6)
    {
        caseType = 2;
        return c; // barycentric coordinates (0,0,1)
    }

    // Check if P in edge region of AC, if so return projection of P onto AC
    const double vb = d5 * d2 - d1 * d6;
    if (vb <= 0.0 && d2 >= 0.0 && d6 <= 0.0)
    {
        caseType = 5;
        double w = d2 / (d2 - d6);
        return a + w * ac; // barycentric coordinates (1-w,0,w)
    }

    // Check if P in edge region of BC, if so return projection of P onto BC
    const double va = d3 * d6 - d5 * d4;
    if (va <= 0.0 && (d4 - d3) >= 0.0 && (d5 - d6) >= 0.0)
    {
        caseType = 4;
        double w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
        return b + w * (c - b); // barycentric coordinates (0,1-w,w)
    }

    // P inside face region. Compute Q through its barycentric coordinates (u,v,w)
    const double denom = 1.0 / (va + vb + vc);
    const double v     = vb * denom;
    const double w     = vc * denom;
    caseType = 6;
    return a + ab * v + ac * w; // = u*a + v*b + w*c, u = va * denom = 1.0f-v-w
}

template<typename T>
static T
baryInterpolate(T v1, T v2, T v3, Vec3d uvw)
{
    return v1 * uvw[0] + v2 * uvw[1] + v3 * uvw[2];
}

SurfaceMeshTextureProject::SurfaceMeshTextureProject()
{
    setNumInputPorts(2);
    setRequiredInputType<SurfaceMesh>(0);
    setRequiredInputType<SurfaceMesh>(1);

    setNumOutputPorts(1);
    setOutput(std::make_shared<SurfaceMesh>(), 0);
}

void
SurfaceMeshTextureProject::setSourceMesh(std::shared_ptr<SurfaceMesh> srcMesh)
{
    setInput(srcMesh, 0);
}

void
SurfaceMeshTextureProject::setDestMesh(std::shared_ptr<SurfaceMesh> destMesh)
{
    setInput(destMesh, 1);
}

std::shared_ptr<SurfaceMesh>
SurfaceMeshTextureProject::getOutputMesh()
{
    return std::dynamic_pointer_cast<SurfaceMesh>(getOutput(0));
}

void
SurfaceMeshTextureProject::requestUpdate()
{
    std::shared_ptr<SurfaceMesh> inputSrcMesh   = std::dynamic_pointer_cast<SurfaceMesh>(getInput(0));
    std::shared_ptr<SurfaceMesh> inputDestMesh  = std::dynamic_pointer_cast<SurfaceMesh>(getInput(1));
    std::shared_ptr<SurfaceMesh> outputDestMesh = std::dynamic_pointer_cast<SurfaceMesh>(getOutput(0));
    outputDestMesh->deepCopy(inputDestMesh);

    if (inputSrcMesh == nullptr || inputDestMesh == nullptr)
    {
        LOG(WARNING) << "Missing input surface mesh";
        return;
    }

    std::shared_ptr<VecDataArray<double, 3>> srcVerticesPtr = inputSrcMesh->getVertexPositions();
    const VecDataArray<double, 3>&           srcVertices    = *srcVerticesPtr;
    std::shared_ptr<VecDataArray<int, 3>>    srcCellsPtr    = inputSrcMesh->getTriangleIndices();
    const VecDataArray<int, 3>&              srcCells       = *srcCellsPtr;
    std::shared_ptr<VecDataArray<float, 2>>  srcTCoordsPtr  = inputSrcMesh->getVertexTCoords();
    if (srcTCoordsPtr == nullptr)
    {
        LOG(WARNING) << "inputSrcMesh does not have texture coordinates";
        return;
    }
    const VecDataArray<float, 2>& inputTexCoords = *srcTCoordsPtr;

    std::shared_ptr<VecDataArray<double, 3>> destVerticesPtr      = inputDestMesh->getVertexPositions();
    const VecDataArray<double, 3>&           destVertices         = *destVerticesPtr;
    auto                                     outputDestTCoordsPtr = std::make_shared<VecDataArray<float, 2>>(destVertices.size());
    outputDestMesh->setVertexTCoords(inputSrcMesh->getActiveVertexTCoords(), outputDestTCoordsPtr);
    VecDataArray<float, 2>& outputTexCoords = *outputDestTCoordsPtr;

    // For every vertex of the destination mesh
    for (int i = 0; i < destVertices.size(); i++)
    {
        const Vec3d& pos = destVertices[i];
        double       minDistSqr   = IMSTK_DOUBLE_MAX;
        int          closestCellI = -1;
        Vec3d        closestPt    = Vec3d::Zero();

        // Find the closest point on the other mesh
        for (int j = 0; j < srcCells.size(); j++)
        {
            const Vec3i& cell = srcCells[j];
            const Vec3d& a    = srcVertices[cell[0]];
            const Vec3d& b    = srcVertices[cell[1]];
            const Vec3d& c    = srcVertices[cell[2]];

            int          caseType = -1;
            const Vec3d  ptOnTri  = closestPointOnTriangle(pos, a, b, c, caseType);
            const double sqrDist  = (ptOnTri - pos).squaredNorm();
            if (sqrDist < minDistSqr)
            {
                minDistSqr   = sqrDist;
                closestPt    = ptOnTri;
                closestCellI = j;
            }
        }

        // Compute interpolate value
        const Vec3i& closestCell = srcCells[closestCellI];
        const Vec2f& va  = inputTexCoords[closestCell[0]];
        const Vec2f& vb  = inputTexCoords[closestCell[1]];
        const Vec2f& vc  = inputTexCoords[closestCell[2]];
        const Vec3d& a   = srcVertices[closestCell[0]];
        const Vec3d& b   = srcVertices[closestCell[1]];
        const Vec3d& c   = srcVertices[closestCell[2]];
        const Vec3d  uvw = baryCentric(closestPt, a, b, c);
        const Vec2f  val = uvw[0] * va + uvw[1] * vb + uvw[2] * vc;
        outputTexCoords[i] = val;
    }
}
} // namespace imstk