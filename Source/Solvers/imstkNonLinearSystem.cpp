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
} //imstk
