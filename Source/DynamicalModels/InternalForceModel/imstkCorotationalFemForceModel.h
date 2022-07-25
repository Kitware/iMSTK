/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkInternalForceModel.h"

namespace vega
{
class VolumetricMesh;
class CorotationalLinearFEM;
class SparseMatrix;
} // namespace vega

namespace imstk
{
///
/// \class CorotationalFemForceModel
///
/// \brief Force model for corotational based finite elements formulation
///
class CorotationalFemForceModel : public InternalForceModel
{
public:
    ///
    /// \brief Constructor using \p mesh
    /// \param warp if use warp
    ///
    CorotationalFemForceModel(std::shared_ptr<vega::VolumetricMesh> mesh, const int warp = 1);
    CorotationalFemForceModel() = delete;
    ~CorotationalFemForceModel() override = default;

    ///
    /// \brief Compute internal force \p internalForce at state \p u
    ///
    void getInternalForce(const Vectord& u, Vectord& internalForce) override;

    ///
    /// \brief Compute stiffness matrix \p tangentStiffnessMatrix at state \u
    ///
    void getTangentStiffnessMatrix(const Vectord& u, SparseMatrixd& tangentStiffnessMatrix) override;

    ///
    /// \brief Build the sparsity pattern for stiffness matrix
    ///
    void getTangentStiffnessMatrixTopology(vega::SparseMatrix** tangentStiffnessMatrix) override;

    ///
    /// \brief Compute internal force \p internalForce and stiffness matrix \p tangentStiffnessMatrix at state \u
    ///
    void getForceAndMatrix(const Vectord& u, Vectord& internalForce, SparseMatrixd& tangentStiffnessMatrix) override;

    ///
    /// \brief Turn on/off warp
    ///
    void setWarp(const int warp);

    ///
    /// \brief Specify tangent stiffness matrix
    ///
    void setTangentStiffness(std::shared_ptr<vega::SparseMatrix> K) override;

protected:
    std::shared_ptr<vega::CorotationalLinearFEM> m_corotationalLinearFem;
    std::shared_ptr<vega::SparseMatrix> m_vegaTangentStiffnessMatrix;
    int m_warp;
};
} // namespace imstk