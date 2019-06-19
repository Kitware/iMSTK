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

#ifndef imstkJacobi_h
#define imstkJacobi_h

#include <memory>

// iMSTK includes
#include "imstkNonlinearSystem.h"
#include "imstkIterativeLinearSolver.h"

// Eigen includes
#include <Eigen/IterativeLinearSolvers>

namespace imstk
{
///
/// \brief Jacobi sparse linear solver
///
class Jacobi : public IterativeLinearSolver
{
public:
    ///
    /// \brief Constructors/Destructor
    ///
    Jacobi() { m_type = Type::Jacobi; };
    Jacobi(const SparseMatrixd &A, const Vectord& rhs);
    ~Jacobi() = default;

    ///
    /// \brief Remove specific constructor signatures
    ///
    Jacobi(const Jacobi &) = delete;
    Jacobi &operator=(const Jacobi &) = delete;

    ///
    /// \brief Do one iteration of the method.
    ///
    void iterate(Vectord& x, bool updateResidual = true) override {};

    ///
    /// \brief Jacobi solver
    ///
    void JacobiSolve(Vectord& x);

    ///
    /// \brief Solve the system of equations.
    ///
    void solve(Vectord& x) override;

    ///
    /// \brief Solve the linear system using Gauss-Seidel iterations to a
    /// specified tolerance.
    ///
    void solve(Vectord& x, const double tolerance);

    ///
    /// \brief Return the error calculated by the solver.
    ///
    double getResidual(const Vectord& x) override;

    ///
    /// \brief Sets the system. System of linear equations.
    ///
    void setSystem(std::shared_ptr<LinearSystemType> newSystem) override;

    ///
    /// \brief set/get the maximum number of iterations for the iterative solver.
    ///
    virtual void setMaxNumIterations(const size_t maxIter) override;

    ///
    /// \brief Set solver tolerance
    ///
    void setTolerance(const double tolerance);

    ///
    /// \brief Print solver information
    ///
    void print() const override;

    ///
    /// \brief Get the vector denoting the filter
    ///
    void setLinearProjectors(std::vector<LinearProjectionConstraint>* f)
    {
        m_FixedLinearProjConstraints = f;
    }

    ///
    /// \brief Get the vector denoting the filter
    ///
    std::vector<LinearProjectionConstraint>& getLinearProjectors()
    {
        return *m_FixedLinearProjConstraints;
    }

    ///
    /// \brief Get the vector denoting the filter
    ///
    void setDynamicLinearProjectors(std::vector<LinearProjectionConstraint>* f)
    {
        m_DynamicLinearProjConstraints = f;
    }

    ///
    /// \brief Get the vector denoting the filter
    ///
    std::vector<LinearProjectionConstraint>& getDynamicLinearProjectors()
    {
        return *m_DynamicLinearProjConstraints;
    }

private:

    std::vector<LinearProjectionConstraint>  *m_FixedLinearProjConstraints;
    std::vector<LinearProjectionConstraint>  *m_DynamicLinearProjConstraints;
};
} // imstk

#endif // imstkJacobi_h
