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
#ifndef SM_ASSEMBLY
#define SM_ASSEMBLY

#include <memory>

// SimMedTK includes
#include "Core/CoreClass.h"

//#include "Core/SDK.h"
#include "Core/Config.h"
#include "Core/ContactHandling.h"
#include "CollisionContext/CollisionContext.h"
#include "Solvers/SystemOfEquations.h"
//#include "Core/SceneObject.h"
#include "Core/Matrix.h"

///
/// \class Assembler
///
/// \brief This class is responsible for using the
/// information in the collision context and the
/// internal, external forces from scene objects
/// to output systems of equations to be solved by
/// solver module.
///
class Assembler : public CoreClass
{
public:
    using LinearSystemType = LinearSystem<core::SparseMatrixd>;

public:
    ///
    /// \brief Default constructor
    ///
    Assembler() = default;

    ///
    /// \brief Destructor
    ///
    ~Assembler() = default;

    ///
    /// \brief Constructor. Takes a collision context.
    ///
    Assembler(std::shared_ptr<CollisionContext> collisionContext);

	///
	/// \brief consolidate the forces/projectors from type 1 interactions such as
    /// forces from penalty based contact handling
	///
    void type1Interactions();

	///
	/// \brief
	///
    void initSystem();

    ///
    /// \brief Helper to concatenate the matrix Q into an block of R.
    ///
    /// \param Q Submatrix
    /// \param R Supermatrix
    /// \param i row offset
    /// \param j column offset
    ///
    void concatenateMatrix(const core::SparseMatrixd &Q, core::SparseMatrixd &R, size_t i, size_t j);

private:
    // inputs
    std::shared_ptr<CollisionContext> collisionContext;

    // output
    std::vector<std::shared_ptr<LinearSystem<core::SparseMatrixd>>> systemOfEquationsList;

    std::vector<core::SparseMatrixd> A;
    std::vector<core::Vectord> b;
};

#endif // SM_ASSEMBLY
