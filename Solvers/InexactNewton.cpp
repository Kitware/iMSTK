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

#include "InexactNewton.h"

#include "Solvers/IterativeLinearSolver.h"
#include "Solvers/ConjugateGradient.h"
#include <iostream>

InexactNewton::InexactNewton():
    linearSolver(std::make_shared<ConjugateGradient>()),
    forcingTerm(0.9),
    absoluteTolerance(1e-3),
    relativeTolerance(1e-6),
    gamma(0.9),
    etaMax(0.9),
    maxIterations(50) {}

//---------------------------------------------------------------------------
void InexactNewton::solve(core::Vectord &x)
{
    if(!this->linearSolver)
    {
        return;
    }

    // Set tolerances and other temporaries
    this->f.resize(x.size());
    this->nonLinearSystem->eval(x, this->f);
    double fnorm = this->f.norm();
    double stopTolerance = this->absoluteTolerance + this->relativeTolerance * fnorm;
    this->linearSolver->setTolerance(stopTolerance);
    core::Vectord dx = x;

    for(size_t i = 0; i < this->maxIterations; ++i)
    {
        if(fnorm < stopTolerance)
        {
            return;
        }

        this->updateJacobian(x);
        this->linearSolver->solve(dx);
        x -= dx;

        double newNorm = this->armijo(dx, x);

        if(this->forcingTerm > 0 && fnorm > 0)
        {
            double ratio = newNorm / fnorm; // Ratio of succesive residual norms
            this->updateForcingTerm(ratio, stopTolerance, fnorm);

            // Reset tolerance in the linear solver according to the new forcing term
            if(this->linearSolver)
            {
                this->linearSolver->setTolerance(this->forcingTerm);
            }
        }

        fnorm = newNorm;
    }
}

//---------------------------------------------------------------------------
void InexactNewton::updateForcingTerm(const double ratio,
                                      const double stopTolerance,
                                      const double fnorm)
{
    double eta = this->gamma * ratio * ratio;
    double forcingTermSqr = this->forcingTerm * this->forcingTerm;

    // Saveguard to prevent the forcing term to become too small for far away iterates
    if(this->gamma * forcingTermSqr > .1)
    {
        eta = std::max(eta, this->gamma * forcingTermSqr);
    }

    this->forcingTerm = std::max(std::min(eta, this->etaMax), 0.5 * stopTolerance / fnorm);
}

//---------------------------------------------------------------------------
void InexactNewton::setLinearSolver(std::shared_ptr< InexactNewton::LinearSolverType > newLinearSolver)
{
    this->linearSolver = newLinearSolver;
}

//---------------------------------------------------------------------------
std::shared_ptr< InexactNewton::LinearSolverType > InexactNewton::getLinearSolver() const
{
    return this->linearSolver;
}

//---------------------------------------------------------------------------
void InexactNewton::updateJacobian(const core::Vectord &x)
{
    // Evaluate the Jacobian and sets the matrix
    if(this->jacobian)
    {
        this->jacobianMatrix.resize(x.size(), x.size());
        this->jacobian(x, this->jacobianMatrix);
    }

    if(this->jacobianMatrix.innerSize() > 0)
    {
        auto linearSystem = std::make_shared<LinearSolverType::LinearSystemType>(
                                this->jacobianMatrix, this->f);
        this->linearSolver->setSystem(linearSystem);
    }
    else
    {
        std::cerr << "Error updating Jacobian" << std::endl;
    }
}

//---------------------------------------------------------------------------
void InexactNewton::setJacobian(const NonLinearSolver::JacobianType &newJacobian)
{
    this->jacobian = newJacobian;
}

//---------------------------------------------------------------------------
const NonLinearSolver::JacobianType &InexactNewton::getJacobian() const
{
    return this->jacobian;
}

//---------------------------------------------------------------------------
void InexactNewton::setJacobianMatrix(const core::SparseMatrixd &newJacobianMatrix)
{
    this->jacobianMatrix = newJacobianMatrix;
}

//---------------------------------------------------------------------------
const core::SparseMatrixd &InexactNewton::getJacobianMatrix() const
{
    return this->jacobianMatrix;
}

//---------------------------------------------------------------------------
core::SparseMatrixd &InexactNewton::getJacobianMatrix()
{
    return this->jacobianMatrix;
}

//---------------------------------------------------------------------------
void InexactNewton::setAbsoluteTolerance(const double aTolerance)
{
    this->absoluteTolerance = aTolerance;
}

//---------------------------------------------------------------------------
double InexactNewton::getAbsoluteTolerance() const
{
    return this->absoluteTolerance;
}
