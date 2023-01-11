/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include <vector>
#include <unordered_set>
#include <algorithm>
#include <gtest/gtest.h>

#include "imstkGeometry.h"
#include "imstkGeometryUtilities.h"
#include "imstkLogger.h"
#include "imstkNew.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkMeshIO.h"

using namespace imstk;

///
/// \brief Build the vertex-to-vertex connectivity of a map
//
/// \param conn element-to-vertex connectivity of the map
/// \param numVerts number of vertice in the map
/// \retval vertToVert vertex-to-vertex connectivity
///
template<typename ElemConn>
static void
buildVertToVert(const std::vector<ElemConn>&             conn,
                const size_t                             numVerts,
                std::vector<std::unordered_set<size_t>>& vertToVert)
{
    // constexpr size_t numVertPerElem = ElemConn::size();
    std::vector<size_t> vertToElemPtr(numVerts + 1, 0);
    std::vector<size_t> vertToElem;

    // find the number of adjacent elements for each vertex
    for (const auto& vertices : conn)
    {
        for (auto vid : vertices)
        {
            vertToElemPtr[vid + 1] += 1;
        }
    }

    // accumulate pointer
    for (size_t i = 0; i < numVerts; ++i)
    {
        vertToElemPtr[i + 1] += vertToElemPtr[i];
    }

    // track the number
    auto   pos    = vertToElemPtr;
    size_t totNum = vertToElemPtr.back();

    vertToElem.resize(totNum);

    for (size_t eid = 0; eid < conn.size(); ++eid)
    {
        for (auto vid : conn[eid])
        {
            vertToElem[pos[vid]] = eid;
            ++pos[vid];
        }
    }

    // connectivity of vertex-to-vertex
    vertToVert.resize(numVerts);
    auto getVertexNbrs =
        [&vertToElem, &vertToElemPtr, &conn, &vertToVert](const size_t i) {
            const auto ptr0 = vertToElemPtr[i];
            const auto ptr1 = vertToElemPtr[i + 1];
            size_t     eid;

            for (auto ptr = ptr0; ptr < ptr1; ++ptr)
            {
                eid = vertToElem[ptr];
                for (auto vid : conn[eid])
                {
                    // vertex-i itself is also included.
                    vertToVert[i].insert(vid);
                }
            }
        };

    for (size_t i = 0; i < numVerts; ++i)
    {
        getVertexNbrs(i);
    }
}

///
/// \brief Returns the bandwidth of a map
///
/// \param neighbors array of neighbors of each vertex; eg, neighbors[i] is a object containing
///
template<typename NBR>
size_t
bandwidth(const std::vector<NBR>& neighbors)
{
    size_t d    = 0;
    size_t dCur = 0;
    for (size_t i = 0; i < neighbors.size(); ++i)
    {
        for (const auto& j : neighbors[i])
        {
            dCur = (i > j) ? (i - j) : (j - i);
            d    = std::max(d, dCur);
        }
    }
    return d;
}

///
/// \brief Returns the bandwidth of a map
///
/// \param conn element-to-vertex connectivity of the map
/// \param numVerts number of vertices in the map
///
template<typename ElemConn>
size_t
bandwidth(const std::vector<ElemConn>& conn, const size_t numVerts)
{
    std::vector<std::unordered_set<size_t>> vertToVert;
    buildVertToVert(conn, numVerts, vertToVert);
    return bandwidth(vertToVert);
}

///
/// \brief create a quad mesh
/// \retval pair of connectivity and num of vertices
///
std::pair<std::shared_ptr<VecDataArray<int, 4>>, size_t> createConn();

template<int N>
void
toSTLVector(const VecDataArray<int, N>& inArr, std::vector<std::array<size_t, static_cast<size_t>(N)>>& outArr)
{
    using ValueType = typename VecDataArray<int, N>::ValueType;

    outArr.clear();
    outArr.reserve(inArr.size());
    for (int i = 0; i < inArr.size(); i++)
    {
        const ValueType&                           vec = inArr[i];
        std::array<size_t, static_cast<size_t>(N)> conn;
        for (int j = 0; j < vec.size(); j++)
        {
            conn[j] = vec[j];
        }
        outArr.push_back(conn);
    }
}

template<int N>
void
testRCM(const VecDataArray<int, N>& conn, const size_t numVerts)
{
    std::vector<std::array<size_t, N>> connSTL;
    toSTLVector(conn, connSTL);

    const size_t oldBandwidth = bandwidth(connSTL, numVerts);

    // new-to-old permutation
    auto perm = GeometryUtils::reorderConnectivity(connSTL, numVerts);

    // old-to-new permutation
    std::vector<size_t> invPerm(perm.size());
    for (size_t i = 0; i < perm.size(); ++i)
    {
        EXPECT_TRUE(perm[i] < numVerts) << "new vertex index should not be greater than number of vertices";
        invPerm[perm[i]] = i;
    }

    auto newConn = connSTL;

    for (auto& vertices : newConn)
    {
        for (auto& vid : vertices)
        {
            EXPECT_TRUE(vid < numVerts) << "Vertex id invalid since its greater than the number of vertices";
            vid = invPerm[vid];
        }
    }

    const size_t newBandwidth = bandwidth(newConn, numVerts);
    EXPECT_LT(newBandwidth, oldBandwidth);
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

TEST(imstkRCMTest, TestRCM)
{
    // a 2D Cartesian mesh
    {
        auto p = createConn();
        testRCM<(int)4>(*p.first, p.second);
    }

    // 3D mesh
    {
        auto         tetMesh  = MeshIO::read<TetrahedralMesh>(iMSTK_DATA_ROOT "/asianDragon/asianDragon.vtu");
        const size_t numVerts = tetMesh->getNumVertices();
        //std::cout << "Number of vertices = " << numVerts << std::endl;
        testRCM(*tetMesh->getCells(), numVerts);
    }

    // a surface mesh cover
    {
        auto surfMesh = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/asianDragon/asianDragon.obj");
        auto tetMesh  = GeometryUtils::createTetrahedralMeshCover(surfMesh, 80, 40, 60);
        auto numVerts = tetMesh->getNumVertices();
        //std::cout << "Number of vertices = " << numVerts << std::endl;
        testRCM(*tetMesh->getCells(), numVerts);
    }
}