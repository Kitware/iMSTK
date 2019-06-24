/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

=========================================================================*/

#pragma once

#include "imstkNonLinearSolver.h"
#include "imstkLinearSolver.h"

namespace imstk
{
///
/// \brief Newton method. This version of the newton method is based on the work
///     by Tim Kelly and others at NC State University. The algorithm is globally convergent
///     in the sense that for any initial iterate the iteration either converge to a
///     root of F or fails. Global convergence is achieved using a line search sub-process
///     and the Armijo rule
///
class NewtonSolver : public NonLinearSolver
{
public:
    using LinearSolverType = LinearSolver<SparseMatrixd>;

public:
    ///
    /// \brief Constructors/Destructors
    ///
    NewtonSolver();
    ~NewtonSolver() = default;
    NewtonSolver(const NewtonSolver &other) = delete;

    NewtonSolver &operator=(const NewtonSolver &other) = delete;

    ///
    /// \brief Solve the non linear system of equations G(x)=0 using Newton's method
    ///
    /// \param x Current iterate
    ///
    void solveGivenState(Vectord &x) override;
    void solve() override;

    ///
    /// \brief Update forcing term according to Eisenstat-Walker criteria
    ///  \see http://softlib.rice.edu/pub/CRPC-TRs/reports/CRPC-TR94463.pdf.
    ///
    /// \param ratio Ratio of consecutive function norms
    /// \param stopTolerance Linear combinarion of relative and absolute tolerances
    /// \param fnorm Current function norm
    ///
    void updateForcingTerm(const double ratio,
                           const double stopTolerance,
                           const double fnorm);

    ///
    /// \brief Set LinearSolver
    ///
    /// \param newLinearSolver Linear solver pointer
    ///
    void setLinearSolver(std::shared_ptr<LinearSolverType> newLinearSolver);

    ///
    /// \brief Get LinearSolver
    ///
    std::shared_ptr<LinearSolverType> getLinearSolver() const;

    ///
    /// \brief Update jacobians
    ///
    /// \param x Current iterate
    ///
    double updateJacobian(const Vectord &x);

    ///
    /// \brief Get JacobianMatrix. Returns jacobian matrix
    ///
    SparseMatrixd &getJacobianMatrix();

    ///
    /// \brief Set AbsoluteTolerance
    ///
    /// \param aTolerance New absolute tolerance value
    ///
    void setAbsoluteTolerance(const double aTolerance);

    ///
    /// \brief Get AbsoluteTolerance. Returns current tolerance value
    ///
    double getAbsoluteTolerance() const;

    ///
    /// \brief Set RelativeTolerance
    ///
    /// \param newRelativeTolerance New relative tolerance value
    ///
    void setRelativeTolerance(const double newRelativeTolerance)
    {
        m_relativeTolerance = newRelativeTolerance;
    }

    ///
    /// \brief Get RelativeTolerance. Returns current relative tolerance value
    ///
    double getRelativeTolerance() const { return m_relativeTolerance; }

    ///
    /// \brief Set Gamma. Sets the gamma parameter used to update the forcing term
    ///
    /// \param newGamma New gamma value
    ///
    void setGamma(const double newGamma) { m_gamma = newGamma; }

    ///
    /// \brief Get Gamma. Returns current gamma value
    ///
    double getGamma() const { return m_gamma; }

    ///
    /// \brief Set EtaMax. Maximum error tolerance for residual in inner
    ///     iteration. The inner iteration terminates when the relative linear residual is
    ///     smaller than eta*| F(x_c) |. eta is determined by the modified
    ///     Eisenstat-Walker formula if etamax > 0. If etamax < 0, then eta = |etamax|
    ///     for the entire iteration
    /// \see updateForcingTerm()
    ///
    void setEtaMax(const double newEtaMax)
    {
        m_etaMax = newEtaMax;
    }

    ///
    /// \brief Get EtaMax. Returns current etaMax value
    ///
    double getEtaMax() const { return m_etaMax; }

    ///
    /// \brief Set MaxIterations. The maximum number of nonlinear iterations
    ///
    void setMaxIterations(const size_t newMaxIterations)
    {
        m_maxIterations = newMaxIterations;
    }

    ///
    /// \brief Get MaxIterations. Returns current maximum nonlinear iterations
    ///
    size_t getMaxIterations() const { return m_maxIterations; }

    ///
    /// \brief Set the useArmijo flag. If useArmijo is true a line search is performed
    ///     using the Armijo-Goldstein condition.
    ///
    void setUseArmijo(const bool value)
    {
        m_useArmijo = value;
        (value) ? m_armijoMax = 30 : m_armijoMax = 0;
    }

    ///
    /// \brief Get MaxIterations. Returns current maximum nonlinear iterations.
    ///
    bool getUseArmijo() const { return m_useArmijo; }

    ///
    /// \brief Set the useArmijo flag. If useArmijo is true a line search is performed
    /// using the Armijo-Goldstein condition
    ///
    void setForcingTerm(const double value) { m_forcingTerm = value; }

    ///
    /// \brief Get MaxIterations. Returns current maximum nonlinear iterations
    ///
    double getForcingTerm() const { return m_forcingTerm; }

    ///
    /// \brief Set the Newton solver to be fully implicit
    ///
    virtual void setToFullyImplicit(const int maxNumIter = 50) override
    {
        m_isSemiImplicit = false;
        m_maxIterations = maxNumIter;
    }

    ///
    /// \brief Set the Newton solver to be fully implicit
    ///
    virtual void setToSemiImplicit() override
    {
        m_isSemiImplicit = true;
        m_maxIterations = 1;
    }

private:
    std::shared_ptr<LinearSolverType> m_linearSolver; ///> Linear solver to use
    double m_forcingTerm;                             ///> Method's forcing term
    double m_absoluteTolerance;                       ///> Tolerance for the method
    double m_relativeTolerance;                       ///> Relative (to the rhs) tolerance
    double m_gamma;                                   ///> Internal parameter used to update the forcing term
    double m_etaMax;                                  ///> Maximum tolerance for the linear solver
    size_t m_maxIterations;                           ///> Maximum number of nonlinear iterations
    bool m_useArmijo;                                 ///> True if Armijo liner search is desired
    std::vector<double> m_fnorms;                     ///> Consecutive function norms
};
} // imstk

