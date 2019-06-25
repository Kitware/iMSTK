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

// imstk includes
#include "imstkLinearSystem.h"
#include "imstkMath.h"

#include "g3log/g3log.hpp"

namespace imstk
{
///
/// \brief Base class for linear solvers
///
template<typename SystemMatrixType>
class LinearSolver
{
public:
    using MatrixType = SystemMatrixType;
    using LinearSystemType = LinearSystem < MatrixType >;

    enum class Type
    {
        ConjugateGradient,
        LUFactorization,
        GaussSeidel,
        SuccessiveOverRelaxation,
        Jacobi,
        GMRES,
        none
    };

public:
    ///
    /// \brief Default constructor/destructor
    ///
    LinearSolver() {};
    virtual ~LinearSolver() {};

    ///
    /// \brief Main solve routine
    ///
    virtual void solve(Vectord& x) = 0;

    ///
    /// \brief Set/get the system. Replaces/Returns the stored linear system of equations.
    ///
    virtual void setSystem(std::shared_ptr<LinearSystemType> newSystem)
    {
        m_linearSystem.reset();
        m_linearSystem = newSystem;
    }

    std::shared_ptr<LinearSystemType> getSystem() const
    {
        return m_linearSystem;
    }

    ///
    /// \brief Set solver tolerance
    ///
    void setTolerance(const double tolerance)
    {
        m_tolerance = tolerance;
    }

    ///
    /// \brief Get solver tolerance
    ///
    double getTolerance() const
    {
        return m_tolerance;
    }

    ///
    /// \brief Print solver information.
    ///
    virtual void print() const
    {
        LOG(INFO) << "Solver type (broad): Linear";
    };

    ///
    /// \brief Returns true if the solver is iterative
    ///
    virtual bool isIterative() const = 0;

    ///
    /// \brief Return the type of the solver
    ///
    Type getType() { return m_type; };

protected:
    Type m_type = Type::none;                                 ///> Type of the scene object
    double m_tolerance = 1.0e-4;                        ///> default tolerance
    std::shared_ptr<LinearSystemType> m_linearSystem;   ///> Linear system of equations
};
} // imstk
