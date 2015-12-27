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

///
/// \brief Stores the state of the differential equation.
///
class OdeSystemState
{
public:

    /// Default constructor/destructor
    OdeSystemState() = default;
    ~OdeSystemState() = default;

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
    void resize(size_t size)
    {
        positions.resize(size);
        velocities.resize(size);
    }
private:
    core::Vectord positions; ///> State position.
    core::Vectord velocities; ///> State velocity.
};

#endif // ODESYSTEMSTATE_H
