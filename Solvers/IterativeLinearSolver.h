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
/// \brief Base class for iterative linear solvers.
///
class IterativeLinearSolver : public LinearSolver<core::SparseMatrixd>
{
public:
    ///
    /// \brief Default constructor/destructor
    ///
    IterativeLinearSolver();
    virtual ~IterativeLinearSolver() = default;

    ///
    /// \brief Do one iteration of the method.
    ///
    /// \param x Current iterate.
    /// \param updateResidual Compute residual if true.
    ///
    virtual void iterate(core::Vectord &x, bool updateResidual = true) = 0;

    ///
    /// \brief Solve the linear system using Gauss-Seidel iterations.
    ///
    /// \param x Current iterate.
    ///
    virtual void solve(core::Vectord &x) override;

    // -------------------------------------------------
    //  Accessors
    // -------------------------------------------------

    ///
    /// \brief set the maximum number of iterations for the iterative solver.
    ///
    /// \param maxIter Maximum number of iterations.
    ///
    virtual void setMaximumIterations(const size_t maxIter);

    ///
    /// \brief get the maximum number of iterations for the iterative solver.
    ///
    virtual size_t getMaximumIterations() const;

    ///
    /// \brief Return residual vector. This fuction does not do any computation.
    ///
    virtual const core::Vectord &getResidual();

    ///
    /// \brief Return error computed from the residual.
    ///
    virtual double getError(const core::Vectord &x);

    ///
    /// \brief Print solver information.
    ///
    virtual void print();

protected:
    size_t maxIterations;   ///> Maximum number of iterations to be performed.
    core::Vectord residual; ///> Storage for residual vector.
};

#endif // SM_ITERATIVE_LINEAR_SOLVER
