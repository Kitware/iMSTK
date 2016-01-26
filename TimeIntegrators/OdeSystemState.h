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

#ifndef TIME_INTEGRATORS_ODESYSTEMSTATE_H
#define TIME_INTEGRATORS_ODESYSTEMSTATE_H

#include "Core/Vector.h"
#include "Core/Matrix.h"

namespace imstk {

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
    OdeSystemState(const size_t size);

    ///
    /// \brief Get constant reference to positions.
    ///
    /// \return Constant reference to positions.
    ///
    const Vectord &getPositions() const;

    ///
    /// \brief Get a writable reference to positions.
    ///
    /// \return Writable reference to positions.
    ///
    Vectord &getPositions();

    ///
    /// \brief Get a constant reference to velocities.
    ///
    /// \return Constant reference to velocities.
    ///
    const Vectord &getVelocities() const;

    ///
    /// \brief Get a writable reference to velocities.
    ///
    /// \return Writable reference to velocities.
    ///
    Vectord &getVelocities();

    ///
    /// \brief Resize positions and velocity vectors.
    ///     Note that this will effectively wipe the storage.
    ///
    void resize(const size_t size);

    ///
    /// \brief Set a vector containing the indices of fixed dofs.
    ///
    /// \param boundaryConditions Vector of indices.
    ///
    void setBoundaryConditions(const std::vector<size_t> &boundaryConditions);

    ///
    /// \brief Apply boundary conditions to sparse matrix.
    ///
    /// \param M Sparse matrix container.
    /// \param withCompliance True if the fixed vertices should have compliance.
    ///
    void applyBoundaryConditions(SparseMatrixd &M, bool withCompliance = true) const;

    ///
    /// \brief Apply boundary conditions to dense matrix.
    ///
    /// \param M Dense matrix container.
    /// \param withCompliance True if the fixed vertices should have complieance.
    ///
    void applyBoundaryConditions(Matrixd &M, bool withCompliance = true) const;

    ///
    /// \brief Apply boundary conditions to a vector.
    ///
    /// \param x vector container.
    ///
    void applyBoundaryConditions(Vectord &x) const;

private:
    Vectord positions; ///> State position.
    Vectord velocities; ///> State velocity.

    std::vector<size_t> fixedVertices; ///> Fixed dof ids
};

}

#endif // ODESYSTEMSTATE_H
