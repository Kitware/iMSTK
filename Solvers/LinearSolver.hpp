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
#ifndef SM_LINEAR_SOLVER_HPP
#define SM_LINEAR_SOLVER_HPP

template<typename SystemMatrixType>
LinearSolver<SystemMatrixType>::LinearSolver() : linearSystem(nullptr), minTolerance(1.0e-6){}

//---------------------------------------------------------------------------
template<typename SystemMatrixType>
void LinearSolver<SystemMatrixType>::setSystem(std::shared_ptr<LinearSystem<SystemMatrixType>> newSystem)
{
    this->linearSystem.reset();
    this->linearSystem = newSystem;
}

//---------------------------------------------------------------------------
template<typename SystemMatrixType>
std::shared_ptr<LinearSystem<SystemMatrixType>> LinearSolver<SystemMatrixType>::getSystem() const
{
    return this->linearSystem;
}

//---------------------------------------------------------------------------
template<typename SystemMatrixType>
void LinearSolver<SystemMatrixType>::setTolerance(const double newTolerance)
{
    this->minTolerance = newTolerance;
}

//---------------------------------------------------------------------------
template<typename SystemMatrixType>
double LinearSolver<SystemMatrixType>::getTolerance() const
{
    return this->minTolerance;
}

#endif // SM_LINEAR_SOLVER
