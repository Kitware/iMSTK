/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkInternalForceModel.h"

#include <StVKStiffnessMatrix.h>

namespace vega
{
class StvkInternalForces;
class VolumetricMesh;
} // namespace vega

namespace imstk
{
///
/// \class StvkForceModel
///
/// \brief
///
class StvkForceModel : public InternalForceModel
{
public:
    StvkForceModel(std::shared_ptr<vega::VolumetricMesh> mesh,
                   const bool withGravity = true, const double gravity = 10.0);
    ~StvkForceModel() override = default;

    ///
    /// \brief Get the internal force
    ///
    inline void getInternalForce(const Vectord& u, Vectord& internalForce) override
    {
        double* data = const_cast<double*>(u.data());
        m_stVKInternalForces->ComputeForces(data, internalForce.data());
    }

    ///
    /// \brief Get the tangent stiffness matrix topology
    ///
    inline void getTangentStiffnessMatrixTopology(vega::SparseMatrix** tangentStiffnessMatrix) override
    {
        m_vegaStVKStiffnessMatrix->GetStiffnessMatrixTopology(tangentStiffnessMatrix);
    }

    ///
    /// \brief Set the tangent stiffness matrix
    ///
    inline void getTangentStiffnessMatrix(const Vectord& u, SparseMatrixd& tangentStiffnessMatrix) override
    {
        double* data = const_cast<double*>(u.data());
        m_vegaStVKStiffnessMatrix->ComputeStiffnessMatrix(data, m_vegaTangentStiffnessMatrix.get());
        InternalForceModel::updateValuesFromMatrix(m_vegaTangentStiffnessMatrix, tangentStiffnessMatrix.valuePtr());
    }

    ///
    /// \brief Speficy tangent stiffness matrix
    ///
    inline void setTangentStiffness(std::shared_ptr<vega::SparseMatrix> K) override
    {
        m_vegaTangentStiffnessMatrix = K;
    }

protected:
    std::shared_ptr<vega::StVKInternalForces>  m_stVKInternalForces;
    std::shared_ptr<vega::SparseMatrix>        m_vegaTangentStiffnessMatrix;
    std::shared_ptr<vega::StVKStiffnessMatrix> m_vegaStVKStiffnessMatrix;
    bool ownStiffnessMatrix;
};
} // namespace imstk