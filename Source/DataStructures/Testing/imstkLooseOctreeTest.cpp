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

#include "imstkLooseOctree.h"
#include "imstkSurfaceMesh.h"
#include "imstkVecDataArray.h"

#include <gtest/gtest.h>

using namespace imstk;

#define BOUND           10.0

#define SPHERE_RADIUS   Real(10)
#define SPHERE_CENTER   Vec3r(0, 0, 0)
#define PARTICLE_RADIUS Real(2)
#define ITERATIONS      10

///
/// \brief Generate a PointSet
///
std::shared_ptr<PointSet>
generatePointSet()
{
    const Vec3r sphereCenter    = SPHERE_CENTER;
    const auto  sphereRadiusSqr = SPHERE_RADIUS * SPHERE_RADIUS;
    const auto  spacing = Real(2) * PARTICLE_RADIUS;
    const int   N       = int(2 * SPHERE_RADIUS / spacing);

    std::shared_ptr<VecDataArray<double, 3>> particles = std::make_shared<VecDataArray<double, 3>>();
    VecDataArray<double, 3>&                 vertices  = *particles;
    vertices.reserve(N * N * N);
    const Vec3r corner = sphereCenter - Vec3r(SPHERE_RADIUS, SPHERE_RADIUS, SPHERE_RADIUS);

    for (int i = 0; i < N; ++i)
    {
        for (int j = 0; j < N; ++j)
        {
            for (int k = 0; k < N; ++k)
            {
                const Vec3r ppos = corner + Vec3r(spacing * Real(i), spacing * Real(j), spacing * Real(k));
                const Vec3r d    = ppos - sphereCenter;
                if (d.squaredNorm() < sphereRadiusSqr)
                {
                    vertices.push_back(ppos);
                }
            }
        }
    }

    auto pointset = std::make_shared<PointSet>();
    pointset->initialize(particles);
    return pointset;
}

///
/// \brief Generate a triangle soup (random discrete triangles)
///
std::shared_ptr<SurfaceMesh>
generateMesh()
{
    std::shared_ptr<VecDataArray<double, 3>> verticesPtr = std::make_shared<VecDataArray<double, 3>>();
    VecDataArray<double, 3>&                 vertices    = *verticesPtr;
    std::shared_ptr<VecDataArray<int, 3>>    indicesPtr  = std::make_shared<VecDataArray<int, 3>>();
    VecDataArray<int, 3>&                    indices     = *indicesPtr;

    auto randD = [] { return (static_cast<double>(rand()) / static_cast<double>(RAND_MAX) * 2.0 - 1.0) * BOUND; };

    vertices.reserve(300);
    indices.reserve(100);
    for (int i = 0; i < 100; ++i)
    {
        indices.push_back(Vec3i(i * 3, i * 3 + 1, i * 3 + 2));
        vertices.push_back(Vec3d(randD(), randD(), randD()));
        vertices.push_back(Vec3d(randD(), randD(), randD()));
        vertices.push_back(Vec3d(randD(), randD(), randD()));
    }
    auto mesh = std::make_shared<SurfaceMesh>();
    mesh->initialize(verticesPtr, indicesPtr);
    return mesh;
}

///
/// \brief Randomize particle positions
///
void
randomizePositions(const std::shared_ptr<PointSet>& pointset)
{
    for (int i = 0; i < pointset->getNumVertices(); ++i)
    {
        pointset->setVertexPosition(i, Vec3r(
            (static_cast<Real>(rand()) / static_cast<Real>(RAND_MAX) * 2.0 - 1.0) * BOUND,
            (static_cast<Real>(rand()) / static_cast<Real>(RAND_MAX) * 2.0 - 1.0) * BOUND,
            (static_cast<Real>(rand()) / static_cast<Real>(RAND_MAX) * 2.0 - 1.0) * BOUND
            ));
    }
    pointset->postModified();
}

///
/// \brief Randomize triangle positions
///
void
randomizePositions(const std::shared_ptr<SurfaceMesh>& mesh)
{
    for (int i = 0; i < mesh->getNumTriangles(); ++i)
    {
        const auto translation = Vec3r(
            (static_cast<Real>(rand()) / static_cast<Real>(RAND_MAX) * 2.0 - 1.0) * BOUND,
            (static_cast<Real>(rand()) / static_cast<Real>(RAND_MAX) * 2.0 - 1.0) * BOUND,
            (static_cast<Real>(rand()) / static_cast<Real>(RAND_MAX) * 2.0 - 1.0) * BOUND
            );
        const Vec3i& face = (*mesh->getTriangleIndices())[i];
        for (unsigned int j = 0; j < 3; ++j)
        {
            mesh->setVertexPosition(face[j], mesh->getVertexPosition(face[j]) + translation);
        }
    }
    mesh->postModified();
}

