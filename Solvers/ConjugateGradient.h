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
#ifndef SM_SPARSE_CG
#define SM_SPARSE_CG

#include <memory>

// SimMedTK includes
#include "Solvers/IterativeLinearSolver.h"
#include "Solvers/SystemOfEquations.h"

// Eigen includes
#include <Eigen/IterativeLinearSolvers>

///
/// \brief Conjugate gradient sparse linear solver for SPD matrices
///
class ConjugateGradient : public IterativeLinearSolver
{
public:
    ///
    /// \brief default constructor
    ///
    ConjugateGradient() = delete;
    ConjugateGradient(const core::SparseMatrixd &A, const core::Vectord &rhs);

    ///
    /// \brief destructor
    ///
    ~ConjugateGradient() = default;

    ///
    /// \brief Do one iteration of the method
    ///
    void iterate(core::Vectord &x, bool updateResidual = true) override;

    ///
    /// \brief Solve the linear system using Conjugate gradient iterations
    ///
    void solve(core::Vectord &x) override;

    ///
    /// \brief Solve the linear system using Conjugate gradient iterations to a
    ///     specified tolerance
    ///
    void solve(core::Vectord &x, double tolerance);

    ///
    /// Return the error calculated by the solver
    ///
    double getError() const;

    ///
    /// \brief Set the tolerance for the iterative solver
    ///
    void setTolerance(const double epsilon)
    {
        this->minTolerance = epsilon;
        this->solver.setTolerance(epsilon);
    }

    ///
    /// \brief Set the maximum number of iterations for the iterative solver
    ///
    void setMaximumIterations(const int maxIter)
    {
        this->maxIterations = maxIter;
        this->solver.setMaxIterations(maxIter);
    }

private:
    // pointer to the Eigen's Conjugate gradient solver
    Eigen::ConjugateGradient<core::SparseMatrixd> solver;
};

#endif // SM_SPARSE_CG
