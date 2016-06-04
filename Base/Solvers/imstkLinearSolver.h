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

#ifndef imstkLinearSolver_h
#define imstkLinearSolver_h

// imstk includes
#include "imstkLinearSystem.h"
#include "imstkMath.h"

#include "g3log/g3log.hpp"

namespace imstk {

///
/// \brief Base class for linear solvers
///
template<typename SystemMatrixType>
class LinearSolver
{
public:
    using MatrixType = SystemMatrixType;
    using LinearSystemType = LinearSystem<MatrixType>;

public:
    ///
    /// \brief Default constructor/destructor
    ///
    LinearSolver();
    virtual ~LinearSolver() = default;

    ///
    /// \brief Main solve routine
    ///
    virtual void solve(Vectord& x) = 0;

    ///
    /// \brief Set/get the system. Replaces/Returns the stored linear system of equations.
    ///
    virtual void setSystem(std::shared_ptr<LinearSystemType> newSystem);
    std::shared_ptr<LinearSystemType> getSystem() const;

    ///
    /// \brief Print solver information.
    ///
    virtual void print();

protected:
    std::shared_ptr<LinearSystemType> m_linearSystem; /// Linear system of equations
};

}
#endif // imstkLinearSolver_h
