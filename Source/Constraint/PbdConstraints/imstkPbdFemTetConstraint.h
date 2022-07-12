/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkPbdFemConstraint.h"

namespace imstk
{
///
/// \class PbdFemTetConstraint
///
/// \brief The FEMTetConstraint class class for constraint as the elastic energy
/// computed by linear shape functions with tetrahedral mesh.
///
class PbdFemTetConstraint : public PbdFemConstraint
{
public:
    PbdFemTetConstraint(MaterialType mType = MaterialType::StVK) :
        PbdFemConstraint(4, mType) { }

    ///
    /// \brief Initialize the constraint
    ///
    bool initConstraint(
        const Vec3d& p0, const Vec3d& p1, const Vec3d& p2, const Vec3d& p3,
        const PbdParticleId& pIdx0, const PbdParticleId& pIdx1,
        const PbdParticleId& pIdx2, const PbdParticleId& pIdx3,
        std::shared_ptr<PbdFemConstraintConfig> config);

    ///
    /// \brief Compute value and gradient of constraint function
    /// \param[inout] set of bodies involved in system
    /// \param[inout] c constraint value
    /// \param[inout] dcdx constraint gradient
    ///
    bool computeValueAndGradient(PbdState& bodies,
                                 double& c, std::vector<Vec3d>& dcdx) override;

    ///
    /// \brief Handle inverted tets with the method described by Irving et. al. in
    /// "Invertible Finite Elements For Robust Simulation of Large Deformation"
    ///
    void handleInversions(
        Mat3d& F,
        Mat3d& U,
        Mat3d& Fhat,
        Mat3d& VT
        ) const;

    ///
    /// \brief Set/Get Inversion Handling
    ///@{
    void setInverstionHandling(const bool handleInversions) { m_handleInversions = handleInversions; }
    bool getInverstionHandling() const { return m_handleInversions; }
///@}

private:
    bool m_handleInversions = true;
};
} // namespace imstk