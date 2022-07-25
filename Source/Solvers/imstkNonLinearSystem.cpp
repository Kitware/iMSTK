/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkNonLinearSystem.h"
#include "imstkMath.h"

namespace imstk
{
template<typename Matrix>
NonLinearSystem<Matrix>::NonLinearSystem(const VectorFunctionType& F, const MatrixFunctionType& dF) : m_F(F), m_dF(dF)
{
    this->m_F_dF = [this](const Vectord& x, const bool semiImplicit)
                   {
                       return std::make_pair(&(this->m_F(x, semiImplicit)), &(this->m_dF(x)));
                   };
}

template<typename Matrix>
NonLinearSystem<Matrix>::NonLinearSystem(const VectorFunctionType& F, const MatrixFunctionType& dF, const VectorMatrixFunctionType& F_dF) : m_F(F), m_dF(dF), m_F_dF(F_dF)
{
}

template<typename Matrix>
void
NonLinearSystem<Matrix>::setFunction(const VectorFunctionType& function)
{
    m_F = function;
}

template<typename Matrix>
void
NonLinearSystem<Matrix>::setJacobian(const MatrixFunctionType& function)
{
    m_dF = function;
}

template<typename Matrix>
const Vectord&
NonLinearSystem<Matrix>::evaluateF(const Vectord& x, const bool isSemiImplicit)
{
    return m_F(x, isSemiImplicit);
}

template<typename Matrix>
const Matrix&
NonLinearSystem<Matrix>::evaluateJacobian(const Vectord& x)
{
    return m_dF(x);
}

template class NonLinearSystem<SparseMatrixd>;
template class NonLinearSystem<Matrixd>;
} // namespace imstk