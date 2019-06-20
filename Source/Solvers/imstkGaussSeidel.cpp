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

#include "imstkGaussSeidel.h"

namespace imstk
{
GaussSeidel::GaussSeidel(const SparseMatrixd& A, const Vectord& rhs) : GaussSeidel()
{
    this->setSystem(std::make_shared<LinearSystem<SparseMatrixd>>(A, rhs));
}

void
GaussSeidel::solve(Vectord& x)
{
    if (!m_linearSystem)
    {
        LOG(WARNING) << "Gauss-Seidel::solve: Linear system is not supplied for Gauss-Seidel solver!";
        return;
    }

    if (!m_FixedLinearProjConstraints)
    {
        this->gaussSeidelSolve(x);
    }
    else
    {
        // Do nothing for now!
    }
}

void
GaussSeidel::solve(Vectord& x, const double tolerance)
{
    this->setTolerance(tolerance);
    this->solve(x);
}

void
GaussSeidel::gaussSeidelSolve(Vectord& x)
{
    const auto &b = m_linearSystem->getRHSVector();
    const auto &A = m_linearSystem->getMatrix();

    // Set the initial guess to zero
    x.setZero();

    auto xOld = x;
    size_t iterNum = 0;
    while (iterNum < this->getMaxNumIterations())
    {
        for (auto k = 0; k < A.outerSize(); ++k)
        {
            double diagEle = 0.;
            double aggregate = 0.;
            for (SparseMatrixd::InnerIterator it(A, k); it; ++it)
            {
                auto col = it.col();
                if (k != col)
                {
                    aggregate += it.value() * x[col];
                }
                else
                {
                    diagEle = it.value();
                }
            }
            x[k] = (b[k] - aggregate) / diagEle; // div by zero is possible!
        }

        if ((x - xOld).norm() < 1.0e-4)
        {
            return;
        }
        xOld = x;
        iterNum++;
    }
}

double
GaussSeidel::getResidual(const Vectord& )
{
    return 0;
}

void
GaussSeidel::setTolerance(const double epsilon)
{
    IterativeLinearSolver::setTolerance(epsilon);
}

void
GaussSeidel::setMaxNumIterations(const size_t maxIter)
{
    IterativeLinearSolver::setMaxNumIterations(maxIter);
}

void
GaussSeidel::setSystem(std::shared_ptr<LinearSystem<SparseMatrixd>> newSystem)
{
    LinearSolver<SparseMatrixd>::setSystem(newSystem);
}

void
GaussSeidel::print() const
{
    IterativeLinearSolver::print();

    LOG(INFO) << "Solver: Gauss-Seidel";
    LOG(INFO) << "Tolerance: " << m_tolerance;
    LOG(INFO) << "max. iterations: " << m_maxIterations;
}
} // imstk