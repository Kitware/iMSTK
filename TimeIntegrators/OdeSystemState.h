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

#ifndef ODESYSTEMSTATE_H
#define ODESYSTEMSTATE_H

#include "Core/Vector.h"
#include "Core/Matrix.h"

///
/// \brief Stores the state of the differential equation.
///
class OdeSystemState
{
public:
    ///
    /// \brief Default constructor/destructor.
    ///
    OdeSystemState() = default;
    ~OdeSystemState() = default;

    /// Constructor with system size.
    OdeSystemState(const size_t size)
    {
        this->resize(size);
    }

    ///
    /// \brief Set the derivative with respect to v of the right hand side.
    ///
    /// \return Constant reference to positions.
    ///
    const core::Vectord &getPositions() const
    {
        return this->positions;
    }

    ///
    /// \brief Set the derivative with respect to v of the right hand side.
    ///
    /// \return Reference to positions.
    ///
    core::Vectord &getPositions()
    {
        return this->positions;
    }

    ///
    /// \brief Set the derivative with respect to v of the right hand side.
    ///
    /// \return Constant reference to velocities.
    ///
    const core::Vectord &getVelocities() const
    {
        return this->velocities;
    }

    ///
    /// \brief Set the derivative with respect to v of the right hand side.
    ///
    /// \return Reference to velocities.
    ///
    core::Vectord &getVelocities()
    {
        return this->velocities;
    }

    ///
    /// \brief Resize positions and velocity vectors.
    ///
    /// \return Reference to velocities.
    ///
    void resize(const size_t size)
    {
        positions.resize(size);
        velocities.resize(size);
        positions.setZero();
        velocities.setZero();
    }

    ///
    /// \brief Return vector containing the indices of fixed dofs
    ///
    /// \return Reference to vector indices.
    ///
    void setBoundaryConditions(const std::vector<size_t> &boundaryConditions)
    {
        this->fixedVertices = boundaryConditions;
    }

    ///
    /// \brief Apply boundary conditions to sparse matrix.
    ///
    /// \param M Sparse matrix container.
    /// \param withCompliance True if the fixed vertices should have compliance.
    ///
    void applyBoundaryConditions(core::SparseMatrixd &M, bool withCompliance = true) const
    {
        double compliance = withCompliance ? 1.0 : 0.0;
        // Set column and row to zero.
        for(auto &index : this->fixedVertices)
        {
            auto idx = static_cast<core::SparseMatrixd::Index>(index);
            for (int k = 0; k < M.outerSize(); ++k)
            {
                for(core::SparseMatrixd::InnerIterator i(M,k); i; ++i)
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

    ///
    /// \brief Apply boundary conditions to dense matrix.
    ///
    /// \param M Dense matrix container.
    /// \param withCompliance True if the fixed vertices should have complieance.
    ///
    void applyBoundaryConditions(core::Matrixd &M, bool withCompliance = true) const
    {
        double compliance = withCompliance ? 1.0 : 0.0;

        for(auto &index : this->fixedVertices)
        {
            M.middleRows(index,1).setZero();
            M.middleCols(index,1).setZero();
            M(index,index) = compliance;
        }
    }

    ///
    /// \brief Apply boundary conditions to a vector.
    ///
    /// \param x vector container.
    ///
    void applyBoundaryConditions(core::Vectord &x) const
    {
        for(auto &index : this->fixedVertices)
        {
            x(index) = 0.0;
        }
    }

private:
    core::Vectord positions; ///> State position.
    core::Vectord velocities; ///> State velocity.

    std::vector<size_t> fixedVertices; ///> Fixed dof ids
};

#endif // ODESYSTEMSTATE_H
