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


#include "Solvers/GaussSeidelSolver.h"

GaussSeidelSolver::GaussSeidelSolver(
    const int epsilon,
    const int maxIterations)
    : IterativeLinearSolver(epsilon, maxIterations)
{
}

GaussSeidelSolver::GaussSeidelSolver(
    const std::shared_ptr<SparseLinearSystem> linSys /*= nullptr*/,
    const int epsilon /*= 1.0e-6*/,
    const int maxIterations /*= 100*/)
    : IterativeLinearSolver(linSys, epsilon, maxIterations)
{
}

GaussSeidelSolver::~GaussSeidelSolver()
{
}

void GaussSeidelSolver::Solve()
{
    if (this->sysOfEquations != nullptr)
    {
        auto linearSys =
            std::static_pointer_cast<SparseLinearSystem>(this->sysOfEquations);

        GaussSeidelSolve(
            linearSys->getMatrix(),
            linearSys->getUnknownVector(),
            linearSys->getForceVector());
    }
    else
    {
        std::cout << "Error: Linear system not set to the Gauss Seidel solver!\n";
    }
}

void GaussSeidelSolver::GaussSeidelSolve(
    Eigen::SparseMatrix<double>& A,
    Eigen::VectorXd& x,
    Eigen::VectorXd& b,
    const int maxIter /*= 100*/,
    const double epsilon /*= 1.0e-6*/)
{
    /*this->eigenGsSolver->setMaxIterations(numIter);
    this->eigenCgSolver->setTolerance(epsilon);
    eigenCgSolver.compute(A);
    x = eigenCgSolver.solve(b);*/

}

void GaussSeidelSolver::GaussSeidelIteration(
    Eigen::SparseMatrix<double>& A,
    Eigen::VectorXd& x,
    Eigen::VectorXd& b,
    const int numIter)
{
    /* this->eigenCgSolver->setMaxIterations(this->maxIterations);
    this->eigenCgSolver->setTolerance(this->tolerance);
    eigenCgSolver.compute(A);
    x = eigenCgSolver.solve(b);*/
}

void GaussSeidelSolver::GaussSeidelSolve(
    std::shared_ptr <SparseMatrix> A,
    double* x,
    double* b,
    const int maxIter /*= 100*/,
    const double epsilon /*= 1.0e-6*/)
{
    int i, numIter = 0;
    double res;

    std::vector<double> r;
    int size = A->GetNumColumns();
    r.resize(size);

    do{
        A->DoOneGaussSeidelIteration(x, b);

        A->ComputeResidual(x, b, r.data());

        res = 0.0;
        for (i = 0; i < size; i++)
        {
            res += r[i] * r[i];
        }

        if (res < epsilon*epsilon)
        {
            break;
        }

        numIter++;

    } while (numIter < maxIter);
}

void GaussSeidelSolver::GaussSeidelIteration(
    std::shared_ptr <SparseMatrix> A,
    double* x,
    double* b,
    const int numIter)
{
    for (int i = 0; i < numIter; i++)
    {
        A->DoOneGaussSeidelIteration(x, b);
    }
}