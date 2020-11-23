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

#include "bandwidth.h"
#include "imstkGeometryUtilities.h"
#include "imstkLogger.h"
#include "imstkMeshIO.h"
#include "imstkNew.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"

using namespace imstk;

///
/// \brief create a quad mesh
/// \retval pair of connectivity and num of vertices
///
std::pair<std::shared_ptr<VecDataArray<int, 4>>, size_t> createConn();

template<int N>
void testRCM(const VecDataArray<int, N>& conn, const size_t numVerts);

template<int N>
void
toSTLVector(const VecDataArray<int, N>& inArr, std::vector<std::array<size_t, N>>& outArr)
{
    outArr.clear();
    outArr.reserve(inArr.size());
    for (int i = 0; i < inArr.size(); i++)
    {
        const VecDataArray<int, N>::VecType& vec = inArr[i];
        std::array<size_t, N>                conn;
        for (int j = 0; j < vec.size(); j++)
        {
            conn[j] = vec[j];
        }
        outArr.push_back(conn);
    }
}

int
main(int argc, char** argv)
{
    // Log to stdout and file
    Logger::startLogger();

    // a 2D Cartesian mesh
    {
        auto p = createConn();
        testRCM(*p.first, p.second);
    }

    // 3D mesh
    {
        auto         tetMesh  = MeshIO::read<TetrahedralMesh>(iMSTK_DATA_ROOT "/asianDragon/asianDragon.veg");
        const size_t numVerts = tetMesh->getNumVertices();
        std::cout << "Number of vertices = " << numVerts << std::endl;
        testRCM(*tetMesh->getTetrahedraIndices(), numVerts);
    }

    // a surface mesh cover
    {
        auto surfMesh = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/asianDragon/asianDragon.obj");
        auto tetMesh  = GeometryUtils::createTetrahedralMeshCover(surfMesh, 80, 40, 60);
        auto numVerts = tetMesh->getNumVertices();
        std::cout << "Number of vertices = " << numVerts << std::endl;
        testRCM(*tetMesh->getTetrahedraIndices(), numVerts);
    }

    return 0;
}

template<int N>
void
testRCM(const VecDataArray<int, N>& conn, const size_t numVerts)
{
    std::vector<std::array<size_t, N>> connSTL;
    toSTLVector(conn, connSTL);

    std::cout << "Old bandwidth = " << bandwidth(connSTL, numVerts) << std::endl;

    // new-to-old permutation
    auto perm = GeometryUtils::reorderConnectivity(connSTL, numVerts);

    // old-to-new permutation
    std::vector<size_t> invPerm(perm.size());
    for (size_t i = 0; i < perm.size(); ++i)
    {
        CHECK(perm[i] < numVerts) << "new vertex index should not be greater than number of vertices";
        invPerm[perm[i]] = i;
    }

    auto newConn = connSTL;

    for (auto& vertices : newConn)
    {
        for (auto& vid : vertices)
        {
            CHECK(vid < numVerts) << "Vertex id invalid since its greater than the number of vertices";
            vid = invPerm[vid];
        }
    }

    std::cout << "New bandwidth = " << bandwidth(newConn, numVerts) << "\n" << std::endl;

    return;
}

std::pair<std::shared_ptr<VecDataArray<int, 4>>, size_t>
createConn()
{
    /**
    6-------9-------7-------8
    |       |       |       |
    |   6   |   7   |   8   |
    |       |       |       |
    4------11-------5-------10
    |       |       |       |
    |   3   |   4   |   5   |
    |       |       |       |
    2------13-------3-------12
    |       |       |       |
    |   0   |   1   |   2   |
    |       |       |       |
    0------15-------1-------14
    **/

    imstkNew<VecDataArray<int, 4>> connPtr(9);
    VecDataArray<int, 4>&          conn = *connPtr.get();
    conn[0] = Vec4i(0, 15, 13, 2);
    conn[1] = Vec4i(15, 1, 3, 13);
    conn[2] = Vec4i(1, 14, 12, 3);
    conn[3] = Vec4i(2, 13, 11, 4);
    conn[4] = Vec4i(13, 3, 5, 11);
    conn[5] = Vec4i(3, 12, 10, 5);
    conn[6] = Vec4i(4, 11, 9, 6);
    conn[7] = Vec4i(11, 5, 7, 9);
    conn[8] = Vec4i(5, 10, 8, 7);

    return std::make_pair(connPtr.get(), 16);
}
