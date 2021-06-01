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

#include <gtest/gtest.h>

#include "imstkCollisionData.h"
#include "imstkNarrowPhaseCD.h"
#include "imstkOctreeBasedCD.h"
#include "imstkPointSetToSurfaceMeshCD.h"
#include "imstkSphere.h"
#include "imstkSurfaceMesh.h"
#include "imstkSurfaceMeshToSurfaceMeshCD.h"

#include <unordered_map>

using namespace imstk;

//#define PRINT_DEBUG

#define BOUND           10.0
#define SPHERE_RADIUS   Real(2)
#define SPHERE_CENTER   Vec3r(1e-10, 1e-10, 1e-10)
#define PARTICLE_RADIUS Real(0.2)
#define ITERATIONS      10

Real
rand01()
{
    return static_cast<double>(rand()) / static_cast<double>(RAND_MAX);
}

Real
rand11()
{
    return (static_cast<double>(rand()) / static_cast<double>(RAND_MAX) * 2.0 - 1.0);
}

///
/// \brief Generate a PointSet
///
std::shared_ptr<PointSet>
generatePointSet(const Real sphereRadius)
{
    const Vec3r sphereCenter    = SPHERE_CENTER;
    const auto  sphereRadiusSqr = sphereRadius * sphereRadius;
    const auto  spacing = Real(2) * PARTICLE_RADIUS;
    const int   N       = static_cast<int>(2.0 * sphereRadius / spacing);

    std::shared_ptr<VecDataArray<double, 3>> particles = std::make_shared<VecDataArray<double, 3>>();
    particles->reserve(N * N * N);
    const Vec3r corner = sphereCenter - Vec3r(1, 1, 1) * sphereRadius;

    for (uint64_t i = 0; i < N; ++i)
    {
        for (uint64_t j = 0; j < N; ++j)
        {
            for (uint64_t k = 0; k < N; ++k)
            {
                const Vec3r ppos = corner + Vec3r(spacing * Real(i), spacing * Real(j), spacing * Real(k));
                const Vec3r d    = ppos - sphereCenter;
                if (d.squaredNorm() < sphereRadiusSqr)
                {
                    particles->push_back(ppos);
                }
            }
        }
    }

    auto pointset = std::make_shared<PointSet>();
    pointset->initialize(particles);
    return pointset;
}

///
/// \brief Generate a Box mesh
///
std::shared_ptr<SurfaceMesh>
generateBoxMesh()
{
    std::vector<double> buffVertices
    {
        0.5, -0.5, 0.5,
        -0.5, -0.5, 0.5,
        0.5, 0.5, 0.5,
        -0.5, 0.5, 0.5,
        -0.5, -0.5, -0.5,
        0.5, -0.5, -0.5,
        -0.5, 0.5, -0.5,
        0.5, 0.5, -0.5
    };

    std::vector<int> buffFaces
    {
        1, 4, 2,
        1, 3, 4,
        5, 8, 6,
        5, 7, 8,
        7, 3, 8,
        7, 4, 3,
        6, 2, 5,
        6, 1, 2,
        6, 3, 1,
        6, 8, 3,
        2, 7, 5,
        2, 4, 7
    };

    std::shared_ptr<VecDataArray<double, 3>> vertices = std::make_shared<VecDataArray<double, 3>>();
    vertices->reserve(static_cast<int>(buffVertices.size() / 3));
    for (size_t i = 0; i < buffVertices.size() / 3; ++i)
    {
        vertices->push_back(Vec3d(buffVertices[i * 3],
                                     buffVertices[i * 3 + 1],
                                     buffVertices[i * 3 + 2]));
    }

    std::shared_ptr<VecDataArray<int, 3>> facesPtr = std::make_shared<VecDataArray<int, 3>>();
    VecDataArray<int, 3>&                 faces    = *facesPtr;
    faces.reserve(static_cast<int>(buffFaces.size()) / 3);
    for (int i = 0; i < static_cast<int>(buffFaces.size()) / 3; ++i)
    {
        // Face ID of triangles is 0-based index (data from .obj file is 1-based index)
        faces.push_back(Vec3i(buffFaces[i * 3] - 1, buffFaces[i * 3 + 1] - 1, buffFaces[i * 3 + 2] - 1));
    }

    const auto mesh = std::make_shared<SurfaceMesh>();
    mesh->initialize(vertices, facesPtr);
    return mesh;
}

