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

#pragma once

#include "imstkLineMesh.h"
#include "imstkLogger.h"
#include "imstkParallelUtils.h"
#include "imstkPbdAreaConstraint.h"
#include "imstkPbdBendConstraint.h"
#include "imstkPbdConstantDensityConstraint.h"
#include "imstkPbdConstraint.h"
#include "imstkPbdDihedralConstraint.h"
#include "imstkPbdDistanceConstraint.h"
#include "imstkPbdFEMConstraint.h"
#include "imstkPbdFETetConstraint.h"
#include "imstkPbdVolumeConstraint.h"
#include "imstkPointSet.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkPbdConstraintContainer.h"

namespace imstk
{
///
/// \brief PbdConstraintFunctor take input geometry and produce constraints.
/// It exists to allow extensible constraint generation
///
struct PbdConstraintFunctor
{
    public:
        PbdConstraintFunctor() = default;
        virtual ~PbdConstraintFunctor() = default;

    public:
        ///
        /// \brief Appends a set of constraint to the container given a geometry
        ///
        virtual void operator()(PbdConstraintContainer& constraints) = 0;

        void setGeometry(std::shared_ptr<PointSet> geom) { m_geom = geom; }

    public:
        std::shared_ptr<PointSet> m_geom = nullptr;
};

struct PbdDistanceConstraintFunctor : public PbdConstraintFunctor
{
    public:
        PbdDistanceConstraintFunctor() = default;
        ~PbdDistanceConstraintFunctor() override = default;

    public:
        virtual void operator()(PbdConstraintContainer& constraints) override
        {
            std::shared_ptr<VecDataArray<double, 3>> verticesPtr = m_geom->getVertexPositions();
            const VecDataArray<double, 3>&           vertices = *verticesPtr;
            auto                                     addDistConstraint =
                [&](std::vector<std::vector<bool>>& E, size_t i1, size_t i2)
                {
                    if (i1 > i2) // Make sure i1 is always smaller than i2
                    {
                        std::swap(i1, i2);
                    }
                    if (E[i1][i2])
                    {
                        E[i1][i2] = 0;
                        auto c = std::make_shared<PbdDistanceConstraint>();
                        c->initConstraint(vertices, i1, i2, m_stiffness);
                        constraints.addConstraint(c);
                    }
                };

            if (m_geom->getTypeName() == "TetrahedralMesh")
            {
                const auto&                    tetMesh  = std::dynamic_pointer_cast<TetrahedralMesh>(m_geom);
                const VecDataArray<int, 4>&    elements = *tetMesh->getTetrahedraIndices();
                const auto                     nV       = tetMesh->getNumVertices();
                std::vector<std::vector<bool>> E(nV, std::vector<bool>(nV, 1));

                for (int k = 0; k < elements.size(); ++k)
                {
                    auto& tet = elements[k];
                    addDistConstraint(E, tet[0], tet[1]);
                    addDistConstraint(E, tet[0], tet[2]);
                    addDistConstraint(E, tet[0], tet[3]);
                    addDistConstraint(E, tet[1], tet[2]);
                    addDistConstraint(E, tet[1], tet[3]);
                    addDistConstraint(E, tet[2], tet[3]);
                }
            }
            else if (m_geom->getTypeName() == "SurfaceMesh")
            {
                const auto&                    triMesh  = std::dynamic_pointer_cast<SurfaceMesh>(m_geom);
                const VecDataArray<int, 3>&    elements = *triMesh->getTriangleIndices();
                const auto                     nV       = triMesh->getNumVertices();
                std::vector<std::vector<bool>> E(nV, std::vector<bool>(nV, 1));

                for (int k = 0; k < elements.size(); ++k)
                {
                    auto& tri = elements[k];
                    addDistConstraint(E, tri[0], tri[1]);
                    addDistConstraint(E, tri[0], tri[2]);
                    addDistConstraint(E, tri[1], tri[2]);
                }
            }
            else if (m_geom->getTypeName() == "LineMesh")
            {
                const auto&                    lineMesh = std::dynamic_pointer_cast<LineMesh>(m_geom);
                const VecDataArray<int, 2>&    elements = *lineMesh->getLinesIndices();
                const auto&                    nV       = lineMesh->getNumVertices();
                std::vector<std::vector<bool>> E(nV, std::vector<bool>(nV, 1));

                for (int k = 0; k < elements.size(); k++)
                {
                    auto& seg = elements[k];
                    addDistConstraint(E, seg[0], seg[1]);
                }
            }
        }

