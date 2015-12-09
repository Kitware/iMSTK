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

#include <memory>
#include <iostream>
#include <assert.h>

// SimMedTK includes
#include "Core/Config.h"
#include "Assembler/Assembler.h"
#include "Solvers/SystemOfEquations.h"
#include "Solvers/SolverBase.h"
#include "Solvers/LinearSolver.h"

// vega includes
#include "sparseMatrix.h"

///
/// \brief Base class for iterative linear solvers
///
class IterativeLinearSolver : public LinearSolver
{
public:

    ///
    /// \brief default constructor
    ///
    IterativeLinearSolver(
        const std::shared_ptr<SparseLinearSystem> linSys=nullptr,
        const int epsilon = 1.0e-6,
        const int maxIterations = 100);

    ///
    /// \brief default constructor
    ///
    IterativeLinearSolver(const int epsilon = 1.0e-6, const int maxIterations = 100);

    ///
    /// \brief destructor
    ///
    ~IterativeLinearSolver();

    // -------------------------------------------------
    //  setters
    // -------------------------------------------------

    ///
	/// \brief set the tolerance for the iterative solver
	///
    void setTolerance(const double epsilon);

    ///
    /// \brief set the maximum number of iterations for the iterative solver
    ///
    void setMaximumIterations(const int maxIter);

    // -------------------------------------------------
    // getters
    // -------------------------------------------------

    ///
    /// \brief get the tolerance for the iterative solver
    ///
    int getTolerance() const;

    ///
    /// \brief get the maximum number of iterations for the iterative solver
    ///
    int getMaximumIterations() const;

protected:
    int maxIterations; ///> maximum limitation of interations to be performed

    double tolerance; ///> convergence tolerance
};

#endif // SM_ITERATIVE_LINEAR_SOLVER
