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

#ifndef BACKWARDSOR_H
#define BACKWARDSOR_H

#include "Solvers//BackwardGaussSeidel.h"

class BackwardSOR : public IterativeLinearSolver
{
public:
    ///
    /// \brief Default constructor/destructor
    ///
    BackwardSOR() = delete;
    BackwardSOR(const BackwardSOR &) = delete;
    BackwardSOR &operator=(const BackwardSOR &) = delete;

    BackwardSOR(const core::SparseMatrixd &A, const core::Vectord &rhs, const double &w = .5);

    ~BackwardSOR() = default;

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
    BackwardGaussSeidel gaussSeidel;
    double weight;

};

#endif // BACKWARDSOR_H
