// This file is part of the iMSTK project.
//
// Copyright (c) Kitware, Inc.
//
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

#ifndef SOLVERS_BACKWARD_GAUSS_SEIDEL_H
#define SOLVERS_BACKWARD_GAUSS_SEIDEL_H

// iMSTK includes
#include "Solvers/IterativeLinearSolver.h"
#include "Core/Matrix.h"
#include "Core/Vector.h"

///
/// \brief Gauss Seidel sparse linear solver
///
class BackwardGaussSeidel : public IterativeLinearSolver
{
public:
    ///
    /// \brief Default constructor/destructor
    ///
    BackwardGaussSeidel() = default;
    ~BackwardGaussSeidel() = default;
    BackwardGaussSeidel(const BackwardGaussSeidel &) = delete;
    BackwardGaussSeidel &operator=(const BackwardGaussSeidel &) = delete;

    ///
    /// \brief Constructor
    ///
    /// \param A System matrix. Symmetric and positive definite.
    /// \param rhs Right hand side of the linear system of equations.
    ///
    BackwardGaussSeidel(const core::SparseMatrixd &A, const core::Vectord &rhs);

    ///
    /// \brief Do one iteration of the GaussSeidel method.
    ///
    /// \param x Current iterate.
    /// \param updateResidual Compute residual if true.
    ///
    void iterate(core::Vectord &x, bool updateResidual = true) override;

    ///
    /// \brief Does one iteration of the GaussSeidel method. This version of the method
    ///     traverses the sparse matrix container instead of relying on Eigen's template
    ///     expressions. This routine assumes that the sparse matrix is row-major.
    ///
    /// \param x Current iterate.
    ///
    void relax(core::Vectord &x);

    ///
    /// \brief Set the system. Replaces the stored linear system of equations.
    ///
    /// \param newSystem New linear system of equations.
    ///
    void setSystem(std::shared_ptr<LinearSystemType> newSystem) override;

};

#endif // BACKWARDGAUSSSEIDEL_H
