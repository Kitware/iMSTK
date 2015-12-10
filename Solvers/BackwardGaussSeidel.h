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

#ifndef BACKWARD_GAUSS_SEIDEL_H
#define BACKWARD_GAUSS_SEIDEL_H

// SimMedTK includes
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
    BackwardGaussSeidel() = delete;
    BackwardGaussSeidel(const BackwardGaussSeidel &) = delete;
    BackwardGaussSeidel &operator=(const BackwardGaussSeidel &) = delete;

    BackwardGaussSeidel(const core::SparseMatrixd &A, const core::Vectord &rhs);

    ~BackwardGaussSeidel() = default;

    ///
    /// \brief Do one iteration of the method
    ///
    void iterate(core::Vectord &x, bool updateResidual = true) override;

private:
    Eigen::SparseTriangularView<core::SparseMatrixd, Eigen::Upper> U;
    Eigen::SparseTriangularView<core::SparseMatrixd, Eigen::StrictlyLower> L;
};

#endif // BACKWARDGAUSSSEIDEL_H
