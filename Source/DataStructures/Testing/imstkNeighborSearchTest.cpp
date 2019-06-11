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

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "imstkMath.h"
#include "imstkSpatialHashTableSeparateChaining.h"
#include "imstkGridBasedNeighborSearch.h"

using namespace imstk;

using Real = double;
using Vec3r  = Vec3d;

#define SPHERE_RADIUS   Real(1)
#define SPHERE_CENTER   Vec3r(0, 0, 0)
#define PARTICLE_RADIUS Real(0.05)
#define ITERATIONS      100
#define STEP            Real(1.02)

///
/// \brief Advance particle positions
///
template<class Data>
void advancePositions(Data& verts)
{
    for(auto& v: verts)
    {
        Vec3r vc  = v - SPHERE_CENTER;
        Real mag = vc.norm() * STEP;
        v = SPHERE_CENTER + vc.normalized() * mag;
    }
}

///
/// \brief Search neighbors using brute-force approach
///
template<class Data>
void neighborSearch_BruteForce(Data& verts, std::vector<std::vector<size_t>>& neighbors)
{
    neighbors.resize(verts.size());
    const Real radius    = Real(4.000000000000001) * PARTICLE_RADIUS;
    const Real radiusSqr = radius * radius;
    for(size_t p = 0; p < verts.size(); ++p)
    {
        const auto ppos       = verts[p];
        auto&      pneighbors = neighbors[p];
        pneighbors.resize(0);

        for(auto q = 0; q < verts.size(); ++q)
        {
            if(p == q)
            {
                continue;
            }

            const auto qpos = verts[q];
            const auto d2   = (ppos - qpos).squaredNorm();
            if(d2 < radiusSqr)
            {
                pneighbors.push_back(q);
            }
        }
    }
}

///
/// \brief Search neighbors using grid-based approach
///
template<class Data>
void neighborSearch_GridBased(Data& verts, std::vector<std::vector<size_t>>& neighbors)
{
    neighbors.resize(verts.size());
    const Real radius = Real(4.000000000000001) * PARTICLE_RADIUS;
    static GridBasedNeighborSearch<Real> gridSearch;
    gridSearch.setSearchRadius(radius);
    gridSearch.getNeighbors(neighbors, verts);
}

///
/// \brief Search neighbors using spatial hashing approach
///
template<class Data>
void neighborSearch_SpatialHashing(Data& verts, std::vector<std::vector<size_t>>& neighbors)
{
    neighbors.resize(verts.size());
    for(auto& list : neighbors)
    {
        list.resize(0);
    }

    const Real radius = Real(4.000000000000001) * PARTICLE_RADIUS;
    static SpatialHashTableSeparateChaining hashTable;

    hashTable.clear();
    hashTable.setCellSize(radius, radius, radius);
    hashTable.insertPoints(verts);

    for(size_t p = 0; p < verts.size(); ++p)
    {
        auto& v = verts[p];
        hashTable.getPointsInSphere(neighbors[p], v, radius);
    }
}

///
/// \brief For each particle in setA, search neighbors in setB using brute-force approach
///
template<class Data>
void neighborSearch_BruteForce(Data& setA, Data& setB, std::vector<std::vector<size_t>>& neighbors)
{
    neighbors.resize(setA.size());
    const Real radius    = Real(4.000000000000001) * PARTICLE_RADIUS;
    const Real radiusSqr = radius * radius;
    for(size_t p = 0; p < setA.size(); ++p)
    {
        const auto ppos       = setA[p];
        auto&      pneighbors = neighbors[p];
        pneighbors.resize(0);

        for(auto q = 0; q < setB.size(); ++q)
        {
            const auto qpos = setB[q];
            const auto d2   = (ppos - qpos).squaredNorm();
            if(d2 < radiusSqr)
            {
                pneighbors.push_back(q);
            }
        }
    }
}

///
/// \brief For each particle in setA, search neighbors in setB using grid-based approach
///
template<class Data>
void neighborSearch_GridBased(Data& setA, Data& setB, std::vector<std::vector<size_t>>& neighbors)
{
    neighbors.resize(setA.size());
    const Real radius = Real(4.000000000000001) * PARTICLE_RADIUS;
    static GridBasedNeighborSearch<Real> gridSearch;
    gridSearch.setSearchRadius(radius);
    gridSearch.getNeighbors(neighbors, setA, setB);
}

