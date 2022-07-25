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
        PbdFemConstraint(4, mType) {}

    ///
    /// \brief Initialize the tetrahedral FEM constraint
    ///
    bool initConstraint(const VecDataArray<double, 3>& initVertexPositions,
                        const size_t& pIdx1, const size_t& pIdx2,
                        const size_t& pIdx3, const size_t& pIdx4,
                        std::shared_ptr<PbdFemConstraintConfig> config);

    ///
    /// \brief Compute the value and gradient of constraint
    ///
    bool computeValueAndGradient(
        const VecDataArray<double, 3>& currVertexPosition,
        double& c,
        std::vector<Vec3d>& dcdx) const override;

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