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
#ifndef SM_ITERATIVE_LINEAR_SOLVER
#define SM_ITERATIVE_LINEAR_SOLVER

// SimMedTK includes
#include "Solvers/LinearSolver.h"

///
/// \brief Base class for iterative linear solvers. Only for sparse matrices, extend if
///     other type of matrices are needed.
///
class IterativeLinearSolver : public LinearSolver<core::SparseMatrixd>
{
public:
    ///
    /// \brief Default constructor/destructor
    ///
    IterativeLinearSolver();
    ~IterativeLinearSolver() = default;

    ///
    /// \brief Do one iteration of the method
    ///
    virtual void iterate(core::Vectord &x, bool updateResidual = true) = 0;

    ///
    /// \brief Solve the linear system using Gauss-Seidel iterations
    ///
    virtual void solve(core::Vectord &x) override;

    // -------------------------------------------------
    //  Accessors
    // -------------------------------------------------

    ///
    /// \brief set the tolerance for the iterative solver
    ///
    virtual void setTolerance(const double epsilon);

    ///
    /// \brief get the tolerance for the iterative solver
    ///
    virtual double getTolerance() const;

    ///
    /// \brief set the maximum number of iterations for the iterative solver
    ///
    virtual void setMaximumIterations(const int maxIter);

    ///
    /// \brief get the maximum number of iterations for the iterative solver
    ///
    virtual int getMaximumIterations() const;

    ///
    /// \brief Get residual
    ///
    const core::Vectord &getResidual();

protected:
    int maxIterations;      ///> maximum number of iterations to be performed
    double minTolerance;    ///> convergence tolerance
    core::Vectord residual; ///> storage for residual vector
};

#endif // SM_ITERATIVE_LINEAR_SOLVER
