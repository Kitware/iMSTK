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

#ifndef ASSEMBLER_ASSEMBLER_H
#define ASSEMBLER_ASSEMBLER_H

// STL includes
#include <memory>

// iMSTK includes
#include "Core/CoreClass.h"
#include "Core/Matrix.h"
#include "Core/Vector.h"

namespace imstk {

// Forward declarations
class CollisionContext;

template<typename T>
class LinearSystem;

///
/// \class Assembler This class is responsible for using the information in the
///     collision context, the internal and external forces from scene objects
///     to assemble an augmented systems of equations.
///
class Assembler : public CoreClass
{
public:
    using SparseLinearSystem = LinearSystem<SparseMatrixd>;

public:
    ///
    /// \brief Default constructor/destructor
    ///
    Assembler() = default;
    ~Assembler() = default;

    ///
    /// \brief Constructor. Takes a collision context.
    ///
    Assembler(std::shared_ptr<CollisionContext> collisionContext);

    ///
    /// \brief Set/Get Collision context.
    ///
    void setCollisionContext(std::shared_ptr<CollisionContext> newCollisionContext);
    std::shared_ptr<CollisionContext> getCollisionContext() const;

    ///
    /// \brief Set/Get System of equations.
    ///
    void setSystemOfEquations(std::vector<std::shared_ptr<SparseLinearSystem>> newSystemOfEquations);
    std::vector<std::shared_ptr<SparseLinearSystem>> getSystemOfEquations() const;

	///
	/// \brief Consolidate the forces/projectors from type 1 interactions such as
    /// forces from penalty based contact handling.
	///
    void type1Interactions();

	///
	/// \brief Initialize the system of equations from the scene models provided by the
    ///     interaction context.
	///
    void initSystem();

    ///
    /// \brief Helper to concatenate the matrix Q into a block of R.
    ///
    /// \param Q Submatrix
    /// \param R Supermatrix
    /// \param i row offset
    /// \param j column offset
    ///
    void concatenateMatrix(const SparseMatrixd &Q,
                           SparseMatrixd &R,
                           size_t i,
                           size_t j);

private:
    std::shared_ptr<CollisionContext> collisionContext; ///> Interaction context

    ///> List of systems to be solved, these can be linear, nonlinear or constrained.
    ///> Each system correspond to one type of interaction in the interaction graph.
    std::vector<std::shared_ptr<SparseLinearSystem>> equationList;

    std::vector<SparseMatrixd> A; ///> Matrices storage
    std::vector<Vectord> b; ///> Right hand sides storage
};

}

#endif // ASSEMBLER_ASSEMBLER_H