        void setStiffness(const double stiffness) { m_stiffness = stiffness; }

    protected:
        double m_stiffness = 0.0;
};

struct PbdFemConstraintFunctor : public PbdConstraintFunctor
{
    public:
        PbdFemConstraintFunctor() = default;
        ~PbdFemConstraintFunctor() override = default;

    public:
        virtual void operator()(PbdConstraintContainer& constraints) override
        {
            // Check if constraint type matches the mesh type
            CHECK(m_geom->getTypeName() == "TetrahedralMesh")
                << "FEM Tetrahedral constraint should come with tetrahedral mesh";

            // Create constraints
            auto                                     tetMesh     = std::dynamic_pointer_cast<TetrahedralMesh>(m_geom);
            std::shared_ptr<VecDataArray<double, 3>> verticesPtr = m_geom->getVertexPositions();
            const VecDataArray<double, 3>&           vertices    = *verticesPtr;
            std::shared_ptr<VecDataArray<int, 4>>    elementsPtr = tetMesh->getTetrahedraIndices();
            const VecDataArray<int, 4>&              elements    = *elementsPtr;

            ParallelUtils::parallelFor(elements.size(),
                [&](const size_t k)
            {
                const Vec4i& tet = elements[k];
                auto c = std::make_shared<PbdFEMTetConstraint>(m_matType);
                c->initConstraint(vertices,
                    tet[0], tet[1], tet[2], tet[3], m_femConfig);
                constraints.addConstraint(c);
            }, elements.size() > 100);
        }

        void setMaterialType(const PbdFEMTetConstraint::MaterialType materialType) { m_matType = materialType; }
        void setFemConfig(std::shared_ptr<PbdFEMConstraintConfig> femConfig) { m_femConfig = femConfig; }

    protected:
        PbdFEMTetConstraint::MaterialType m_matType = PbdFEMTetConstraint::MaterialType::StVK;
        std::shared_ptr<PbdFEMConstraintConfig> m_femConfig = nullptr;
};

struct PbdVolumeConstraintFunctor : public PbdConstraintFunctor
{
    public:
        PbdVolumeConstraintFunctor() = default;
        ~PbdVolumeConstraintFunctor() override = default;

    public:
        virtual void operator()(PbdConstraintContainer& constraints) override
        {
            // Check if constraint type matches the mesh type
            CHECK(m_geom->getTypeName() == "TetrahedralMesh")
                << "Volume constraint should come with volumetric mesh";

            // Create constraints
            auto                                     tetMesh     = std::dynamic_pointer_cast<TetrahedralMesh>(m_geom);
            std::shared_ptr<VecDataArray<double, 3>> verticesPtr = m_geom->getVertexPositions();
            const VecDataArray<double, 3>&           vertices    = *verticesPtr;
            std::shared_ptr<VecDataArray<int, 4>>    elementsPtr = tetMesh->getTetrahedraIndices();
            const VecDataArray<int, 4>&              elements    = *elementsPtr;

            ParallelUtils::parallelFor(elements.size(),
                [&](const size_t k)
            {
                auto& tet = elements[k];
                auto c    = std::make_shared<PbdVolumeConstraint>();
                c->initConstraint(vertices,
                    tet[0], tet[1], tet[2], tet[3], m_stiffness);
                constraints.addConstraint(c);
            });
        }

        void setStiffness(const double stiffness) { m_stiffness = stiffness; }

    protected:
        double m_stiffness = 0.0;
};

struct PbdAreaConstraintFunctor : public PbdConstraintFunctor
{
    public:
        PbdAreaConstraintFunctor() = default;
        ~PbdAreaConstraintFunctor() override = default;

