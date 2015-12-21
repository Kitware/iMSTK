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

#ifndef FORWARD_SOR_H
#define FORWARD_SOR_H

// SimMedTK includes
#include "Solvers/ForwardGaussSeidel.h"

///
/// \brief Forward version of the Gauss Seidel successive overrelaxation sparse
///     linear solver.
///
class ForwardSOR : public ForwardGaussSeidel
{
public:
    ///
    /// \brief Default constructor/destructor
    ///
    ForwardSOR();
    ~ForwardSOR() = default;
    ForwardSOR(const ForwardSOR &) = delete;

    ForwardSOR &operator=(const ForwardSOR &) = delete;

    ///
    /// \brief This constructor creates a system object with A and rhs.
    ///
    /// \param A System matrix. Symmetric and positive definite.
    /// \param rhs Right hand side of the linear equation.
    ///
    ForwardSOR(const core::SparseMatrixd &A, const core::Vectord &rhs, const double &w = .5);

    ///
    /// \brief Do one iteration of the method.
    ///
    /// \param x Current iterate.
    /// \param updateResidual True if you want to compute the residual.
    ///
    void iterate(core::Vectord &x, bool updateResidual = true) override;

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

#endif // FORWARD_SOR_H
