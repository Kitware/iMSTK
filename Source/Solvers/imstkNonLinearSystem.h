/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkMath.h"
#include <utility>

namespace imstk
{
///
/// \class NonLinearSystem
///
/// \brief Base class for a multi-variable nonlinear system
///
template<typename Matrix>
class NonLinearSystem
{
public:
    using VectorFunctionType = std::function<const Vectord& (const Vectord&, const bool)>;
    using MatrixFunctionType = std::function<const Matrix& (const Vectord&)>;
    // using VecMatPair = std::pair<const Vectord&, const Matrix&>;
    using VecMatPair = std::pair<const Vectord*, const Matrix*>;
    using VectorMatrixFunctionType    = std::function<VecMatPair(const Vectord&, const bool)>;
    using UpdateFunctionType          = std::function<void (const Vectord&, const bool)>;
    using UpdatePrevStateFunctionType = std::function<void ()>;

    NonLinearSystem() { }
    NonLinearSystem(const VectorFunctionType& F, const MatrixFunctionType& dF);
    NonLinearSystem(const VectorFunctionType& F, const MatrixFunctionType& dF, const VectorMatrixFunctionType& F_dF);

    virtual ~NonLinearSystem() { }

    ///
    /// \brief Set nonlinear method that evaluates the nonlinear function.
    ///
    virtual void setFunction(const VectorFunctionType& function);

    ///
    /// \brief Set the method that evaluates the gradient of the nonlinear function
    ///
    virtual void setJacobian(const MatrixFunctionType& function);

    ///
    /// \brief Evaluate function at a given state
    ///
    virtual const Vectord& evaluateF(const Vectord& x, const bool isSemiImplicit);

    ///
    /// \brief Evaluate gradient of the function at a given state
    ///
    virtual const Matrix& evaluateJacobian(const Vectord& x);

    ///
    /// \brief
    ///
    void setUnknownVector(Vectord& v)
    {
        m_unknown = &v;
    }

    ///
    /// \brief Get the vector used to populate the solution
    ///
    Vectord& getUnknownVector()
    {
        return *m_unknown;
    }

    /// \brief Get the vector denoting the filter
    ///
    /*void setLinearProjectors(std::vector<LinearProjectionConstraint>& f)
    {
        m_LinearProjConstraints = &f;
    }

    /// \brief Get the vector denoting the filter
    ///
    std::vector<LinearProjectionConstraint>& getLinearProjectors()
    {
        return *m_LinearProjConstraints;
    }*/

    ///
    /// \brief Set the update function
    ///
    void setUpdateFunction(const UpdateFunctionType& updateFunc)
    {
        m_FUpdate = updateFunc;
    }

    ///
    /// \brief Set the update function
    ///
    void setUpdatePreviousStatesFunction(const UpdatePrevStateFunctionType& updateFunc)
    {
        m_FUpdatePrevState = updateFunc;
    }

/// \brief Get the vector denoting the filter
///
/*void setDynamicLinearProjectors(std::vector<LinearProjectionConstraint>* f)
    {
        m_DynamicLinearProjConstraints = f;
    }

    /// \brief Get the vector denoting the filter
    ///
    std::vector<LinearProjectionConstraint>& getDynamicLinearProjectors()
    {
        return *m_DynamicLinearProjConstraints;
    }*/

    VectorFunctionType       m_F;  ///< Nonlinear function
    MatrixFunctionType       m_dF; ///< Gradient of the Nonlinear function with respect to the unknown vector
    VectorMatrixFunctionType m_F_dF;
    Vectord* m_unknown = nullptr;

    UpdateFunctionType m_FUpdate;
    UpdatePrevStateFunctionType m_FUpdatePrevState;
    /*std::vector<LinearProjectionConstraint>  *m_LinearProjConstraints;
    std::vector<LinearProjectionConstraint>  *m_DynamicLinearProjConstraints;*/
};
} // namespace imstk