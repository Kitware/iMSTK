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

#ifndef SOLVERS_BACKWARDSOR_H
#define SOLVERS_BACKWARDSOR_H

#include "Solvers/BackwardGaussSeidel.h"

namespace imstk {

///
/// \brief Forward version of the Gauss Seidel successive overrelaxation sparse
///     linear solver.
///
class BackwardSOR : public BackwardGaussSeidel
{
public:
    ///
    /// \brief Default Constructor/Destructor
    ///
    BackwardSOR();
    ~BackwardSOR() = default;
    BackwardSOR(const BackwardSOR &) = delete;

    BackwardSOR &operator=(const BackwardSOR &) = delete;

    ///
    /// \brief Constructor
    ///
    /// \param A System matrix. Symmetric and positive definite.
    /// \param rhs Right hand side of the linear system of equations.
    ///
    BackwardSOR(const SparseMatrixd &A, const Vectord &rhs, const double &w = .5);

    ///
    /// \brief Do one iteration of the method.
    ///
    /// \param x Current iterate.
    /// \param updateResidual Compute residual if true.
    ///
    void iterate(Vectord &x, bool updateResidual = true) override;

    ///
    /// \brief Set acceleration parameter.
    ///
    /// \param newWeight New acceleration paramater.
    ///
    void setWeight(const double &newWeight);

    ///
    /// \brief Get Weight. Return current acceleration parameter.
    ///
    const double &getWeight() const;

private:
    double weight;

};

}

#endif // BACKWARDSOR_H
