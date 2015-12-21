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
#ifndef SM_LINEAR_SOLVER
#define SM_LINEAR_SOLVER

// simmedtk includes
#include "Solvers/SystemOfEquations.h"
#include "Core/Matrix.h"

template<typename SystemMatrixType>
class LinearSystem;

///
/// \brief Base class for linear solvers
///
template<typename SystemMatrixType>
class LinearSolver
{
public:
    using MatrixType = SystemMatrixType;
    using LinearSystemType = LinearSystem<MatrixType>;

public:
    ///
    /// \brief Default constructor/destructor
    ///
    LinearSolver();
    virtual ~LinearSolver() = default;

    ///
    /// \brief Main solve routine
    ///
    virtual void solve(core::Vectord &x) = 0;

    ///
    /// \brief Set/get the system. Replaces/Returns the stored linear system of equations.
    ///
    /// \param newSystem New linear system of equations.
    ///
    virtual void setSystem(std::shared_ptr<LinearSystemType> newSystem);
    inline std::shared_ptr<LinearSystemType> getSystem() const;

    ///
    /// \brief Set/Get the tolerance for the linear solver.
    ///
    /// \param newTolerance New tolerance to be used.
    ///
    virtual void setTolerance(const double newTolerance);
    inline double getTolerance() const;

protected:
    std::shared_ptr<LinearSystemType> linearSystem; /// Linear system of equations.
    double minTolerance;    ///> Convergence tolerance
};

#include "Solvers/LinearSolver.hpp"

#endif // SM_LINEAR_SOLVER