namespace imstk
{
class LooseOctreeTest : public ::testing::Test
{
public:
    LooseOctreeTest() {}

    void reset()
    {
        m_Octree.reset(new LooseOctree(Vec3d(0, 0, 0), 100.0, 0.1, 4));
    }

    void buildExample()
    {
        reset();

        m_PointSet = generatePointSet();
        m_Octree->addPointSet(m_PointSet);
        EXPECT_EQ(m_Octree->m_vPrimitivePtrs[OctreePrimitiveType::Point].size(), m_PointSet->getNumVertices());
        EXPECT_EQ(m_Octree->m_vPrimitivePtrs[OctreePrimitiveType::Triangle].size(), 0);
        EXPECT_EQ(m_Octree->m_vPrimitivePtrs[OctreePrimitiveType::Analytical].size(), 0);

        m_Mesh = generateMesh();
        m_Octree->addTriangleMesh(m_Mesh);
        EXPECT_EQ(m_Octree->m_vPrimitivePtrs[OctreePrimitiveType::Point].size(), m_PointSet->getNumVertices());
        EXPECT_EQ(m_Octree->m_vPrimitivePtrs[OctreePrimitiveType::Triangle].size(), m_Mesh->getNumTriangles());
        EXPECT_EQ(m_Octree->m_vPrimitivePtrs[OctreePrimitiveType::Analytical].size(), 0);

        m_Octree->build();
    }

    void testOctree()
    {
        m_Octree->update();
        // Check node pool
        {
            unsigned int numInactive = 0;
            auto         pIter       = m_Octree->m_pNodeBlockPoolHead;
            while (pIter) {
                pIter        = pIter->m_NextBlock;
                numInactive += 8u;
            }
            EXPECT_EQ(numInactive, m_Octree->m_NumAvaiableBlocksInPool * 8u);
            ////////////////////////////////////////////////////////////////////////////////
            unsigned int            numActives = 0;
            std::queue<OctreeNode*> nodeQ;
            nodeQ.push(m_Octree->getRootNode());
            while (!nodeQ.empty()) {
                ++numActives;
                const auto node = nodeQ.front();
                nodeQ.pop();
                if (!node->isLeaf())
                {
                    for (unsigned int childIdx = 0; childIdx < 8u; ++childIdx)
                    {
                        nodeQ.push(node->getChildNode(childIdx));
                    }
                }
            }
            EXPECT_EQ(numActives, m_Octree->getNumActiveNodes());
            EXPECT_EQ(numActives + numInactive, m_Octree->m_NumAllocatedNodes);
        }

        // Check primitives in nodes
        {
            unsigned int primitiveCounts[OctreePrimitiveType::NumPrimitiveTypes];
            for (int i = 0; i < OctreePrimitiveType::NumPrimitiveTypes; ++i)
            {
                primitiveCounts[i] = 0;
            }

            for (const auto& it:  m_Octree->m_sActiveTreeNodeBlocks)
            {
                for (unsigned int idx = 0; idx < 8u; ++idx)
                {
                    for (int i = 0; i < OctreePrimitiveType::NumPrimitiveTypes; ++i)
                    {
                        primitiveCounts[i] += it->m_Nodes[idx].m_PrimitiveCounts[i];

                        unsigned int numPrimitives = 0;
                        auto         pIter = it->m_Nodes[idx].m_pPrimitiveListHeads[i];
                        while (pIter) {
                            pIter = pIter->m_pNext;
                            ++numPrimitives;
                        }
                        EXPECT_EQ(numPrimitives, it->m_Nodes[idx].m_PrimitiveCounts[i]);
                    }
                }
            }

            for (int i = 0; i < OctreePrimitiveType::NumPrimitiveTypes; ++i)
            {
                EXPECT_EQ(primitiveCounts[i], m_Octree->m_vPrimitivePtrs[i].size());
            }
        }

        // Check validity of primitives
        {
            for (int i = 0; i < OctreePrimitiveType::NumPrimitiveTypes; ++i)
            {
                for (const auto& pPrimitive : m_Octree->m_vPrimitivePtrs[i])
                {
                    const auto pNode = pPrimitive->m_pNode;
                    if (pPrimitive->m_pGeometry->getTypeName() == "PointSet")
                    {
                        EXPECT_EQ(pNode->looselyContains(pPrimitive->m_Position), true);
                    }
                    else
                    {
                        EXPECT_EQ(pNode->looselyContains(pPrimitive->m_LowerCorner, pPrimitive->m_UpperCorner), true);
                    }
                }
            }
        }
    }