///
/// \brief Generate a triangle soup (random discrete triangles)
///
std::shared_ptr<SurfaceMesh>
generateMesh()
{
    std::shared_ptr<VecDataArray<double, 3>> vertices = std::make_shared<VecDataArray<double, 3>>();
    std::shared_ptr<VecDataArray<int, 3>>    facesPtr = std::make_shared<VecDataArray<int, 3>>();
    VecDataArray<int, 3>&                    faces    = *facesPtr;

    auto randD = [] { return (static_cast<double>(rand()) / static_cast<double>(RAND_MAX) * 2.0 - 1.0) * BOUND; };

    vertices->reserve(300);
    faces.reserve(100);
    for (int i = 0; i < 100; ++i)
    {
        faces.push_back(Vec3i(i * 3, i * 3 + 1, i * 3 + 2));
        vertices->push_back(Vec3d(randD(), randD(), randD()));
        vertices->push_back(Vec3d(randD(), randD(), randD()));
        vertices->push_back(Vec3d(randD(), randD(), randD()) * rand11() * 0.1);
    }
    auto mesh = std::make_shared<SurfaceMesh>();
    mesh->initialize(vertices, facesPtr);
    return mesh;
}

namespace imstk
{
class OctreeBasedCDTest : public ::testing::Test
{
public:
    OctreeBasedCDTest() {}

    void reset()
    {
        m_OctreeCD.reset(new OctreeBasedCD(Vec3d(0, 0, 0), 100.0, 0.1, 2));
    }

    void testPointMeshManual()
    {
        reset();
        const Real sphereRadius = rand01() * SPHERE_RADIUS + 0.5;
        const auto pointset     = generatePointSet(sphereRadius);
        const auto mesh = generateBoxMesh();
        m_OctreeCD->addPointSet(pointset);
        m_OctreeCD->addTriangleMesh(mesh);
        m_OctreeCD->build();

        // Manually check for penetration
        std::vector<int>    pointPenetrations(pointset->getNumVertices());
        std::vector<double> pointPenetrationDistances(pointset->getNumVertices());
        size_t              numPenetrations = 0;
        for (uint32_t p = 0; p < pointset->getNumVertices(); ++p)
        {
            const auto& point = pointset->getVertexPosition(p);
            bool        bPenetration = true;
            double      penetrationDistance = 1e10;
            for (uint32_t i = 0; i < 3; ++i)
            {
                if (point[i] < -0.5 || point[i] > 0.5)
                {
                    bPenetration = false;
                    break;
                }
                penetrationDistance = std::min(penetrationDistance, std::abs(point[i] - 0.5));
                penetrationDistance = std::min(penetrationDistance, std::abs(point[i] + 0.5));
            }
            pointPenetrations[p] = bPenetration;
            pointPenetrationDistances[p] = penetrationDistance;
            if (bPenetration)
            {
                ++numPenetrations;
            }
        }

        // Detect penetration using octree
        m_OctreeCD->addCollisionPair(pointset, mesh, CollisionDetection::Type::PointSetToSurfaceMesh, std::make_shared<CollisionData>());
        m_OctreeCD->detectCollision();
        const auto collisionData = m_OctreeCD->getCollisionPairData(pointset->getGlobalIndex(), mesh->getGlobalIndex());
#ifdef PRINT_DEBUG
        std::cout << "VTColData.getSize() = " << collisionData->VTColData.getSize() << std::endl;
#endif

        // Compare result
        EXPECT_EQ(collisionData->VTColData.getSize(), numPenetrations);
        std::unordered_set<uint32_t> penetratedPoints;
        for (size_t i = 0; i < collisionData->VTColData.getSize(); ++i)
        {
            penetratedPoints.insert(collisionData->VTColData[i].vertexIdx);
            EXPECT_EQ(pointPenetrations[collisionData->VTColData[i].vertexIdx], true);
            EXPECT_EQ(std::abs(pointPenetrationDistances[collisionData->VTColData[i].vertexIdx] -
                      collisionData->VTColData[i].closestDistance) < 1e-10, true);
        }
        EXPECT_EQ(penetratedPoints.size(), numPenetrations);
    }

