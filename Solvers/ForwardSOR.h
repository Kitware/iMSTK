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
#include "Core/Matrix.h"
#include "Core/Vector.h"

///
/// \brief Gauss Seidel successive overrelaxation sparse linear solver
///
class ForwardSOR : public IterativeLinearSolver
{
public:
    ///
    /// \brief Default constructor/destructor
    ///
    ForwardSOR() = delete;
    ForwardSOR(const ForwardSOR &) = delete;
    ForwardSOR &operator=(const ForwardSOR &) = delete;

    ForwardSOR(const core::SparseMatrixd &A, const core::Vectord &rhs, const double &w = .5);

    ~ForwardSOR() = default;

    ///
    /// \brief Do one iteration of the method
    ///
    void iterate(core::Vectord &x) override;

    ///
    /// \brief Set Weight
    ///
    void setWeight(const double &newWeight);

    ///
    /// \brief Get Weight
    ///
    const double &getWeight() const;

private:
    ForwardGaussSeidel gaussSeidel;
    double weight;

};

#endif // FORWARD_SOR_H
