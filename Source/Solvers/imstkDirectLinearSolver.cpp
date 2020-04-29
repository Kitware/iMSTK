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

#include "imstkDirectLinearSolver.h"
#include "imstkLinearSystem.h"
#include "imstkLogger.h"
#include <iostream>

namespace imstk
{
DirectLinearSolver<Matrixd>::
DirectLinearSolver(const Matrixd& matrix, const Vectord& b)
{
    m_linearSystem = std::make_shared<LinearSystem<Matrixd>>(matrix, b);
    m_solver.compute(matrix);
}

void
DirectLinearSolver<Matrixd>::
setSystem(std::shared_ptr<LinearSystem<Matrixd>> newSystem)
{
    LinearSolver<Matrixd>::setSystem(newSystem);
    m_solver.compute(m_linearSystem->getMatrix());
}

DirectLinearSolver<SparseMatrixd>::
DirectLinearSolver(const SparseMatrixd& matrix, const Vectord& b)
{
    m_linearSystem = std::make_shared<LinearSystem<SparseMatrixd>>(matrix, b);
    m_solver.compute(matrix);
}

void
DirectLinearSolver<SparseMatrixd>::
setSystem(std::shared_ptr<LinearSystem<SparseMatrixd>> newSystem)
{
    LinearSolver<SparseMatrixd>::setSystem(newSystem);
    m_solver.compute(m_linearSystem->getMatrix());
}

void
DirectLinearSolver<SparseMatrixd>::solve(const Vectord& rhs, Vectord& x)
{
    if(!m_linearSystem) LOG(FATAL) << "Linear system has not been set";
    x = m_solver.solve(rhs);
}

void
DirectLinearSolver<SparseMatrixd>::solve(Vectord& x)
{
    if(!m_linearSystem) LOG(FATAL) << "Linear system has not been set";
    x.setZero();

    auto b = m_linearSystem->getRHSVector();
    x = m_solver.solve(b);
}

void
DirectLinearSolver<Matrixd>::solve(const Vectord& rhs, Vectord& x)
{
    if(!m_linearSystem) LOG(FATAL) << "Linear system has not been set";
    x = m_solver.solve(rhs);
}

void
DirectLinearSolver<Matrixd>::solve(Vectord& x)
{
    if(!m_system_set) LOG(FATAL) << "Linear system has not been set";
    x.setZero();

    auto b = m_linearSystem->getRHSVector();
    x = m_solver.solve(b);
}
} // imstk