    void testPointMeshUsingPointSetToSurfaceMeshCD()
    {
        const Real sphereRadius = rand01() * SPHERE_RADIUS + 0.5;
        const auto pointset     = generatePointSet(sphereRadius);
        const auto mesh = generateBoxMesh();

        // Manually check for penetration
        std::vector<int>    pointPenetrations(pointset->getNumVertices());
        std::vector<double> pointPenetrationDistances(pointset->getNumVertices());
        size_t              numPenetrations = 0;
        for (uint32_t p = 0; p < pointset->getNumVertices(); ++p)
        {
            const auto& point = pointset->getVertexPosition(p);
            bool        bPenetration = true;
            double      penetrationDistance = 1e10;
            for (uint32_t i = 0; i < 3; ++i)
            {
                if (point[i] < -0.5 || point[i] > 0.5)
                {
                    bPenetration = false;
                    break;
                }
                penetrationDistance = std::min(penetrationDistance, std::abs(point[i] - 0.5));
                penetrationDistance = std::min(penetrationDistance, std::abs(point[i] + 0.5));
            }
            pointPenetrations[p] = bPenetration;
            pointPenetrationDistances[p] = penetrationDistance;
            if (bPenetration)
            {
                ++numPenetrations;
            }
        }

        // Detect penetration using PointSetToSurfaceMeshCD
        // Firstly we must reset the octree
        CollisionDetection::clearInternalOctree();
        const auto collisionData = std::make_shared<CollisionData>();
        const auto CD = std::make_shared<PointSetToSurfaceMeshCD>(pointset, mesh, collisionData);
        CollisionDetection::updateInternalOctreeAndDetectCollision();
#ifdef PRINT_DEBUG
        std::cout << "VTColData.getSize() = " << collisionData->VTColData.getSize() << std::endl;
#endif

        // Compare result
        EXPECT_EQ(collisionData->VTColData.getSize(), numPenetrations);
        std::unordered_set<uint32_t> penetratedPoints;
        for (size_t i = 0; i < collisionData->VTColData.getSize(); ++i)
        {
            penetratedPoints.insert(collisionData->VTColData[i].vertexIdx);
            EXPECT_EQ(pointPenetrations[collisionData->VTColData[i].vertexIdx], true);
            EXPECT_EQ(std::abs(pointPenetrationDistances[collisionData->VTColData[i].vertexIdx] -
                      collisionData->VTColData[i].closestDistance) < 1e-10, true);
        }
        EXPECT_EQ(penetratedPoints.size(), numPenetrations);
    }

    void testPointSphere()
    {
        reset();
        const Real sphereRadius = rand01() * SPHERE_RADIUS + 0.5;
        const auto pointset     = generatePointSet(sphereRadius);
        const auto sphere       = std::make_shared<Sphere>();
        sphere->setRadius(5.0);
        m_OctreeCD->addPointSet(pointset);
        m_OctreeCD->addAnalyticalGeometry(sphere);
        m_OctreeCD->build();

        // Manually check for penetration
        std::vector<int> pointPenetration(pointset->getNumVertices());
        size_t           numPenetrations = 0;
        for (uint32_t p = 0; p < pointset->getNumVertices(); ++p)
        {
            const auto& point = pointset->getVertexPosition(p);
            if ((point - SPHERE_CENTER).norm() < sphereRadius)
            {
                pointPenetration[p] = true;
                ++numPenetrations;
            }
            else
            {
                pointPenetration[p] = false;
            }
        }

        // Detect penetration using octree
        m_OctreeCD->addCollisionPair(pointset, sphere, CollisionDetection::Type::PointSetToSphere, std::make_shared<CollisionData>());
        m_OctreeCD->detectCollision();
        const auto collisionData = m_OctreeCD->getCollisionPairData(pointset->getGlobalIndex(), sphere->getGlobalIndex());
#ifdef PRINT_DEBUG
        std::cout << "MAColData.getSize() = " << collisionData->MAColData.getSize() << std::endl;
#endif

        // Compare result
        EXPECT_EQ(collisionData->PColData.getSize(), numPenetrations);
        for (size_t i = 0; i < collisionData->PColData.getSize(); ++i)
        {
            EXPECT_EQ(pointPenetration[collisionData->PColData[i].nodeIdx], true);
        }
    }

