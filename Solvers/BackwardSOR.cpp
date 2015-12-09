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

#include "BackwardSOR.h"

BackwardSOR::BackwardSOR(const core::SparseMatrixd &A,
                         const core::Vectord &rhs,
                         const double &w): gaussSeidel(A, rhs), weight(w)
{}

//---------------------------------------------------------------------------
void BackwardSOR::iterate(core::Vectord &x, bool updateResidual)
{
    auto old = x; // necessary copy
    this->gaussSeidel.iterate(x, updateResidual);
    x = this->weight * x + (1 - this->weight) * old;
}

//---------------------------------------------------------------------------
void BackwardSOR::setWeight(const double &newWeight)
{
    this->weight = newWeight;
}

//---------------------------------------------------------------------------
const double &BackwardSOR::getWeight() const
{
    return this->weight;
}
