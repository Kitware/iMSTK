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

#include "imstkPbdDistanceConstraint.h"
#include "imstkPbdConstraintFunctor.h"

namespace imstk
{
///
/// \class PbdInflatableDistanceConstraint
///
class PbdInflatableDistanceConstraint : public PbdDistanceConstraint
{
public:
    ///
    /// \brief Constructor
    ///
    PbdInflatableDistanceConstraint() : PbdDistanceConstraint() {}

    ///
    /// \brief Initializes the inflatable distance constraint
    ///
    void initConstraint(const VecDataArray<double, 3>& initVertexPositions,
                        const size_t& pIdx0,
                        const size_t& pIdx1,
                        const double k = 1e5)
    {
        PbdDistanceConstraint::initConstraint(initVertexPositions, pIdx0, pIdx1, k);
        m_initialRestLength = m_restLength;
    }

    void setRestLength(const double restLength) { m_restLength = restLength; }
    const double getRestLength() const { return m_restLength; }

    void setInitRestLength(const double initRestLength) { m_initialRestLength = initRestLength; }
    const double getInitRestLength() const { return m_initialRestLength; }

    ///
    /// \brief Reset constraint rest measurement
    ///
    void resetRestLength() { m_restLength = m_initialRestLength; }

public:
    double m_initialRestLength = 0.0; ///> Rest measurement(length, area, volume, etc.)
};

struct PbdInflatableDistanceConstraintFunctor : public PbdDistanceConstraintFunctor
{
    public:
        PbdInflatableDistanceConstraintFunctor() : PbdDistanceConstraintFunctor() {}
        ~PbdInflatableDistanceConstraintFunctor() override = default;

        virtual void operator()(PbdConstraintContainer& constraints) override
        {
            const VecDataArray<double, 3>& vertices = *m_geom->getVertexPositions();
            auto                           addDistConstraint =
                [&](std::vector<std::vector<bool>>& E, size_t i1, size_t i2)
                {
                    if (i1 > i2) // Make sure i1 is always smaller than i2
                    {
                        std::swap(i1, i2);
                    }
                    if (E[i1][i2])
                    {
                        E[i1][i2] = 0;
                        auto c = std::make_shared<PbdInflatableDistanceConstraint>();
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
};
} // imstk
