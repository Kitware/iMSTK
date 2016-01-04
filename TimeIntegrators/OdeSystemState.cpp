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

#include "TimeIntegrators/OdeSystemState.h"

OdeSystemState::OdeSystemState(const size_t size)
{
    this->resize(size);
}

//---------------------------------------------------------------------------
const core::Vectord &OdeSystemState::getPositions() const
{
    return this->positions;
}

//---------------------------------------------------------------------------
core::Vectord &OdeSystemState::getPositions()
{
    return this->positions;
}

//---------------------------------------------------------------------------
const core::Vectord &OdeSystemState::getVelocities() const
{
    return this->velocities;
}

//---------------------------------------------------------------------------
core::Vectord &OdeSystemState::getVelocities()
{
    return this->velocities;
}

//---------------------------------------------------------------------------
void OdeSystemState::resize(const size_t size)
{
    positions.resize(size);
    velocities.resize(size);
    positions.setZero();
    velocities.setZero();
}

//---------------------------------------------------------------------------
void OdeSystemState::setBoundaryConditions(const std::vector< std::size_t > &boundaryConditions)
{
    this->fixedVertices = boundaryConditions;
}

//---------------------------------------------------------------------------
void OdeSystemState::applyBoundaryConditions(core::SparseMatrixd &M, bool withCompliance) const
{
    double compliance = withCompliance ? 1.0 : 0.0;

    // Set column and row to zero.
    for(auto & index : this->fixedVertices)
    {
        auto idx = static_cast<core::SparseMatrixd::Index>(index);

        for(int k = 0; k < M.outerSize(); ++k)
        {
            for(core::SparseMatrixd::InnerIterator i(M, k); i; ++i)
            {
                if(i.row() == idx || i.col() == idx)
                {
                    i.valueRef() = 0.0;
                }

                if(i.row() == idx && i.col() == idx)
                {
                    i.valueRef() = compliance;
                }
            }
        }
    }
}

//---------------------------------------------------------------------------
void OdeSystemState::applyBoundaryConditions(core::Matrixd &M, bool withCompliance) const
{
    double compliance = withCompliance ? 1.0 : 0.0;

    for(auto & index : this->fixedVertices)
    {
        M.middleRows(index, 1).setZero();
        M.middleCols(index, 1).setZero();
        M(index, index) = compliance;
    }
}

//---------------------------------------------------------------------------
void OdeSystemState::applyBoundaryConditions(core::Vectord &x) const
{
    for(auto & index : this->fixedVertices)
    {
        x(index) = 0.0;
    }
}
