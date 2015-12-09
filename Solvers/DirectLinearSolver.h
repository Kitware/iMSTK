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
#ifndef SM_DIRECT_LINEAR_SOLVER
#define SM_DIRECT_LINEAR_SOLVER

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
class DirectLinearSolver : public LinearSolver
{
public:

    ///
    /// \brief default constructor
    ///
    DirectLinearSolver(const std::shared_ptr<LinearSystem> linSys = nullptr);

    ///
    /// \brief destructor
    ///
    ~DirectLinearSolver();

    ///
    /// \brief Get the unknown vector
    ///
    virtual Eigen::Vector<double>& getUnknownVector() override;

    ///
    /// \brief Get the force vector on the r.h.s
    ///
    virtual Eigen::Vector<double>& getForceVector() override;

    ///
    /// \brief Solve the system of equations
    ///
    virtual void Solve() override;

protected:
};

#endif // SM_DIRECT_LINEAR_SOLVER
