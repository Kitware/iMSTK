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

#include "imstkPbdInteractionPair.h"
#include "imstkSurfaceMesh.h"
#include "imstkLineMesh.h"
#include "imstkGeometryMap.h"
#include "imstkIntersectionTestUtils.h"
#include "imstkPbdModel.h"

namespace imstk
{
bool
PbdInteractionPair::doBroadPhaseCollision()
{
    auto g1 = first->getCollidingGeometry();
    auto g2 = second->getCollidingGeometry();
    auto mesh1 = std::static_pointer_cast<PointSet>(g1);
    auto mesh2 = std::static_pointer_cast<PointSet>(g2);

    Vec3d min1, max1;
    mesh1->computeBoundingBox(min1, max1);

    Vec3d min2, max2;
    mesh2->computeBoundingBox(min2, max2);

    auto dynaModel1 = std::static_pointer_cast<PbdModel>(first->getDynamicalModel());
    auto dynaModel2 = std::static_pointer_cast<PbdModel>(second->getDynamicalModel());

    auto prox1 = dynaModel1->getParameters()->m_proximity;
    auto prox2 = dynaModel2->getParameters()->m_proximity;

    return testAABBToAABB(min1[0] - prox1, max1[0] + prox1, min1[1] - prox1, max1[1] + prox1,
                          min1[2] - prox1, max1[2] + prox1, min2[0] - prox2, max2[0] + prox2,
                          min2[1] - prox2, max2[1] + prox2, min2[2] - prox2, max2[2] + prox2);
}

void
PbdInteractionPair::doNarrowPhaseCollision()
{
    auto g1 = first->getCollidingGeometry();
    auto g2 = second->getCollidingGeometry();

    auto map1 = first->getPhysicsToCollidingMap();
    auto map2 = second->getPhysicsToCollidingMap();

    auto dynaModel1 = std::static_pointer_cast<PbdModel>(first->getDynamicalModel());
    auto dynaModel2 = std::static_pointer_cast<PbdModel>(second->getDynamicalModel());

    auto prox1 = dynaModel1->getParameters()->m_proximity;
    auto prox2 = dynaModel2->getParameters()->m_proximity;

    auto mesh2 = std::static_pointer_cast<SurfaceMesh>(g2);

    if (g1->getType() == Geometry::Type::LineMesh)
    {
        auto mesh1 = std::static_pointer_cast<LineMesh>(g1);

        // brute force, use BVH or spatial grid would be much better
        // point
        for (size_t i = 0; i < mesh1->getNumVertices(); ++i)
        {
            Vec3d p = mesh1->getVertexPosition(i);
            std::vector<SurfaceMesh::TriangleArray> elements = mesh2->getTrianglesVertices();

            for (size_t j = 0; j < elements.size(); ++j)
            {
                SurfaceMesh::TriangleArray& e = elements[j];
                const Vec3d p0 = mesh2->getVertexPosition(e[0]);
                const Vec3d p1 = mesh2->getVertexPosition(e[1]);
                const Vec3d p2 = mesh2->getVertexPosition(e[2]);

                if (testPointToTriAABB(p[0], p[1], p[2],
                    p0[0], p0[1], p0[2],
                    p1[0], p1[1], p1[2],
                    p2[0], p2[1], p2[2], prox1, prox2))
                {
                    auto c = std::make_shared<PbdPointTriangleConstraint>();
                    c->initConstraint(dynaModel1, i,
                        dynaModel2, map2->getMapIdx(e[0]), map2->getMapIdx(e[1]), map2->getMapIdx(e[2]));
                    m_collisionConstraints.push_back(c);
                }
            }
        }

        const auto nL1 = mesh1->getNumLines();
        const auto nV2 = mesh2->getNumVertices();
        std::vector<std::vector<bool>> E2(nV2, std::vector<bool>(nV2, 1));
        std::vector<SurfaceMesh::TriangleArray> elements2 = mesh2->getTrianglesVertices();

        for (int k = 0; k < nL1; ++k)
        {
            auto nodes = mesh1->getLinesVertices()[k];
            unsigned int i1 = nodes[0];
            unsigned int i2 = nodes[1];

            const Vec3d P = mesh1->getVertexPosition(i1);
            const Vec3d Q = mesh1->getVertexPosition(i2);

            for (size_t j = 0; j < elements2.size(); ++j)
            {
                SurfaceMesh::TriangleArray& e = elements2[j];
                const Vec3d p0 = mesh2->getVertexPosition(e[0]);
                const Vec3d p1 = mesh2->getVertexPosition(e[1]);
                const Vec3d p2 = mesh2->getVertexPosition(e[2]);

                if (E2[e[0]][e[1]] && E2[e[1]][e[0]])
                {
                    if (testLineToLineAABB(P[0], P[1], P[2],
                        Q[0], Q[1], Q[2],
                        p0[0], p0[1], p0[2],
                        p1[0], p1[1], p1[2], prox1, prox2))
                    {
                        auto c = std::make_shared<PbdEdgeEdgeConstraint>();
                        c->initConstraint(dynaModel1, map1->getMapIdx(i1), map1->getMapIdx(i2),
                            dynaModel2, map2->getMapIdx(e[0]), map1->getMapIdx(e[1]));
                        m_collisionConstraints.push_back(c);
                    }
                    E2[e[0]][e[1]] = 0;
                }
                if (E2[e[1]][e[2]] && E2[e[2]][e[1]])
                {
                    if (testLineToLineAABB(P[0], P[1], P[2],
                        Q[0], Q[1], Q[2],
                        p1[0], p1[1], p1[2],
                        p2[0], p2[1], p2[2], prox1, prox2))
                    {
                        auto c = std::make_shared<PbdEdgeEdgeConstraint>();
                        c->initConstraint(dynaModel1, map1->getMapIdx(i1), map1->getMapIdx(i2),
                            dynaModel2, map2->getMapIdx(e[1]), map1->getMapIdx(e[2]));
                        m_collisionConstraints.push_back(c);
                    }
                    E2[e[1]][e[2]] = 0;
                }
                if (E2[e[2]][e[0]] && E2[e[0]][e[2]])
                {
                    if (testLineToLineAABB(P[0], P[1], P[2],
                        Q[0], Q[1], Q[2],
                        p2[0], p2[1], p2[2],
                        p0[0], p0[1], p0[2], prox1, prox2))
                    {
                        auto c = std::make_shared<PbdEdgeEdgeConstraint>();
                        c->initConstraint(dynaModel1, map1->getMapIdx(i1), map1->getMapIdx(i2),
                            dynaModel2, map2->getMapIdx(e[2]), map1->getMapIdx(e[0]));
                        m_collisionConstraints.push_back(c);
                    }
                    E2[e[2]][e[0]] = 0;
                }
            }
        }
    }
    else if (g1->getType() == Geometry::Type::PointSet)
    {
        auto mesh1 = std::static_pointer_cast<PointSet>(g1);

        // brute force, use BVH or spatial grid would be much better
        // point
        for (size_t i = 0; i < mesh1->getNumVertices(); ++i)
        {
            const auto p = mesh1->getVertexPosition(i);
            auto elements = mesh2->getTrianglesVertices();

            for (size_t j = 0; j < elements.size(); ++j)
            {
                auto& e = elements[j];
                const Vec3d p0 = mesh2->getVertexPosition(e[0]);
                const Vec3d p1 = mesh2->getVertexPosition(e[1]);
                const Vec3d p2 = mesh2->getVertexPosition(e[2]);

                if (testPointToTriAABB(p[0], p[1], p[2],
                    p0[0], p0[1], p0[2],
                    p1[0], p1[1], p1[2],
                    p2[0], p2[1], p2[2], prox1, prox2))
                {
                    auto c = std::make_shared<PbdPointTriangleConstraint>();
                    auto mappedIndex1 = (map1) ? map1->getMapIdx(i) : i;

                    c->initConstraint(dynaModel1, mappedIndex1, dynaModel2, map2->getMapIdx(e[0]), map2->getMapIdx(e[1]), map2->getMapIdx(e[2]));
                    m_collisionConstraints.push_back(c);
                }
            }
        }
    }
    else
    {
        auto mesh1 = std::static_pointer_cast<SurfaceMesh>(g1);

        // brute force, use BVH or spatial grid would be much better
        // point
        for (size_t i = 0; i < mesh1->getNumVertices(); ++i)
        {
            const Vec3d p = mesh1->getVertexPosition(i);
            auto elements = mesh2->getTrianglesVertices();

            for (size_t j = 0; j < elements.size(); ++j)
            {
                SurfaceMesh::TriangleArray& e = elements[j];
                const Vec3d p0 = mesh2->getVertexPosition(e[0]);
                const Vec3d p1 = mesh2->getVertexPosition(e[1]);
                const Vec3d p2 = mesh2->getVertexPosition(e[2]);

                if (testPointToTriAABB(p[0], p[1], p[2],
                    p0[0], p0[1], p0[2],
                    p1[0], p1[1], p1[2],
                    p2[0], p2[1], p2[2], prox1, prox2))
                {
                    auto c = std::make_shared<PbdPointTriangleConstraint>();
                    c->initConstraint(dynaModel1, map1->getMapIdx(i),
                        dynaModel2, map2->getMapIdx(e[0]), map2->getMapIdx(e[1]), map2->getMapIdx(e[2]));
                    m_collisionConstraints.push_back(c);
                }
            }
        }

        // edge
        // since we don't have edge structure, the following is not good
        const auto nV = mesh1->getNumVertices();
        std::vector<std::vector<bool>> E(nV, std::vector<bool>(nV, 1));

        const auto nV2 = mesh2->getNumVertices();
        std::vector<std::vector<bool>> E2(nV2, std::vector<bool>(nV2, 1));

        std::vector<SurfaceMesh::TriangleArray> elements = mesh1->getTrianglesVertices();
        for (size_t k = 0; k < elements.size(); ++k)
        {
            SurfaceMesh::TriangleArray& tri = elements[k];
            size_t i1 = tri[0];
            size_t i2 = tri[1];

            if (E[i1][i2] && E[i2][i1])
            {
                const Vec3d P = mesh1->getVertexPosition(i1);
                const Vec3d Q = mesh1->getVertexPosition(i2);
                std::vector<SurfaceMesh::TriangleArray> elements2 = mesh2->getTrianglesVertices();
                for (size_t j = 0; j < elements2.size(); ++j)
                {
                    SurfaceMesh::TriangleArray& e = elements2[j];
                    const Vec3d p0 = mesh2->getVertexPosition(e[0]);
                    const Vec3d p1 = mesh2->getVertexPosition(e[1]);
                    const Vec3d p2 = mesh2->getVertexPosition(e[2]);
                    if (E2[e[0]][e[1]] && E2[e[1]][e[0]])
                    {
                        if (testLineToLineAABB(P[0], P[1], P[2],
                            Q[0], Q[1], Q[2],
                            p0[0], p0[1], p0[2],
                            p1[0], p1[1], p1[2], prox1, prox2))
                        {
                            auto c = std::make_shared<PbdEdgeEdgeConstraint>();
                            c->initConstraint(dynaModel1, map1->getMapIdx(i1), map1->getMapIdx(i2),
                                dynaModel2, map2->getMapIdx(e[0]), map1->getMapIdx(e[1]));
                            m_collisionConstraints.push_back(c);
                            E2[e[0]][e[1]] = 0;
                        }
                    }
                    if (E2[e[1]][e[2]] && E2[e[2]][e[1]])
                    {
                        if (testLineToLineAABB(P[0], P[1], P[2],
                            Q[0], Q[1], Q[2],
                            p1[0], p1[1], p1[2],
                            p2[0], p2[1], p2[2], prox1, prox2))
                        {
                            auto c = std::make_shared<PbdEdgeEdgeConstraint>();
                            c->initConstraint(dynaModel1, map1->getMapIdx(i1), map1->getMapIdx(i2),
                                dynaModel2, map2->getMapIdx(e[1]), map1->getMapIdx(e[2]));
                            m_collisionConstraints.push_back(c);
                            E2[e[1]][e[2]] = 0;
                        }
                    }
                    if (E2[e[2]][e[0]] && E2[e[0]][e[2]])
                    {
                        if (testLineToLineAABB(P[0], P[1], P[2],
                            Q[0], Q[1], Q[2],
                            p2[0], p2[1], p2[2],
                            p0[0], p0[1], p0[2], prox1, prox2))
                        {
                            auto c = std::make_shared<PbdEdgeEdgeConstraint>();
                            c->initConstraint(dynaModel1, map1->getMapIdx(i1), map1->getMapIdx(i2),
                                dynaModel2, map2->getMapIdx(e[2]), map1->getMapIdx(e[0]));
                            m_collisionConstraints.push_back(c);
                            E2[e[2]][e[0]] = 0;
                        }
                    }
                }
                E[i1][i2] = 0;
            }

            i1 = tri[1];
            i2 = tri[2];
            if (E[i1][i2] && E[i2][i1])
            {
                const Vec3d P = mesh1->getVertexPosition(i1);
                const Vec3d Q = mesh1->getVertexPosition(i2);
                std::vector<SurfaceMesh::TriangleArray> elements2 = mesh2->getTrianglesVertices();

                for (size_t j = 0; j < elements2.size(); ++j)
                {
                    SurfaceMesh::TriangleArray& e = elements2[j];
                    const Vec3d p0 = mesh2->getVertexPosition(e[0]);
                    const Vec3d p1 = mesh2->getVertexPosition(e[1]);
                    const Vec3d p2 = mesh2->getVertexPosition(e[2]);
                    if (E2[e[0]][e[1]] && E2[e[1]][e[0]])
                    {
                        if (testLineToLineAABB(P[0], P[1], P[2],
                            Q[0], Q[1], Q[2],
                            p0[0], p0[1], p0[2],
                            p1[0], p1[1], p1[2], prox1, prox2))
                        {
                            auto c = std::make_shared<PbdEdgeEdgeConstraint>();
                            c->initConstraint(dynaModel1, map1->getMapIdx(i1), map1->getMapIdx(i2),
                                dynaModel2, map2->getMapIdx(e[0]), map1->getMapIdx(e[1]));
                            m_collisionConstraints.push_back(c);
                            E2[e[0]][e[1]] = 0;
                        }
                    }
                    if (E2[e[1]][e[2]] && E2[e[2]][e[1]])
                    {
                        if (testLineToLineAABB(P[0], P[1], P[2],
                            Q[0], Q[1], Q[2],
                            p1[0], p1[1], p1[2],
                            p2[0], p2[1], p2[2], prox1, prox2))
                        {
                            auto c = std::make_shared<PbdEdgeEdgeConstraint>();
                            c->initConstraint(dynaModel1, map1->getMapIdx(i1), map1->getMapIdx(i2),
                                dynaModel2, map2->getMapIdx(e[1]), map1->getMapIdx(e[2]));
                            m_collisionConstraints.push_back(c);
                            E2[e[1]][e[2]] = 0;
                        }
                    }
                    if (E2[e[2]][e[0]] && E2[e[0]][e[2]])
                    {
                        if (testLineToLineAABB(P[0], P[1], P[2],
                            Q[0], Q[1], Q[2],
                            p2[0], p2[1], p2[2],
                            p0[0], p0[1], p0[2], prox1, prox2))
                        {
                            auto c = std::make_shared<PbdEdgeEdgeConstraint>();
                            c->initConstraint(dynaModel1, map1->getMapIdx(i1), map1->getMapIdx(i2),
                                dynaModel2, map2->getMapIdx(e[2]), map1->getMapIdx(e[0]));
                            m_collisionConstraints.push_back(c);
                            E2[e[2]][e[0]] = 0;
                        }
                    }
                }
                E[i1][i2] = 0;
            }

            i1 = tri[2];
            i2 = tri[0];
            if (E[i1][i2] && E[i2][i1])
            {
                const Vec3d P = mesh1->getVertexPosition(i1);
                const Vec3d Q = mesh1->getVertexPosition(i2);
                std::vector<SurfaceMesh::TriangleArray> elements2 = mesh2->getTrianglesVertices();
                for (size_t j = 0; j < elements2.size(); ++j)
                {
                    SurfaceMesh::TriangleArray& e = elements2[j];
                    const Vec3d p0 = mesh2->getVertexPosition(e[0]);
                    const Vec3d p1 = mesh2->getVertexPosition(e[1]);
                    const Vec3d p2 = mesh2->getVertexPosition(e[2]);
                    if (E2[e[0]][e[1]] && E2[e[1]][e[0]])
                    {
                        if (testLineToLineAABB(P[0], P[1], P[2],
                            Q[0], Q[1], Q[2],
                            p0[0], p0[1], p0[2],
                            p1[0], p1[1], p1[2], prox1, prox2))
                        {
                            auto c = std::make_shared<PbdEdgeEdgeConstraint>();
                            c->initConstraint(dynaModel1, map1->getMapIdx(i1), map1->getMapIdx(i2),
                                dynaModel2, map2->getMapIdx(e[0]), map1->getMapIdx(e[1]));
                            m_collisionConstraints.push_back(c);
                            E2[e[0]][e[1]] = 0;
                        }
                    }
                    if (E2[e[1]][e[2]] && E2[e[2]][e[1]])
                    {
                        if (testLineToLineAABB(P[0], P[1], P[2],
                            Q[0], Q[1], Q[2],
                            p1[0], p1[1], p1[2],
                            p2[0], p2[1], p2[2], prox1, prox2))
                        {
                            auto c = std::make_shared<PbdEdgeEdgeConstraint>();
                            c->initConstraint(dynaModel1, map1->getMapIdx(i1), map1->getMapIdx(i2),
                                dynaModel2, map2->getMapIdx(e[1]), map1->getMapIdx(e[2]));
                            m_collisionConstraints.push_back(c);
                            E2[e[1]][e[2]] = 0;
                        }
                    }
                    if (E2[e[2]][e[0]] && E2[e[0]][e[2]])
                    {
                        if (testLineToLineAABB(P[0], P[1], P[2],
                            Q[0], Q[1], Q[2],
                            p2[0], p2[1], p2[2],
                            p0[0], p0[1], p0[2], prox1, prox2))
                        {
                            auto c = std::make_shared<PbdEdgeEdgeConstraint>();
                            c->initConstraint(dynaModel1, map1->getMapIdx(i1), map1->getMapIdx(i2),
                                dynaModel2, map2->getMapIdx(e[2]), map1->getMapIdx(e[0]));
                            m_collisionConstraints.push_back(c);
                            E2[e[2]][e[0]] = 0;
                        }
                    }
                }
                E[i1][i2] = 0;
            }
        }
    }
}

void
PbdInteractionPair::resolveCollision()
{
    if (!m_collisionConstraints.empty())
    {
        unsigned int i = 0;
        while (++i < maxIter)
        {
            for (size_t k = 0; k < m_collisionConstraints.size(); ++k)
            {
                m_collisionConstraints[k]->solvePositionConstraint();
            }
        }
    }
}
} // imstk
