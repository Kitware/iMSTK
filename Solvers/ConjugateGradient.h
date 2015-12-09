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
#include "Core/Config.h"
#include "Assembler/Assembler.h"
#include "Solvers/IterativeLinearSolver.h"
#include "Solvers/SystemOfEquations.h"

// vega includes
#include "CGSolver.h"

///
/// \brief Conjugate gradient sparse linear solvers
/// \todo write unit tests
///
class ConjugateGradient : public IterativeLinearSolver
{
public:

    ///
    /// \brief default constructor
    ///
    ConjugateGradient(
        const std::shared_ptr<SparseLinearSystem> linSys = nullptr,
        const int epsilon = 1.0e-6,
        const int maxIterations = 100);

    ///
    /// \brief default constructor
    ///
    ConjugateGradient(const int epsilon = 1.0e-6, const int maxIterations = 100);

    ///
    /// \brief destructor
    ///
    ~ConjugateGradient();

	///
	/// \brief Solve the linear system using Conjugate gradient iterations
	///
    void Solve() override;

    //@{

    ///
	/// \brief Solve using the conjugate gradient iterations
    /// Utility function to be called without an object of this class
	///
    static void conjugateGradientSolve(
        Eigen::SparseMatrix<double>& A,
        Eigen::VectorXd& x,
        Eigen::VectorXd& b,
        const int maxIter = 100,
        const double epsilon = 1.0e-6);


    static void conjugateGradientSolve(
        std::shared_ptr<LinearSystem> linearSys,
        const int maxIter /*= 100*/,
        const double epsilon /*= 1.0e-6*/);

    ststic void conjugateGradientSolve(
        Eigen::Matrix<double>& A,
        Eigen::VectorXd& x,
        Eigen::VectorXd& b,
        const int maxIter /*= 100*/,
        const double epsilon /*= 1.0e-6*/);

    static void conjugateGradientIterations(
        Eigen::SparseMatrix<double>& A,
        Eigen::VectorXd& x,
        Eigen::VectorXd& b,
        const int numIter);

    static void conjugateGradientSolve(
        std::shared_ptr <SparseMatrix> A,
        double* x,
        double* b,
        const int maxIter = 100,
        const double epsilon = 1.0e-6);

    static void conjugateGradientIterations(
        std::shared_ptr <SparseMatrix> A,
        double* x,
        double* b,
        const int numIter);
    //@}

private:
    // pointer to the Eigen's Conjugate gradient solver
    std::shared_ptr<Eigen::ConjugateGradient<Eigen::SparseMatrix<double>>> eigenCgSolver;

    // pointer to the vega's Conjugate gradient solver
    std::shared_ptr<CGSolver> vegaCgSolver;
};

#endif // SM_SPARSE_CG
