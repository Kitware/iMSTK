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
    ///
    /// \brief constructor
    ///
    PbdInflatableVolumeConstraint() : PbdVolumeConstraint() {}

    ///
    /// \brief Initializes the inflatable volume constraint
    ///
    void initConstraint(const VecDataArray<double, 3>& initVertexPositions,
                        const size_t& pIdx1, const size_t& pIdx2,
                        const size_t& pIdx3, const size_t& pIdx4,
                        const double k = 2.0);

    ///
    /// \brief Soften or strengthen constraint by multiplying current restVolume by a ratio
    ///
    void multiplyRestVolumeBy(const double ratio);

    ///
    /// \brief Soften or strengthen constraint by multiplying initial restVolume by a ratio
    ///
    void multiplyInitRestVolumeBy(const double ratio) { m_restVolume = ratio * m_initialRestVolume; }

    ///
    /// \brief Reset constraint rest volume
    ///
    void resetRestVolume() { m_restVolume = m_initialRestVolume; }

protected:
    double m_initialRestVolume = 0.0; ///> Rest measurement(length, area, volume, etc.)
};

struct  PbdInflatableVolumeConstraintFunctor : public PbdVolumeConstraintFunctor
{
    PbdInflatableVolumeConstraintFunctor() : PbdVolumeConstraintFunctor() {}
    ~PbdInflatableVolumeConstraintFunctor() override = default;

    virtual void operator()(PbdConstraintContainer& constraints) override
    {
        // Check if constraint type matches the mesh type
        CHECK(m_geom->getTypeName() == "TetrahedralMesh")
            << "Volume constraint should come with volumetric mesh";

        // Create constraints
        auto                           tetMesh  = std::dynamic_pointer_cast<TetrahedralMesh>(m_geom);
        const VecDataArray<double, 3>& vertices = *m_geom->getVertexPositions();
        const VecDataArray<int, 4>&    elements = *tetMesh->getTetrahedraIndices();

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