    void testMeshMeshManual()
    {
        reset();
        const auto mesh = generateMesh();
        const auto box  = generateBoxMesh();
        m_OctreeCD->addTriangleMesh(mesh);
        m_OctreeCD->addTriangleMesh(box);
        m_OctreeCD->build();

        // Brute-force check for collision
        auto collisionData = std::make_shared<CollisionData>();
        for (uint32_t i = 0; i < mesh->getNumTriangles(); ++i)
        {
            for (uint32_t j = 0; j < box->getNumTriangles(); ++j)
            {
                NarrowPhaseCD::triangleToTriangle(i, mesh.get(), j, box.get(), collisionData);
            }
        }

        auto computePairHash = [](uint32_t idx1, uint32_t idx2)
                               {
                                   const uint64_t first  = static_cast<uint64_t>(idx1);
                                   const uint64_t second = static_cast<uint64_t>(idx2);
                                   const auto     pair   = (first << 32) | second;
                                   return pair;
                               };

        std::unordered_set<uint64_t>                               VTCollisions;
        std::unordered_map<uint64_t, std::unordered_set<uint64_t>> EECollisions;
        uint64_t                                                   numEECollisions = 0;
        for (size_t i = 0; i < collisionData->VTColData.getSize(); ++i)
        {
            VTCollisions.insert(computePairHash(collisionData->VTColData[i].vertexIdx, collisionData->VTColData[i].triIdx));
        }
        for (size_t i = 0; i < collisionData->EEColData.getSize(); ++i)
        {
            const auto edgeA = computePairHash(collisionData->EEColData[i].edgeIdA.first, collisionData->EEColData[i].edgeIdA.second);
            const auto edgeB = computePairHash(collisionData->EEColData[i].edgeIdB.first, collisionData->EEColData[i].edgeIdB.second);
            EECollisions[edgeA].insert(edgeB);
            ++numEECollisions;
        }

        // Detect collision using octree
        m_OctreeCD->addCollisionPair(mesh, box, CollisionDetection::Type::SurfaceMeshToSurfaceMesh, std::make_shared<CollisionData>());
        m_OctreeCD->detectCollision();
        collisionData = m_OctreeCD->getCollisionPairData(mesh->getGlobalIndex(), box->getGlobalIndex());
#ifdef PRINT_DEBUG
        std::cout << "VTColData.getSize() = " << collisionData->VTColData.getSize() << std::endl;
        std::cout << "EEColData.getSize() = " << collisionData->EEColData.getSize() << std::endl;
#endif

        // Compare result
        EXPECT_EQ(collisionData->VTColData.getSize(), VTCollisions.size());
        EXPECT_EQ(collisionData->EEColData.getSize(), numEECollisions);

        for (size_t i = 0; i < collisionData->VTColData.getSize(); ++i)
        {
            const auto vt = computePairHash(collisionData->VTColData[i].vertexIdx, collisionData->VTColData[i].triIdx);
            EXPECT_EQ(VTCollisions.find(vt) != VTCollisions.end(), true);
        }
        for (size_t i = 0; i < collisionData->EEColData.getSize(); ++i)
        {
            const auto edgeA = computePairHash(collisionData->EEColData[i].edgeIdA.first, collisionData->EEColData[i].edgeIdA.second);
            const auto edgeB = computePairHash(collisionData->EEColData[i].edgeIdB.first, collisionData->EEColData[i].edgeIdB.second);
            EXPECT_EQ(EECollisions.find(edgeA) != EECollisions.end(), true);
            EXPECT_EQ(EECollisions[edgeA].find(edgeB) != EECollisions[edgeA].end(), true);
        }
    }