///
/// \brief Verify if two neighbor search results are identical
///
bool verify(std::vector<std::vector<size_t>>& neighbors1,  std::vector<std::vector<size_t>>& neighbors2)
{
    if(neighbors1.size() != neighbors2.size())
    {
        return false;
    }
    for(size_t p = 0; p < neighbors1.size(); ++p)
    {
        auto& list1 = neighbors1[p];
        auto& list2 = neighbors2[p];
        list1.erase(std::remove(list1.begin(), list1.end(), p), list1.end());
        list2.erase(std::remove(list2.begin(), list2.end(), p), list2.end());

        std::sort(list1.begin(), list1.end());
        std::sort(list2.begin(), list2.end());

        if(list1.size() != list2.size())
        {
            return false;
        }

        for(size_t idx = 0; idx < list1.size(); ++idx)
        {
            if(list1[idx] != list2[idx])
            {
                return false;
            }
        }
    }
    return true;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
class dummyClass : public ::testing::Test {};

///
/// \brief Generate a sphere-shape particles and search neighbors for each particle
///
TEST_F(dummyClass, CompareGridSearchAndSpatialHashing)
{
    StdVectorOfVec3d verts;

    const Vec3r sphereCenter    = SPHERE_CENTER;
    const auto sphereRadiusSqr = SPHERE_RADIUS * SPHERE_RADIUS;
    const auto spacing         = Real(2) * PARTICLE_RADIUS;
    const int N = int(2 * SPHERE_RADIUS / spacing);

    verts.reserve(N * N * N);
    const Vec3r corner = sphereCenter - Vec3r(SPHERE_RADIUS, SPHERE_RADIUS, SPHERE_RADIUS);

    for(int i = 0; i < N; ++i)
    {
        for(int j = 0; j < N; ++j)
        {
            for(int k = 0; k < N; ++k)
            {
                const Vec3r ppos = corner + Vec3r(spacing * Real(i), spacing * Real(j), spacing * Real(k));
                const Vec3r d    = ppos - sphereCenter;
                if(d.squaredNorm() < sphereRadiusSqr)
                {
                    verts.push_back(ppos);
                }
            }
        }
    }
    //////////////////////////////////////////////////////////////////////////////////////////
    std::vector<std::vector<size_t>> neighbors0;
    std::vector<std::vector<size_t>> neighbors1;
    std::vector<std::vector<size_t>> neighbors2;
    for(int iter = 0; iter < ITERATIONS; ++iter)
    {
        neighborSearch_BruteForce(verts, neighbors0);
        neighborSearch_GridBased(verts, neighbors1);
        neighborSearch_SpatialHashing(verts, neighbors2);
        //////////////////////////////////////////////////////////////////////////////////////////
        EXPECT_EQ(verify(neighbors1, neighbors0), true);
        EXPECT_EQ(verify(neighbors2, neighbors0), true);
        advancePositions(verts);
    }
}

///
/// \brief Generate a sphere-shape particles and divide them into two point sets, then for each point in setA search neighbors in setB
///
TEST_F(dummyClass, TestGridSearchFromDifferentPointSet)
{
    StdVectorOfVec3d verts;

    const Vec3r sphereCenter    = SPHERE_CENTER;
    const auto sphereRadiusSqr = SPHERE_RADIUS * SPHERE_RADIUS;
    const auto spacing         = Real(2) * PARTICLE_RADIUS;
    const int N = int(2 * SPHERE_RADIUS / spacing);

    verts.reserve(N * N * N);
    const Vec3r corner = sphereCenter - Vec3r(SPHERE_RADIUS, SPHERE_RADIUS, SPHERE_RADIUS);

    for(int i = 0; i < N; ++i)
    {
        for(int j = 0; j < N; ++j)
        {
            for(int k = 0; k < N; ++k)
            {
                const Vec3r ppos = corner + Vec3r(spacing * Real(i), spacing * Real(j), spacing * Real(k));
                const Vec3r d    = ppos - sphereCenter;
                if(d.squaredNorm() < sphereRadiusSqr)
                {
                    verts.push_back(ppos);
                }
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    StdVectorOfVec3d setA;
    StdVectorOfVec3d setB;
    std::vector<std::vector<size_t>> neighbors0;
    std::vector<std::vector<size_t>> neighbors1;

    for(int iter = 0; iter < ITERATIONS; ++iter)
    {
        // separate verts into setA and setB randomly
        setA.resize(0);
        setB.resize(0);
        setA.reserve(verts.size());
        setB.reserve(verts.size());

        for(size_t i = 0; i < verts.size(); ++i)
        {
            if(rand() % 2)
            {
                setA.push_back(verts[i]);
            }
            else
            {
                setB.push_back(verts[i]);
            }
        }
        // search for neighbors and compare
        neighborSearch_BruteForce(setA, setB, neighbors0);
        neighborSearch_GridBased(setA, setB, neighbors1);
        EXPECT_EQ(verify(neighbors1, neighbors0), true);
    }
    //////////////////////////////////////////////////////////////////////////////////////////
}


int imstkNeighborSearchTest(int argc, char* argv[])
{
    // Init Google Test & Mock
    ::testing::InitGoogleTest(&argc, argv);
    // Run tests with gtest
    return RUN_ALL_TESTS();
}
