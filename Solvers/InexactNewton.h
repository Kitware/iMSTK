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

#ifndef INEXACT_NEWTON_H
#define INEXACT_NEWTON_H

#include "Solvers/NonLinearSolver.h"
#include "Solvers/LinearSolver.h"

///
/// @brief Inexact Newton method. This version of the newton method is based on the work
///     Tim Kelly and others at NC State University. The algorithm is globally convergent
///     in the sense that for any initial  iterate the iteration either converge to a
///     root of F or fails. Global convergence is achieved using a line search sub-process
///     and the Armijo rule.
///
class InexactNewton : public NonLinearSolver
{
public:
    using LinearSolverType = LinearSolver<core::SparseMatrixd>;

public:
    ///
    /// \brief Constructors/Destructors.
    ///
    InexactNewton();
    ~InexactNewton() = default;
    InexactNewton(const InexactNewton &other) = delete;

    InexactNewton &operator=(const InexactNewton &other) = delete;

    ///
    /// \brief Solve the non linear system of equations for a contant Jacobian matrix.
    ///
    /// \param x Current iterate.
    ///
    void solve(core::Vectord &x) override;

    ///
    /// \brief Update forcing term according to Eisenstat-Walker criteria.
    ///  \see http://softlib.rice.edu/pub/CRPC-TRs/reports/CRPC-TR94463.pdf.
    ///
    /// \param ratio Ratio of consecutive function norms.
    /// \param stopTolerance Linear combinarion of relative and absolute tolerances.
    /// \param fnorm Current function norm.
    ///
    void updateForcingTerm(const double ratio,
                           const double stopTolerance,
                           const double fnorm);

    ///
    /// \brief Set LinearSolver.
    ///
    /// \param newLinearSolver Linear solver pointer.
    ///
    void setLinearSolver(std::shared_ptr<LinearSolverType> newLinearSolver);

    ///
    /// \brief Get LinearSolver.
    ///
    std::shared_ptr<LinearSolverType> getLinearSolver() const;

    ///
    /// \brief Update jacobians.
    ///
    /// \param x Current iterate.
    ///
    void updateJacobian(const core::Vectord &x);

    ///
    /// \brief Set Jacobian. This sets the jacobian function, used to compute the jacobian matrix.
    ///
    /// \param newJacobian Jacobian function.
    ///
    void setJacobian(const JacobianType &newJacobian);

    ///
    /// \brief Get Jacobian. Returns the Jacobian function.
    ///
    const JacobianType &getJacobian() const;

    ///
    /// \brief Set JacobianMatrix.
    ///
    /// \param newJacobianMatrix Replaces the stored jacobian matrix.
    ///
    void setJacobianMatrix(const core::SparseMatrixd &newJacobianMatrix);

    ///
    /// \brief Get JacobianMatrix. Returns jacobian matrix.
    ///
    const core::SparseMatrixd &getJacobianMatrix() const;

    ///
    /// \brief Get JacobianMatrix. Returns jacobian matrix.
    ///
    core::SparseMatrixd &getJacobianMatrix();

    ///
    /// \brief Set AbsoluteTolerance.
    ///
    /// \param aTolerance New absolute tolerance value.
    ///
    void setAbsoluteTolerance(const double aTolerance);

    ///
    /// \brief Get AbsoluteTolerance. Returns current tolerance value.
    ///
    double getAbsoluteTolerance() const;

    ///
    /// \brief Set RelativeTolerance.
    ///
    /// \param newRelativeTolerance New relative tolerance value
    ///
    void setRelativeTolerance(const double newRelativeTolerance)
    {
        this->relativeTolerance = newRelativeTolerance;
    }

    ///
    /// \brief Get RelativeTolerance. Returns current relative tolerance value.
    ///
    double getRelativeTolerance() const
    {
        return this->relativeTolerance;
    }

    ///
    /// \brief Set Gamma. Sets the gamma parameter used to update the forcing term.
    ///
    /// \param newGamma New gamma value.
    ///
    void setGamma(const double newGamma)
    {
        this->gamma = newGamma;
    }

    ///
    /// \brief Get Gamma. Returns current gamma value.
    ///
    double getGamma() const
    {
        return this->gamma;
    }

    ///
    /// \brief Set EtaMax. Maximum error tolerance for residual in inner
    ///     iteration. The inner iteration terminates when the relative linear residual is
    ///     smaller than eta*| F(x_c) |. eta is determined by the modified
    ///     Eisenstat-Walker formula if etamax > 0. If etamax < 0, then eta = |etamax|
    ///     for the entire iteration.
    /// \see updateForcingTerm()
    ///
    void setEtaMax(const size_t newEtaMax)
    {
        this->etaMax = newEtaMax;
    }

    ///
    /// \brief Get EtaMax. Returns current etaMax value.
    ///
    size_t getEtaMax() const
    {
        return this->etaMax;
    }

    ///
    /// \brief Set MaxIterations. The maximum number of nonlinear iterations.
    ///
    void setMaxIterations(const size_t newMaxIterations)
    {
        this->maxIterations = newMaxIterations;
    }

    ///
    /// \brief Get MaxIterations. Returns current maximum nonlinear iterations.
    ///
    size_t getMaxIterations() const
    {
        return this->maxIterations;
    }

    ///
    /// \brief Set the useArmijo flag. If useArmijo is true a line search is performed
    ///     using the Armijo-Goldstein condition.
    ///
    void setUseArmijo(const bool value)
    {
        this->useArmijo = value;
    }

    ///
    /// \brief Get MaxIterations. Returns current maximum nonlinear iterations.
    ///
    bool getUseArmijo() const
    {
        return this->useArmijo;
    }

private:
    std::shared_ptr<LinearSolverType> linearSolver; ///> Linear solver to use. Default: Conjugate gradient.
    JacobianType jacobian;                          ///> Jacobian matrix function.
    double forcingTerm;                             ///> Method's forcing term (Default: 0.9).
    double absoluteTolerance;                       ///> Tolerance for the method (Default: 1.0e-3).
    double relativeTolerance;                       ///> Relative (to the rhs) tolerance (Default: 1.0e-6).
    double gamma;                                   ///> Internal parameter used to update the forcing term  (Default: 0.9).
    double etaMax;                                  ///> Maximum tolerance for the linear solver (Default: 0.9).
    size_t maxIterations;                           ///> Maximum number of nonlinear iterations (Default: 50).
    bool useArmijo;                                 ///> True if Armijo liner search is desired (Default: true).
};

#endif // INEXACT_NEWTON_H
