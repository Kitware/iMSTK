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

#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkVecDataArray.h"

#include <gtest/gtest.h>

using namespace imstk;

///
/// \brief Test the mesh extraction of a cube
///
TEST(imstkTetrahedralMeshTest, ExtractSurfaceMesh)
{
    TetrahedralMesh tetMesh;

    // Setup a cube
    //    0-------1
    //   /|      /|
    //  / |     / |
    // 3--|----2  |
    // |  4----|--5    +y +z
    // | /     | /     | /
    // 7-------6       |/__+x
    {
        auto verticesPtr = std::make_shared<VecDataArray<double, 3>>(8);
        auto indicesPtr  = std::make_shared<VecDataArray<int, 4>>(5);

        VecDataArray<double, 3>& vertices = *verticesPtr;
        VecDataArray<int, 4>&    indices  = *indicesPtr;

        vertices[0] = Vec3d(-0.5, 0.5, 0.5);
        vertices[1] = Vec3d(0.5, 0.5, 0.5);
        vertices[2] = Vec3d(0.5, 0.5, -0.5);
        vertices[3] = Vec3d(-0.5, 0.5, -0.5);
        vertices[4] = Vec3d(-0.5, -0.5, 0.5);
        vertices[5] = Vec3d(0.5, -0.5, 0.5);
        vertices[6] = Vec3d(0.5, -0.5, -0.5);
        vertices[7] = Vec3d(-0.5, -0.5, -0.5);

        indices[0] = Vec4i(0, 7, 5, 4);
        indices[1] = Vec4i(3, 7, 2, 0);
        indices[2] = Vec4i(2, 7, 5, 0);
        indices[3] = Vec4i(1, 2, 0, 5);
        indices[4] = Vec4i(2, 6, 7, 5);

        tetMesh.initialize(verticesPtr, indicesPtr);
    }

    // Extract the surface
    std::shared_ptr<SurfaceMesh>             surfMesh = tetMesh.extractSurfaceMesh();
    std::shared_ptr<VecDataArray<double, 3>> surfVerticesPtr = surfMesh->getVertexPositions();
    std::shared_ptr<VecDataArray<int, 3>>    surfIndicesPtr  = surfMesh->getTriangleIndices();
    VecDataArray<double, 3>&                 surfVertices    = *surfVerticesPtr;
    VecDataArray<int, 3>&                    surfIndices     = *surfIndicesPtr;

    // There should be 12 faces and 8 verts of the cube
    ASSERT_EQ(12, surfIndices.size());
    ASSERT_EQ(8, surfVertices.size());

    // To test the winding direction of the faces we'll use the normals
    for (int i = 0; i < surfIndices.size(); i++)
    {
        const Vec3i& face     = surfIndices[i];
        const Vec3d  centroid = (surfVertices[face[0]] + surfVertices[face[1]] + surfVertices[face[2]]) / 3.0;
        const Vec3d  n = ((surfVertices[face[1]] - surfVertices[face[0]]).cross(surfVertices[face[2]] - surfVertices[face[0]])).normalized();

        // Project the centroid (of the centered 0,0,0 cube) onto the face normal
        // It should be positive when facing outwards (the correct direction)
        const double val = centroid.dot(n);
        ASSERT_GT(val, 0.0) << "face: " << i;
    }
}

///
/// \brief Test the computation of volume
///
TEST(imstkTetrahedralMeshTest, GetVolume)
{
    TetrahedralMesh tetMesh;

    // We use a regular tetrahedron with edge lengths 2
    // V = (edge length)^3/(6sqrt(2))
    const double edgeLenth      = 2.0;
    const double expectedVolume = std::pow(edgeLenth, 3.0) / (6.0 * std::sqrt(2.0));

    auto                     verticesPtr = std::make_shared<VecDataArray<double, 3>>(4);
    auto                     indicesPtr  = std::make_shared<VecDataArray<int, 4>>(1);
    VecDataArray<double, 3>& vertices    = *verticesPtr;
    VecDataArray<int, 4>&    indices     = *indicesPtr;

    vertices[0] = Vec3d(1.0, 0.0, -1.0 / std::sqrt(edgeLenth));
    vertices[1] = Vec3d(-1.0, 0.0, -1.0 / std::sqrt(edgeLenth));
    vertices[2] = Vec3d(0.0, 1.0, 1.0 / std::sqrt(edgeLenth));
    vertices[3] = Vec3d(0.0, -1.0, 1.0 / std::sqrt(edgeLenth));

    indices[0] = Vec4i(0, 1, 2, 3);

    tetMesh.initialize(verticesPtr, indicesPtr);
    EXPECT_NEAR(expectedVolume, tetMesh.getVolume(), 0.000001);
}
