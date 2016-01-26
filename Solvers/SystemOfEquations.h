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

#ifndef SOLVERS_SYSTEMOFEQUATIONS_H
#define SOLVERS_SYSTEMOFEQUATIONS_H

#include <memory>

// iMSTK includes
#include "Core/Vector.h"
#include "Core/Matrix.h"

namespace imstk {

///
/// \class SystemOfEquations
///
/// \brief Base class for all system of equations. This includes linear, nonlinear,
///     constrained, and differential equation systems.
///
class SystemOfEquations
{
public:
    using VectorFunctionType = std::function<const Vectord&(const Vectord &)>;
    using MatrixFunctionType = std::function<const SparseMatrixd&(const Vectord &)>;

public:
    ///
    /// \brief default Constructor/Destructor
    ///
    SystemOfEquations() = default;
    virtual ~SystemOfEquations() = default;

    ///
    /// \brief Set function to evaluate.
    ///
    inline virtual void setFunction(const VectorFunctionType &function)
    {
        this->F = function;
    }

    ///
    /// \brief Set function to evaluate.
    ///
    inline virtual void setJacobian(const MatrixFunctionType &function)
    {
        this->DF = function;
    }

    ///
    /// \brief Evaluate function at specified argument
    ///
    /// \param x Value.
    /// \return Function value.
    ///
    inline virtual const Vectord &evalF(const Vectord &x)
    {
        return this->F(x);
    }

    ///
    /// \brief Evaluate function at specified argument
    ///
    /// \param x Value.
    /// \return Function value.
    ///
    inline virtual const SparseMatrixd &evalDF(const Vectord &x)
    {
        return this->DF(x);
    }

public:
    VectorFunctionType F;  ///> Function associated with the system of equation to solve.
    MatrixFunctionType DF;  ///> Function associated with the system of equation to solve.
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
    LinearSystem(const MatrixType &matrix, const Vectord &b) :
        A(matrix),
        rhs(b)
    {
        this->F = [this](const Vectord & x) -> Vectord &
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
    inline const Vectord &getRHSVector() const
    {
        return this->rhs;
    }

    ///
    /// \brief Set the system rhs corresponding to this system.
    ///
    /// \param newRhs new rhs.
    ///
    inline void setRHSVector(const Vectord &newRhs)
    {
        this->rhs = newRhs;
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
    /// \brief Set the system matrix corresponding to this ODE system.
    ///
    /// \param newMatrix New matrix.
    ///
    inline void setMatrix(const SparseMatrixd &newMatrix)
    {
        this->A = newMatrix;
    }

    ///
    /// \brief Compute the residual as \f$\left \| b-Ax \right \|_2\f$.
    ///
    /// \param x Current iterate.
    /// \param r Upon return, contains the residual vector.
    ///
    /// \return Residual vector r.
    ///
    inline Vectord &computeResidual(const Vectord &x, Vectord &r) const
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

    ///
    /// \brief Get the value of the function F
    ///
    /// \return Function value.
    ///
    inline Vectord &getFunctionValue()
    {
        return this->f;
    }

private:
    const MatrixType &A;
    const Vectord &rhs;
    Vectord f; ///> Scratch storage for matrix-vector operations
};

}

#endif // SYSTEM_OF_EQUATIONS
