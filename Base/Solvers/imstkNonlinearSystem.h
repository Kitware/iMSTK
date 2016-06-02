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

#ifndef imstkNonlinearSystem_h
#define imstkNonlinearSystem_h

#include <memory>

#include "imstkMath.h"

namespace imstk {

///
/// \class NonLinearSystem
///
/// \brief Base class for a nonlinear system of equations
///
class NonLinearSystem
{
public:
    using VectorFunctionType = std::function < const Vectord&(const Vectord&) >;
    using MatrixFunctionType = std::function < const SparseMatrixd&(const Vectord&) > ;

public:
    ///
    /// \brief default Constructor/Destructor
    ///
    NonLinearSystem() = default;
    virtual ~NonLinearSystem() = default;

    ///
    /// \brief Set function to evaluate.
    ///
    virtual void setFunction(const VectorFunctionType& function);

    ///
    /// \brief Set gradient function to evaluate.
    ///
    virtual void setJacobian(const MatrixFunctionType& function);

    ///
    /// \brief Evaluate function at specified argument
    ///
    /// \param x Value.
    /// \return Function value.
    ///
    virtual const Vectord& evaluateF(const Vectord& x);

    ///
    /// \brief Evaluate function at specified argument
    ///
    /// \param x Value.
    /// \return Function value.
    ///
    virtual const SparseMatrixd& evaluateJacobian(const Vectord& x);

public:
    VectorFunctionType m_F;  ///> Nonlinear function
    MatrixFunctionType m_dF;  ///> Gradient of the Nonlinear function with respect to the unknown vector
};

}

#endif // imstkNonlinearSystem_h
