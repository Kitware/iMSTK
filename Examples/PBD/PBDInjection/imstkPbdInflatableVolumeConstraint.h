/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

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
    void initConstraint(const Vec3d& p0, const Vec3d& p1, const Vec3d& p2, const Vec3d& p3,
                        const PbdParticleId& pIdx0, const PbdParticleId& pIdx1,
                        const PbdParticleId& pIdx2, const PbdParticleId& pIdx3,
                        const double k = 2.0)
    {
        PbdVolumeConstraint::initConstraint(p0, p1, p2, p3, pIdx0, pIdx1, pIdx2, pIdx3, k);
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
        std::shared_ptr<VecDataArray<int, 4>>    elementsPtr = tetMesh->getCells();
        const VecDataArray<int, 4>&              elements    = *elementsPtr;

        ParallelUtils::parallelFor(elements.size(),
            [&](const int k)
            {
                auto& tet = elements[k];
                auto c    = std::make_shared<PbdInflatableVolumeConstraint>();
                c->initConstraint(
                    vertices[tet[0]], vertices[tet[1]], vertices[tet[2]], vertices[tet[3]],
                    { m_bodyIndex, tet[0] }, { m_bodyIndex, tet[1] },
                    { m_bodyIndex, tet[2] }, { m_bodyIndex, tet[3] },
                    m_stiffness);
                constraints.addConstraint(c);
            });
    }
};
} // imstk