    public:
        virtual void operator()(PbdConstraintContainer& constraints) override
        {
            // check if constraint type matches the mesh type
            CHECK(m_geom->getTypeName() == "SurfaceMesh")
                << "Area constraint should come with a triangular mesh";

            // ok, now create constraints
            auto                                     triMesh     = std::dynamic_pointer_cast<SurfaceMesh>(m_geom);
            std::shared_ptr<VecDataArray<double, 3>> verticesPtr = m_geom->getVertexPositions();
            const VecDataArray<double, 3>&           vertices    = *verticesPtr;
            std::shared_ptr<VecDataArray<int, 3>>    elemenstPtr = triMesh->getTriangleIndices();
            const VecDataArray<int, 3>&              elements    = *elemenstPtr;

            ParallelUtils::parallelFor(elements.size(),
                [&](const size_t k)
            {
                auto& tri = elements[k];
                auto c    = std::make_shared<PbdAreaConstraint>();
                c->initConstraint(vertices, tri[0], tri[1], tri[2], m_stiffness);
                constraints.addConstraint(c);
            });
        }

        void setStiffness(const double stiffness) { m_stiffness = stiffness; }

    protected:
        double m_stiffness = 0.0;
};

struct PbdBendConstraintFunctor : public PbdConstraintFunctor
{
    public:
        PbdBendConstraintFunctor() = default;
        ~PbdBendConstraintFunctor() override = default;

    public:
        virtual void operator()(PbdConstraintContainer& constraints) override
        {
            CHECK(m_geom->getTypeName() == "LineMesh")
                << "Bend constraint should come with a line mesh";

            auto                                     lineMesh    = std::dynamic_pointer_cast<LineMesh>(m_geom);
            std::shared_ptr<VecDataArray<double, 3>> verticesPtr = m_geom->getVertexPositions();
            const VecDataArray<double, 3>&           vertices    = *verticesPtr;
            /*std::shared_ptr< VecDataArray<int, 2>> indicesPtr = lineMesh->getLinesIndices();
            const VecDataArray<int, 2>& indices = *indicesPtr*/

            auto addBendConstraint =
                [&](const double k, size_t i1, size_t i2, size_t i3)
                {
                    // i1 should always come first
                    if (i2 < i1)
                    {
                        std::swap(i1, i2);
                    }
                    // i3 should always come last
                    if (i2 > i3)
                    {
                        std::swap(i2, i3);
                    }

                    auto c = std::make_shared<PbdBendConstraint>();
                    c->initConstraint(vertices, i1, i2, i3, k);
                    constraints.addConstraint(c);
                };

            // Iterate sets of stride # of segments
            for (int k = 0; k < vertices.size() - m_stride * 2; k += m_stride)
            {
                addBendConstraint(m_stiffness, k, k + m_stride, k + 2 * m_stride);
            }
        }

        void setStiffness(const double stiffness) { m_stiffness = stiffness; }
        void setStride(const int stride)
        {
            CHECK(m_stride > 1) << "Stride should be at least 1.";
            m_stride = stride;
        }

    protected:
        double m_stiffness = 0.0;
        int m_stride       = 3;
};

struct PbdDihedralConstraintFunctor : public PbdConstraintFunctor
{
    public:
        PbdDihedralConstraintFunctor() = default;
        ~PbdDihedralConstraintFunctor() override = default;

