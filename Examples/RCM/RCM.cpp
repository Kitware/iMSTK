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

#include "imstkMeshIO.h"
#include "imstkTetrahedralMesh.h"
#include "imstkGeometry.h"

#include "bandwidth.h"

using namespace imstk;

using QuadConn = std::array<size_t, 4>;

///
/// \brief create a quad mesh
/// \retval pair of connectivity and num of vertices
///
std::pair<std::vector<QuadConn>, size_t> createConn();

template <typename ElemConn>
void testRCM(const std::vector<ElemConn>& conn, const size_t numVerts);

int main(int argc, char** argv) {
     
    // a 2D Cartesian mesh
    {
        auto p = createConn();
        testRCM(p.first, p.second);
    }

    // dragon mesh
    {
        auto tetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(MeshIO::read(iMSTK_DATA_ROOT "/asianDragon/asianDragon.veg"));
        auto conn = tetMesh->getTetrahedraVertices(); 
        auto numVerts = tetMesh->getNumVertices();
        testRCM(conn, numVerts);
    }

    // a mesh cover
    {
        const size_t nx = 80;
        const size_t ny = 40;
        const size_t nz = 60;
        auto surfMesh = std::dynamic_pointer_cast<SurfaceMesh>(MeshIO::read(iMSTK_DATA_ROOT "/asianDragon/asianDragon.obj"));
        auto tetMesh = TetrahedralMesh::createTetrahedralMeshCover(surfMesh, nx, ny, nz);
        auto conn = tetMesh->getTetrahedraVertices(); 
        auto numVerts = tetMesh->getNumVertices();
        testRCM(conn, numVerts);
    }

    return 0;
}

template <typename ElemConn>
void testRCM(const std::vector<ElemConn>& conn, const size_t numVerts) 
{
    std::cout << "bandwidth_old = " << bandwidth(conn, numVerts) << std::endl;

    // new-to-old permutation
    auto perm = GeometricUtils::RCM(conn, numVerts);

    // old-to-new permutation
    std::vector<size_t> invPerm(perm.size());
    for (size_t i=0; i<perm.size(); ++i)
    {
        CHECK(perm[i] < numVerts);
        invPerm[perm[i]] = i;
    }

    auto newConn = conn;

    for (auto& vertices : newConn)
    {
        for (auto& vid : vertices)
        {
            CHECK(vid < numVerts);
            vid = invPerm[vid];
        }
    }

    std::cout << "bandwidth_new = " << bandwidth(newConn, numVerts) << "\n" << std::endl;

    return;
}

std::pair<std::vector<QuadConn>, size_t> createConn()
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

    std::vector<QuadConn> conn(9);
    conn[0] = {0, 15, 13, 2};
    conn[1] = {15, 1, 3, 13};
    conn[2] = {1, 14, 12, 3};
    conn[3] = {2, 13, 11, 4};
    conn[4] = {13, 3, 5, 11};
    conn[5] = {3, 12, 10, 5};
    conn[6] = {4, 11, 9, 6};
    conn[7] = {11, 5, 7, 9};
    conn[8] = {5, 10, 8, 7};

    return std::make_pair(conn, 16);
}
