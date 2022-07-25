/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkMath.h"
#include "imstkInternalForceModelTypes.h"

#ifdef WIN32
#pragma warning( push )
#pragma warning( disable : 4458 )
#pragma warning( disable : 4100 )
#endif
#include <forceModel.h>
#ifdef WIN32
#pragma warning( pop )
#endif

namespace imstk
{
///
/// \class InternalForceModel
///
/// \brief Base class for internal force model within the following context
///
///
class InternalForceModel
{
public:
    InternalForceModel() = default;
    virtual ~InternalForceModel() = default;

    ///
    /// \brief Compute internal force \p internalForce at state \p u
    ///
    virtual void getInternalForce(const Vectord& u, Vectord& internalForce) = 0;

    ///
    /// \brief Compute stiffness matrix \p tangentStiffnessMatrix at state \u
    ///
    virtual void getTangentStiffnessMatrix(const Vectord& u, SparseMatrixd& tangentStiffnessMatrix) = 0;

    ///
    /// \brief Build the sparsity pattern for stiffness matrix
    ///
    virtual void getTangentStiffnessMatrixTopology(vega::SparseMatrix** tangentStiffnessMatrix) = 0;

    ///
    /// \brief Compute both internal force \p internalForce and stiffness matrix \p tangentStiffnessMatrix at state \u
    /// \note This default implementation just call \p getInternalForce and \p getTangentStiffnessMatrix, but a more efficient implementation can be rewrite this.
    ///
    virtual void getForceAndMatrix(const Vectord& u, Vectord& internalForce, SparseMatrixd& tangentStiffnessMatrix);

    ///
    /// \brief Update the values of the Eigen sparse matrix given the linearized array of data from the Vega matrix
    ///
    static void updateValuesFromMatrix(std::shared_ptr<vega::SparseMatrix> vegaMatrix, double* values);

    ///
    /// \brief Specify tangent stiffness matrix
    ///
    virtual void setTangentStiffness(std::shared_ptr<vega::SparseMatrix> K) = 0;
};
} // namespace imstk