// This file is part of the SimMedTK project.
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//---------------------------------------------------------------------------

#include "Solvers/ConjugateGradient.h"

ConjugateGradient::ConjugateGradient(
    const int epsilon /*= 1.0e-6*/,
    const int maxIterations /*= 100*/) :
    IterativeLinearSolver(nullptr, epsilon, maxIterations)
{
}

ConjugateGradient::ConjugateGradient(
    const std::shared_ptr<SparseLinearSystem> linSys /*= nullptr*/,
    const int epsilon /*= 1.0e-6*/,
    const int maxIterations /*= 100*/) :
    IterativeLinearSolver(linSys, epsilon, maxIterations)
{
}

ConjugateGradient::~ConjugateGradient()
{
}

void ConjugateGradient::Solve()
{
    if (this->sysOfEquations != nullptr)
    {
        auto linearSysSparse =
            std::dynamic_pointer_cast<SparseLinearSystem>(this->sysOfEquations);

        if (linearSysSparse != nullptr)
        {
            conjugateGradientSolve(
                linearSysSparse->getMatrix(),
                linearSysSparse->getUnknownVector(),
                linearSysSparse->getForceVector());

            return;
        }
        else
        {
            auto linearSysDense =
                std::dynamic_pointer_cast<DenseLinearSystem>(this->sysOfEquations);

            conjugateGradientSolve(
                linearSysDense->getMatrix(),
                linearSysDense->getUnknownVector(),
                linearSysDense->getForceVector());

            return;

        }
    }
    else
    {
        std::cout << "Error: Linear system not set to the CG solver!\n";
    }
}

void ConjugateGradient::conjugateGradientSolve(
    std::shared_ptr<LinearSystem> linearSys,
    const int maxIter /*= 100*/,
    const double epsilon /*= 1.0e-6*/)
{

    if (linearSys != nullptr)
    {
        auto linearSysSparse =
            std::dynamic_pointer_cast<SparseLinearSystem>(linearSys);
        if (linearSysSparse != nullptr)
        {
            conjugateGradientSolve(
                linearSysSparse->getMatrix(),
                linearSysSparse->getUnknownVector(),
                linearSysSparse->getForceVector());
            return;
        }

        auto linearSysDense =
            std::dynamic_pointer_cast<DenseLinearSystem>(linearSys);
        if (linearSysDense)
        {
            conjugateGradientSolve(
                linearSysDense->getMatrix(),
                linearSysDense->getUnknownVector(),
                linearSysDense->getForceVector());
            return;
        }

        std::cout << "Error: The linear system points to a base class\n";
        return;
    }
    else
    {
        std::cout << "Error: The linear system is non-existent\n";
    }
}

void ConjugateGradient::conjugateGradientSolve(
    Eigen::MatrixXd& A,
    Eigen::VectorXd& x,
    Eigen::VectorXd& b,
    const int maxIter /*= 100*/,
    const double epsilon /*= 1.0e-6*/)
{

}

void ConjugateGradient::conjugateGradientSolve(
    Eigen::SparseMatrix<double>& A,
    Eigen::VectorXd& x,
    Eigen::VectorXd& b,
    const int maxIter /*= 100*/,
    const double epsilon /*= 1.0e-6*/)
{
    auto eigenCgSolver = std::make_shared<
        Eigen::ConjugateGradient<Eigen::SparseMatrix<double>>>(A) ;

    eigenCgSolver->setMaxIterations(maxIter);
    eigenCgSolver->setTolerance(epsilon);
    x = eigenCgSolver->solve(b);
}

void ConjugateGradient::conjugateGradientIterations(
    Eigen::SparseMatrix<double>& A,
    Eigen::VectorXd& x,
    Eigen::VectorXd& b,
    const int numIter)
{

}

void ConjugateGradient::conjugateGradientSolve(
    std::shared_ptr <SparseMatrix> A,
    double* x,
    double* b,
    const int maxIter /*= 100*/,
    const double epsilon /*= 1.0e-6*/)
{
    auto cgSolver = std::make_shared<CGSolver>(A.get());
    cgSolver->setEpsilon(epsilon);
    cgSolver->setNumberOfIterations(maxIter);
    cgSolver->SolveLinearSystem(x, b);
}

void ConjugateGradient::conjugateGradientIterations(
    std::shared_ptr <SparseMatrix> A,
    double* x,
    double* b,
    const int numIter)
{

}