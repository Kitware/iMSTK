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

#include "imstkPbdVolumeConstraint.h"
#include "imstkPbdConstraintFunctor.h"

namespace imstk
{
///
/// \class PbdInflatableVolumeConstraint
///
class PbdInflatableVolumeConstraint : public PbdVolumeConstraint
{
public:
    PbdInflatableVolumeConstraint() : PbdVolumeConstraint() { }

    ///
    /// \brief Initializes the inflatable volume constraint
    ///
    void initConstraint(const VecDataArray<double, 3>& initVertexPositions,
                        const size_t& pIdx0, const size_t& pIdx1,
                        const size_t& pIdx2, const size_t& pIdx3,
                        const double k = 2.0)
    {
        PbdVolumeConstraint::initConstraint(initVertexPositions, pIdx0, pIdx1, pIdx2, pIdx3, k);
        m_initialRestVolume = m_restVolume;
    }

    const double getRestVolume() const { return m_restVolume; }
    const double getInitRestVolume() const { return m_initialRestVolume; }

    void setRestVolume(const double restVolume) { m_restVolume = restVolume; }
    void setInitRestVolume(const double initRestVolume) { m_initialRestVolume = initRestVolume; }

    ///
    /// \brief Reset constraint rest volume
    ///
    void resetRestVolume() { m_restVolume = m_initialRestVolume; }
    void resetStiffness() { m_stiffness = m_initialStiffness; }

    bool isInflated() const { return m_inflated; }
    void setInflated() { m_inflated = true; }

    double getCurrentVolume(std::shared_ptr<VecDataArray<double, 3>> currVertexPositions)
    {
        VecDataArray<double, 3> positions = *currVertexPositions;
        Vec3d                   p0 = positions[m_vertexIds[0]];
        Vec3d                   p1 = positions[m_vertexIds[1]];
        Vec3d                   p2 = positions[m_vertexIds[2]];
        Vec3d                   p3 = positions[m_vertexIds[3]];

        return ((p1 - p0).cross(p2 - p0)).dot(p3 - p0) / 6.0;
    }

protected:
    double m_initialRestVolume = 0.0;
    double m_initialStiffness  = 1.0;
    double m_diffusionRate     = 0.00001;

    bool m_inflated = false;
};

struct PbdInflatableVolumeConstraintFunctor : public PbdVolumeConstraintFunctor
{
    PbdInflatableVolumeConstraintFunctor() : PbdVolumeConstraintFunctor() {}
    ~PbdInflatableVolumeConstraintFunctor() override = default;

    void operator()(PbdConstraintContainer& constraints) override
    {
        // Check for correct mesh type
        CHECK(std::dynamic_pointer_cast<TetrahedralMesh>(m_geom) != nullptr)
            << "PbdInflatableVolumeConstraint can only be generated with a TetrahedralMesh";

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
                auto c    = std::make_shared<PbdInflatableVolumeConstraint>();
                c->initConstraint(vertices,
                    tet[0], tet[1], tet[2], tet[3], m_stiffness);
                constraints.addConstraint(c);
            });
    }
};
} // imstk
