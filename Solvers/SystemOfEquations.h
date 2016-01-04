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

#ifndef SYSTEM_OF_EQUATIONS
#define SYSTEM_OF_EQUATIONS

#include <memory>

#include "Core/Vector.h"
#include "Core/Matrix.h"

///
/// \class SystemOfEquations
///
/// \brief Base class for system of equations
///
class SystemOfEquations
{
public:
    using FunctionType = std::function<const core::Vectord&(const core::Vectord &)>;

public:
    ///
    /// \brief default Constructor/Destructor
    ///
    SystemOfEquations() = default;
    virtual ~SystemOfEquations() = default;

    ///
    /// \brief Set function to evaluate.
    ///
    inline void setFunction(const FunctionType &function)
    {
        this->F = function;
    }

    ///
    /// \brief Evaluate function at specified argument
    ///
    /// \param x Value.
    /// \return Function value.
    ///
    inline const core::Vectord &eval(const core::Vectord &x)
    {
        this->f = this->F(x);
        return this->f;
    }

    ///
    /// \brief Get the value of the function F
    ///
    /// \return Function value.
    ///
    inline core::Vectord &getFunctionValue()
    {
        return this->f;
    }

protected:
    FunctionType F;  ///> Function associated with the system of equation to solve.
    core::Vectord f; ///> Storage for function values
};

///
/// \class LinearSystem
/// \brief Represents the linear system \f$ Ax = b \f$
///
template<typename SystemMatrixType>
class LinearSystem : public SystemOfEquations
{
public:
    using MatrixType = SystemMatrixType;

public:
    ///
    /// \brief Constructor/destructor(s). This class can't be constructed without
    ///     a matrix and rhs. Also, avoid copying this system.
    ///
    LinearSystem() = delete;
    virtual ~LinearSystem() = default;
    LinearSystem(const LinearSystem &) = delete;

    LinearSystem &operator=(const LinearSystem &) = delete;

    ///
    /// \brief Constructor.
    ///
    LinearSystem(const MatrixType &matrix, const core::Vectord &b) :
        A(matrix),
        rhs(b)
    {
        this->F = [this](const core::Vectord & x) -> core::Vectord &
        {
            this->f = this->A * x;
            return this->f;
        };
    }

    // -------------------------------------------------
    //  Accessors
    // -------------------------------------------------

    ///
    ///  \brief Returns a reference to local right hand side vector.
    ///
    ///  \return Right hand side.
    ///
    inline const core::Vectord &getRHSVector() const
    {
        return this->rhs;
    }

    ///
    /// \brief Returns reference to local matrix.
    ///
    /// \return Systems matrix.
    ///
    inline const MatrixType &getMatrix() const
    {
        return A;
    }

    ///
    /// \brief Compute the residual as \f$\left \| b-Ax \right \|_2\f$.
    ///
    /// \param x Current iterate.
    /// \param r Upon return, contains the residual vector.
    ///
    /// \return Residual vector r.
    ///
    inline core::Vectord &computeResidual(const core::Vectord &x, core::Vectord &r) const
    {
        r = this->rhs - this->F(x);
        return r;
    }

    ///
    /// \brief Returns template expression for the lower triangular part of A.
    ///
    inline Eigen::SparseTriangularView<MatrixType,Eigen::Lower>
    getLowerTriangular() const
    {
        return this->A.template triangularView<Eigen::Lower>();
    }

    ///
    /// \brief Returns template expression for the strict lower triangular part of A.
    ///
    inline Eigen::SparseTriangularView<MatrixType,Eigen::StrictlyLower>
    getStrictLowerTriangular() const
    {
        return this->A.template triangularView<Eigen::StrictlyLower>();
    }

    ///
    /// \brief Returns template expression for the upper triangular part of A.
    ///
    inline Eigen::SparseTriangularView<MatrixType,Eigen::Upper>
    getUpperTrianglular() const
    {
        return this->A.template triangularView<Eigen::Upper>();
    }

    ///
    /// \brief Returns template expression for the strict upper triangular part of A.
    ///
    inline Eigen::SparseTriangularView<MatrixType,Eigen::StrictlyUpper>
    getStrictUpperTriangular() const
    {
        return this->A.template triangularView<Eigen::StrictlyUpper>();
    }

private:
    const MatrixType &A;
    const core::Vectord &rhs;
};

#endif // SYSTEM_OF_EQUATIONS
