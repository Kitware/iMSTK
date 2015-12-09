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

#include "Solvers/DirectLinearSolver.h"

DirectLinearSolver::DirectLinearSolver(
    const std::shared_ptr<LinearSystem> linSys /*= nullptr*/) :
    LinearSolver(linSys)
{
}

DirectLinearSolver::~DirectLinearSolver()
{
}

Eigen::Vector<double>& DirectLinearSolver::getUnknownVector()
{
    return this->linSystem->getUnknownVector();
}

Eigen::Vector<double>& DirectLinearSolver::getForceVector()
{
    return this->linSystem->getForceVector();
}

void DirectLinearSolver::Solve()
{
    if (this->sysOfEquations != nullptr)
    {
        auto linearSysSparse =
            std::dynamic_pointer_cast<SparseLinearSystem>(this->sysOfEquations);

        if (linearSysSparse != nullptr)
        {


            return;
        }
        else
        {
            auto linearSysDense =
                std::dynamic_pointer_cast<DenseLinearSystem>(this->sysOfEquations);



            return;

        }
    }
    else
    {
        std::cout << "Error: Linear system not set to the direct solver!\n";
    }
}