    public:
        virtual void operator()(PbdConstraintContainer& constraints) override
        {
            CHECK(m_geom->getTypeName() == "SurfaceMesh")
                << "Dihedral constraint should come with a triangular mesh";

            // Create constraints
            auto                                     triMesh     = std::dynamic_pointer_cast<SurfaceMesh>(m_geom);
            std::shared_ptr<VecDataArray<double, 3>> verticesPtr = triMesh->getVertexPositions();
            const VecDataArray<double, 3>&           vertices    = *verticesPtr;
            std::shared_ptr<VecDataArray<int, 3>>    elementsPtr = triMesh->getTriangleIndices();
            const VecDataArray<int, 3>&              elements    = *elementsPtr;
            const int                                nV = triMesh->getNumVertices();
            std::vector<std::vector<int>>            vertIdsToTriangleIds(nV);

            for (int k = 0; k < elements.size(); ++k)
            {
                const Vec3i& tri = elements[k];
                vertIdsToTriangleIds[tri[0]].push_back(k);
                vertIdsToTriangleIds[tri[1]].push_back(k);
                vertIdsToTriangleIds[tri[2]].push_back(k);
            }

            // Used to resolve duplicates
            std::vector<std::vector<bool>> E(nV, std::vector<bool>(nV, 1));

            auto addDihedralConstraint =
                [&](const std::vector<int>& r1, const std::vector<int>& r2,
                    const int k, int i1, int i2)
                {
                    if (i1 > i2) // Make sure i1 is always smaller than i2
                    {
                        std::swap(i1, i2);
                    }
                    if (E[i1][i2])
                    {
                        E[i1][i2] = 0;

                        // Find the shared edge
                        std::vector<size_t> rs(2);
                        auto                it = std::set_intersection(r1.begin(), r1.end(), r2.begin(), r2.end(), rs.begin());
                        rs.resize(static_cast<size_t>(it - rs.begin()));
                        if (rs.size() > 1)
                        {
                            size_t      idx  = (rs[0] == k) ? 1 : 0;
                            const auto& tri0 = elements[k];
                            const auto& tri1 = elements[rs[idx]];
                            size_t      idx0 = 0;
                            size_t      idx1 = 0;
                            for (size_t i = 0; i < 3; ++i)
                            {
                                if (tri0[i] != i1 && tri0[i] != i2)
                                {
                                    idx0 = tri0[i];
                                }
                                if (tri1[i] != i1 && tri1[i] != i2)
                                {
                                    idx1 = tri1[i];
                                }
                            }
                            auto c = std::make_shared<PbdDihedralConstraint>();
                            c->initConstraint(vertices, idx0, idx1, i1, i2, m_stiffness);
                            constraints.addConstraint(c);
                        }
                    }
                };

            for (int i = 0; i < vertIdsToTriangleIds.size(); i++)
            {
                std::sort(vertIdsToTriangleIds[i].begin(), vertIdsToTriangleIds[i].end());
            }

            // For every triangle
            for (int k = 0; k < elements.size(); ++k)
            {
                const Vec3i& tri = elements[k];

                // Get all the neighbor triangles (to the vertices)
                std::vector<int>& neighborTriangles0 = vertIdsToTriangleIds[tri[0]];
                std::vector<int>& neighborTriangles1 = vertIdsToTriangleIds[tri[1]];
                std::vector<int>& neighborTriangles2 = vertIdsToTriangleIds[tri[2]];

                // Add constraints between all the triangles
                addDihedralConstraint(neighborTriangles0, neighborTriangles1, k, tri[0], tri[1]);
                addDihedralConstraint(neighborTriangles0, neighborTriangles2, k, tri[0], tri[2]);
                addDihedralConstraint(neighborTriangles1, neighborTriangles2, k, tri[1], tri[2]);
            }
        }

        void setStiffness(const double stiffness) { m_stiffness = stiffness; }

    protected:
        double m_stiffness = 0.0;
};

struct PbdConstantDensityConstraintFunctor : public PbdConstraintFunctor
{
    public:
        PbdConstantDensityConstraintFunctor() = default;
        ~PbdConstantDensityConstraintFunctor() override = default;

    public:
        virtual void operator()(PbdConstraintContainer& constraints) override
        {
            // check if constraint type matches the mesh type
            CHECK(std::dynamic_pointer_cast<PointSet>(m_geom) != nullptr)
                << "Constant constraint should come with a mesh!";

            auto c = std::make_shared<PbdConstantDensityConstraint>();
            c->initConstraint(*m_geom->getVertexPositions(), m_stiffness);
            constraints.addConstraint(c);
        }

        void setStiffness(const double stiffness) { m_stiffness = stiffness; }

    protected:
        double m_stiffness = 0.0;
};
}