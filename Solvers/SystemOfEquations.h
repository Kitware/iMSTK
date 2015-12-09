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

#ifndef SM_SYSTEM_OF_EQUATIONS
#define SM_SYSTEM_OF_EQUATIONS

#include <memory>

#include "Core/Vector.h"

///
/// \class systemOfEquations
///
/// \brief Base class for system of equations
///
class SystemOfEquations
{
public:
    using FunctionType = std::function < core::Vectord &(const core::Vectord &,
                         core::Vectord &) >;

public:
    ///
    /// \brief default Constructor/Destructor
    ///
    SystemOfEquations() = default;
    virtual ~SystemOfEquations() = default;

    ///
    /// \brief Set function
    ///
    void setFunction(const FunctionType &function)
    {
        this->F = function;
    }

protected:
    //!< Function associated with the system of equation to solve: F(x)=b
    FunctionType F;
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
    LinearSystem(const LinearSystem &) = delete;
    LinearSystem &operator=(const LinearSystem &) = delete;
    virtual ~LinearSystem() = default;

    ///
    /// \brief Constructor
    ///
    LinearSystem(const MatrixType &matrix, const core::Vectord &b) : A(matrix), rhs(b)
    {
        this->F = [this](const core::Vectord & x, core::Vectord & y) -> core::Vectord &
        {
            return y = this->A * x;
        };
    }

    // -------------------------------------------------
    //  Accessors
    // -------------------------------------------------

    ///
    /// \brief Set new right hand side vector
    ///
    void setRHSVector(const core::Vectord &newRhs)
    {
        this->rhs = newRhs;
    }

    ///
    ///  \brief Get the right hand side vector
    ///
    inline const core::Vectord &getRHSVector() const
    {
        return this->rhs;
    }

    ///
    /// \brief Return reference to local matrix
    ///
    inline const MatrixType &getMatrix() const
    {
        return A;
    }

    ///
    /// \brief Compute the residual as \f$\left \| b-Ax \right \|_2\f$
    ///
    inline core::Vectord &computeResidual(const core::Vectord &x, core::Vectord &r) const
    {
        r = this->rhs - this->F(x, r);
        return r;
    }

private:
    const MatrixType &A;
    const core::Vectord &rhs;
};

#endif // SM_SYSTEM_OF_EQUATIONS
