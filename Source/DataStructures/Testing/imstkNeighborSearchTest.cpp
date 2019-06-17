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

#define SPHERE_RADIUS   Real(1)
#define SPHERE_CENTER   Vec3r(0, 0, 0)
#define PARTICLE_RADIUS Real(0.05)
#define ITERATIONS      5
#define STEP            Real(1.1)

///
/// \brief Advance particle positions
///
void advancePositions(StdVectorOfVec3r& particles)
{
    for(auto& pos: particles)
    {
        Vec3r pc  = pos - SPHERE_CENTER;
        Real mag = pc.norm() * STEP;
        pos = SPHERE_CENTER + pc.normalized() * mag;
    }
}

///
/// \brief Search neighbors using brute-force approach
///
void neighborSearchBruteForce(StdVectorOfVec3r& particles, std::vector<std::vector<size_t>>& neighbors)
{
    neighbors.resize(particles.size());
    const Real radius    = Real(4.000000000000001) * PARTICLE_RADIUS;
    const Real radiusSqr = radius * radius;

    for(size_t p = 0; p < particles.size(); ++p)
    {
        const auto ppos       = particles[p];
        auto&      pneighbors = neighbors[p];
        pneighbors.resize(0);

        for(size_t q = 0; q < particles.size(); ++q)
        {
            if(p == q)
            {
                continue;
            }

            const auto qpos = particles[q];
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
void neighborSearchGridBased(StdVectorOfVec3r& particles, std::vector<std::vector<size_t>>& neighbors)
{
    neighbors.resize(particles.size());
    const Real radius = Real(4.000000000000001) * PARTICLE_RADIUS;
    static GridBasedNeighborSearch gridSearch;
    gridSearch.setSearchRadius(radius);
    gridSearch.getNeighbors(neighbors, particles);
}

///
/// \brief Search neighbors using spatial hashing approach
///
void neighborSearchSpatialHashing(StdVectorOfVec3r& particles, std::vector<std::vector<size_t>>& neighbors)
{
    neighbors.resize(particles.size());
    for(auto& list : neighbors)
    {
        list.resize(0);
    }

    const Real radius = Real(4.000000000000001) * PARTICLE_RADIUS;
    static SpatialHashTableSeparateChaining hashTable;

    hashTable.clear();
    hashTable.setCellSize(radius, radius, radius);
    hashTable.insertPoints(particles);

    for(size_t p = 0; p < particles.size(); ++p)
    {
        auto& v = particles[p];
        hashTable.getPointsInSphere(neighbors[p], v, radius);
    }
}

///
/// \brief For each particle in setA, search neighbors in setB using brute-force approach
///
void neighborSearchBruteForce(StdVectorOfVec3r& setA, StdVectorOfVec3r& setB, std::vector<std::vector<size_t>>& neighbors)
{
    neighbors.resize(setA.size());
    const Real radius    = Real(4.000000000000001) * PARTICLE_RADIUS;
    const Real radiusSqr = radius * radius;

    for(size_t p = 0; p < setA.size(); ++p)
    {
        const auto ppos       = setA[p];
        auto&      pneighbors = neighbors[p];
        pneighbors.resize(0);

        for(size_t q = 0; q < setB.size(); ++q)
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
void neighborSearchGridBased(StdVectorOfVec3r& setA, StdVectorOfVec3r& setB, std::vector<std::vector<size_t>>& neighbors)
{
    neighbors.resize(setA.size());
    const Real radius = Real(4.000000000000001) * PARTICLE_RADIUS;
    static GridBasedNeighborSearch gridSearch;
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

class dummyClass : public ::testing::Test {};

///
/// \brief Generate a sphere-shape particles and search neighbors for each particle
///
TEST_F(dummyClass, CompareGridSearchAndSpatialHashing)
{
    const Vec3r sphereCenter   = SPHERE_CENTER;
    const auto sphereRadiusSqr = SPHERE_RADIUS * SPHERE_RADIUS;
    const auto spacing         = Real(2) * PARTICLE_RADIUS;
    const int N                = int(2 * SPHERE_RADIUS / spacing);

    StdVectorOfVec3r particles;
    particles.reserve(N * N * N);
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
                    particles.push_back(ppos);
                }
            }
        }
    }

    std::vector<std::vector<size_t>> neighbors0;
    std::vector<std::vector<size_t>> neighbors1;
    std::vector<std::vector<size_t>> neighbors2;

    for(int iter = 0; iter < ITERATIONS; ++iter)
    {
        neighborSearchBruteForce(particles, neighbors0);
        neighborSearchGridBased(particles, neighbors1);
        neighborSearchSpatialHashing(particles, neighbors2);

        EXPECT_EQ(verify(neighbors1, neighbors0), true);
        EXPECT_EQ(verify(neighbors2, neighbors0), true);
        advancePositions(particles);
    }
}

///
/// \brief Generate a sphere-shape particles and divide them into two point sets, then for each point in setA search neighbors in setB
///
TEST_F(dummyClass, TestGridSearchFromDifferentPointSet)
{
    const Vec3r sphereCenter   = SPHERE_CENTER;
    const auto sphereRadiusSqr = SPHERE_RADIUS * SPHERE_RADIUS;
    const auto spacing         = Real(2) * PARTICLE_RADIUS;
    const int N                = int(2 * SPHERE_RADIUS / spacing);

    StdVectorOfVec3r particles;
    particles.reserve(N * N * N);
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
                    particles.push_back(ppos);
                }
            }
        }
    }

    StdVectorOfVec3r setA;
    StdVectorOfVec3r setB;
    std::vector<std::vector<size_t>> neighbors0;
    std::vector<std::vector<size_t>> neighbors1;

    for(int iter = 0; iter < ITERATIONS; ++iter)
    {
        // separate verts into setA and setB randomly
        setA.resize(0);
        setB.resize(0);
        setA.reserve(particles.size());
        setB.reserve(particles.size());

        for(size_t i = 0; i < particles.size(); ++i)
        {
            if(rand() % 2)
            {
                setA.push_back(particles[i]);
            }
            else
            {
                setB.push_back(particles[i]);
            }
        }

        // search for neighbors and compare
        neighborSearchBruteForce(setA, setB, neighbors0);
        neighborSearchGridBased(setA, setB, neighbors1);
        EXPECT_EQ(verify(neighbors1, neighbors0), true);
    }
}

int imstkNeighborSearchTest(int argc, char* argv[])
{
    // Init Google Test & Mock
    ::testing::InitGoogleTest(&argc, argv);
    // Run tests with gtest
    return RUN_ALL_TESTS();
}