    void testMeshMeshUsingSurfaceMeshToSurfaceMeshCD()
    {
        const auto mesh = generateMesh();
        const auto box  = generateBoxMesh();

        // Brute-force check for collision
        auto collisionData = std::make_shared<CollisionData>();
        for (uint32_t i = 0; i < mesh->getNumTriangles(); ++i)
        {
            for (uint32_t j = 0; j < box->getNumTriangles(); ++j)
            {
                NarrowPhaseCD::triangleToTriangle(i, mesh.get(), j, box.get(), collisionData);
            }
        }

        auto computePairHash = [](uint32_t idx1, uint32_t idx2)
                               {
                                   const uint64_t first  = static_cast<uint64_t>(idx1);
                                   const uint64_t second = static_cast<uint64_t>(idx2);
                                   const auto     pair   = (first << 32) | second;
                                   return pair;
                               };

        std::unordered_set<uint64_t>                               VTCollisions;
        std::unordered_map<uint64_t, std::unordered_set<uint64_t>> EECollisions;
        uint64_t                                                   numEECollisions = 0;
        for (size_t i = 0; i < collisionData->VTColData.getSize(); ++i)
        {
            VTCollisions.insert(computePairHash(collisionData->VTColData[i].vertexIdx, collisionData->VTColData[i].triIdx));
        }
        for (size_t i = 0; i < collisionData->EEColData.getSize(); ++i)
        {
            const auto edgeA = computePairHash(collisionData->EEColData[i].edgeIdA.first, collisionData->EEColData[i].edgeIdA.second);
            const auto edgeB = computePairHash(collisionData->EEColData[i].edgeIdB.first, collisionData->EEColData[i].edgeIdB.second);
            EECollisions[edgeA].insert(edgeB);
            ++numEECollisions;
        }

        // Detect penetration using SurfaceMeshToSurfaceMeshCD
        // Firstly we must reset the octree
        CollisionDetection::clearInternalOctree();
        const auto CD = std::make_shared<SurfaceMeshToSurfaceMeshCD>(mesh, box, collisionData);
        CollisionDetection::updateInternalOctreeAndDetectCollision();
#ifdef PRINT_DEBUG
        std::cout << "VTColData.getSize() = " << collisionData->VTColData.getSize() << std::endl;
        std::cout << "EEColData.getSize() = " << collisionData->EEColData.getSize() << std::endl;
#endif

        // Compare result
        EXPECT_EQ(collisionData->VTColData.getSize(), VTCollisions.size());
        EXPECT_EQ(collisionData->EEColData.getSize(), numEECollisions);

        for (size_t i = 0; i < collisionData->VTColData.getSize(); ++i)
        {
            const auto vt = computePairHash(collisionData->VTColData[i].vertexIdx, collisionData->VTColData[i].triIdx);
            EXPECT_EQ(VTCollisions.find(vt) != VTCollisions.end(), true);
        }
        for (size_t i = 0; i < collisionData->EEColData.getSize(); ++i)
        {
            const auto edgeA = computePairHash(collisionData->EEColData[i].edgeIdA.first, collisionData->EEColData[i].edgeIdA.second);
            const auto edgeB = computePairHash(collisionData->EEColData[i].edgeIdB.first, collisionData->EEColData[i].edgeIdB.second);
            EXPECT_EQ(EECollisions.find(edgeA) != EECollisions.end(), true);
            EXPECT_EQ(EECollisions[edgeA].find(edgeB) != EECollisions[edgeA].end(), true);
        }
    }

protected:
    std::shared_ptr<OctreeBasedCD> m_OctreeCD;
};
} // end namespace imstk

///
/// \brief Test collision detection between pointset and surface mesh
///
TEST_F(OctreeBasedCDTest, TestPointMeshManual)
{
    for (int iter = 0; iter < ITERATIONS; ++iter)
    {
        testPointMeshManual();
    }
}

///
/// \brief Test collision detection between pointset and surface mesh
///
TEST_F(OctreeBasedCDTest, TestPointMeshUsingPointSetToSurfaceMeshCD)
{
    for (int iter = 0; iter < ITERATIONS; ++iter)
    {
        testPointMeshUsingPointSetToSurfaceMeshCD();
    }
}

///
/// \brief Test collision detection between pointset and sphere
///
TEST_F(OctreeBasedCDTest, TestPointSphere)
{
    for (int iter = 0; iter < ITERATIONS; ++iter)
    {
        testPointSphere();
    }
}

///
/// \brief Test collision detection between surface mesh and surface mesh
///
TEST_F(OctreeBasedCDTest, TestMeshMeshManual)
{
    for (int iter = 0; iter < ITERATIONS; ++iter)
    {
        testMeshMeshManual();
    }
}

///
/// \brief Test collision detection between surface mesh and surface mesh
///
TEST_F(OctreeBasedCDTest, TestMeshMeshUsingSurfaceMeshToSurfaceMeshCD)
{
    testMeshMeshUsingSurfaceMeshToSurfaceMeshCD();
    for (int iter = 0; iter < ITERATIONS; ++iter)
    {
        testMeshMeshUsingSurfaceMeshToSurfaceMeshCD();
    }
}
