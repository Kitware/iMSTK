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

#include "imstkNonlinearSystem.h"

#include <g3log/g3log.hpp>

namespace imstk
{

NonLinearSystem::NonLinearSystem(const VectorFunctionType& F, const MatrixFunctionType& dF)
{
    m_F = F;
    m_dF = dF;
}

void
NonLinearSystem::setFunction(const VectorFunctionType& function)
{
    this->m_F = function;
}

void
NonLinearSystem::setJacobian(const MatrixFunctionType& function)
{
    this->m_dF = function;
}

const Vectord&
NonLinearSystem::evaluateF(const Vectord& x)
{
    return this->m_F(x);
}

const SparseMatrixd&
NonLinearSystem::evaluateJacobian(const Vectord& x)
{
    return this->m_dF(x);
}

} //imstk
