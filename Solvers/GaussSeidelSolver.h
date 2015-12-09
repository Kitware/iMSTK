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
#ifndef SM_SPARSE_GAUSS_SEIDEL
#define SM_SPARSE_GAUSS_SEIDEL

#include <memory>
#include <iostream>

// SimMedTK includes
#include "Core/Config.h"
#include "Assembler/Assembler.h"
#include "Solvers/IterativeLinearSolver.h"
#include "Solvers/SystemOfEquations.h"

///
/// \brief Gauss Seidel sparse linear solver
/// \todo write unit tests
///
class GaussSeidelSolver : public IterativeLinearSolver
{
public:

    ///
    /// \brief default constructor
    ///
    GaussSeidelSolver(
        const std::shared_ptr<SparseLinearSystem> linSys = nullptr,
        const int epsilon = 1.0e-6,
        const int maxIterations = 100);

    ///
    /// \brief default constructor
    ///
    GaussSeidelSolver(const int epsilon = 1.0e-6, const int maxIterations = 100);

    ///
    /// \brief destructor
    ///
    ~GaussSeidelSolver();

	///
	/// \brief Solve the linear system using Gauss-Seidel iterations
	///
    void Solve() override;

    //@{

    ///
	/// \brief Solve using the Gauss-Seidel iterations
    /// Utility function to be called without an object of this class
    /// \todo optimize the residual computation
	///
    static void GaussSeidelSolve(
        Eigen::SparseMatrix<double>& A,
        Eigen::VectorXd& x,
        Eigen::VectorXd& b,
        const int maxIter = 100,
        const double epsilon = 1.0e-6);

    static void GaussSeidelIteration(
        Eigen::SparseMatrix<double>& A,
        Eigen::VectorXd& x,
        Eigen::VectorXd& b,
        const int numIter);

    static void GaussSeidelSolve(
        std::shared_ptr <SparseMatrix> A,
        double* x,
        double* b,
        const int maxIter = 100,
        const double epsilon = 1.0e-6);

    static void GaussSeidelIteration(
        std::shared_ptr <SparseMatrix> A,
        double* x,
        double* b,
        const int numIter);

    //@}

private:
};

#endif // SM_SPARSE_GAUSS_SEIDEL