    void testDummyPrimitives(bool bRebuild)
    {
        // Test points
        reset();
        m_Octree->setAlwaysRebuild(bRebuild);
        std::vector<std::shared_ptr<PointSet>> pointsets;
        for (int iter = 0; iter < ITERATIONS; ++iter)
        {
            const auto                               pointset = std::make_shared<PointSet>();
            std::shared_ptr<VecDataArray<double, 3>> vertices = std::make_shared<VecDataArray<double, 3>>();
            vertices->push_back(Vec3d(rand(), rand(), rand()) * 10.0 / static_cast<double>(RAND_MAX));
            pointset->initialize(vertices);
            m_Octree->addPointSet(pointset);
            pointsets.push_back(std::move(pointset));

            m_Octree->build();
            EXPECT_EQ(m_Octree->m_MaxDepth, 10U);
            const auto vPrimitives = m_Octree->m_vPrimitivePtrs[OctreePrimitiveType::Point];
            EXPECT_EQ(vPrimitives.size(), iter + 1);

            const auto pPrimitive = vPrimitives.back();
            const auto pNode      = pPrimitive->m_pNode;
            EXPECT_EQ(pNode->m_Depth, 10U);
            EXPECT_EQ(std::abs(pNode->m_HalfWidth * 2.0 - 0.1953125) < 1e-8, true);
            EXPECT_EQ(pNode->looselyContains(pPrimitive->m_Position), true);
            EXPECT_EQ(pNode->contains(pPrimitive->m_Position), true);
        }

        auto testTriangle =
            [&](bool bHasPoint)
            {
                std::vector<std::shared_ptr<SurfaceMesh>> meshes;
                for (int iter = 0; iter < ITERATIONS; ++iter)
                {
                    std::shared_ptr<VecDataArray<double, 3>> vertices = std::make_shared<VecDataArray<double, 3>>();
                    *vertices =
                    {
                        Vec3d(0, 0, 0),
                        Vec3d(1, 0, 0),
                        Vec3d(1, 1, 1)
                    };

                    std::shared_ptr<SurfaceMesh>          surfMesh = std::make_shared<SurfaceMesh>();
                    std::shared_ptr<VecDataArray<int, 3>> indices  = std::make_shared<VecDataArray<int, 3>>();
                    indices->push_back(Vec3i(0, 1, 2));
                    surfMesh->initialize(vertices, indices);
                    m_Octree->addTriangleMesh(surfMesh);
                    meshes.push_back(std::move(surfMesh));

                    m_Octree->build();
                    const auto vPrimitives = m_Octree->m_vPrimitivePtrs[OctreePrimitiveType::Triangle];
                    EXPECT_EQ(vPrimitives.size(), iter + 1);
                    const auto pPrimitive = vPrimitives.back();
                    const auto pNode      = pPrimitive->m_pNode;
                    EXPECT_EQ(pNode->looselyContains(pPrimitive->m_LowerCorner, pPrimitive->m_UpperCorner), true);

                    if (bHasPoint)
                    {
                        EXPECT_EQ(m_Octree->m_MaxDepth, 10U);
                        EXPECT_EQ(pNode->m_Depth, 8U);
                        EXPECT_EQ(std::abs(pNode->m_HalfWidth * 2.0 - 0.78125) < 1e-8, true);
                    }
                    else
                    {
                        EXPECT_EQ(m_Octree->m_MaxDepth, 5U);
                        EXPECT_EQ(pNode->m_Depth, 5U);
                        EXPECT_EQ(std::abs(pNode->m_HalfWidth * 2.0 - 6.25) < 1e-8, true);
                    }
                }
            };

        // Test triangles with points generated above
        testTriangle(true);

        // Test only triangles
        reset();
        m_Octree->setAlwaysRebuild(bRebuild);
        testTriangle(false);
    }

protected:
    std::shared_ptr<LooseOctree> m_Octree;
    std::shared_ptr<PointSet>    m_PointSet;
    std::shared_ptr<SurfaceMesh> m_Mesh;
};
} // end namespace imstk

///
/// \brief Test octree update while primitives moving around randomly
///
TEST_F(LooseOctreeTest, TestTeleportingPrimitives)
{
    buildExample();
    m_Octree->setAlwaysRebuild(true);
    for (int iter = 0; iter < ITERATIONS; ++iter)
    {
        testOctree();
        randomizePositions(m_PointSet);
        randomizePositions(m_Mesh);
    }

    ////////////////////////////////////////////////////////////////////////////////
    buildExample();
    m_Octree->setAlwaysRebuild(false);
    for (int iter = 0; iter < ITERATIONS; ++iter)
    {
        testOctree();
        randomizePositions(m_PointSet);
        randomizePositions(m_Mesh);
    }
}
