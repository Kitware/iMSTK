/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

=========================================================================*/

#pragma once

#include "imstkMath.h"

namespace imstk
{
class PointSet;
///
/// \class PbdState
///
/// \brief State of the body governed by PBD mathematical model
///
class PbdState
{
public:
    ///
    /// \brief Default constructor/destructor
    ///
    PbdState()  = default;
    ~PbdState() = default;

    ///
    /// \brief Initialize the pbd state
    ///
    void initialize(const size_t numNodes);
    void initialize(const StdVectorOfVec3d& vertices);

    ///
    /// \brief Get/Set nodal position given the index
    ///
    void setVertexPosition(const size_t& idx, const Vec3d& pos) { m_pos->at(idx) = pos; }
    Vec3d& getVertexPosition(const size_t& idx) { return m_pos->at(idx); }

    ///
    /// \brief Returns the vector of current nodal positions
    ///
    std::shared_ptr<StdVectorOfVec3d> getPositions() { return m_pos; }
    void setPositions(std::shared_ptr<StdVectorOfVec3d> p)
    {
        m_pos->resize(p->size());
        std::copy(p->begin(), p->end(), m_pos->begin());
    }

    void setPositions(const StdVectorOfVec3d& p)
    {
        m_pos->resize(p.size());
        std::copy(p.begin(), p.end(), m_pos->begin());
    }

    ///
    /// \brief Returns the vector of current nodal velocities
    ///
    std::shared_ptr<StdVectorOfVec3d> getVelocities() { return m_vel; }

    ///
    /// \brief Returns the vector of current nodal accelerations
    ///
    std::shared_ptr<StdVectorOfVec3d> getAccelerations() { return m_acc; }

    ///
    /// \brief Set the state to a given one
    ///
    void setState(std::shared_ptr<PbdState> rhs);

private:
    std::shared_ptr<StdVectorOfVec3d> m_pos = std::make_shared<StdVectorOfVec3d>(); ///> Nodal positions
    std::shared_ptr<StdVectorOfVec3d> m_vel = std::make_shared<StdVectorOfVec3d>(); ///> Nodal velocities
    std::shared_ptr<StdVectorOfVec3d> m_acc = std::make_shared<StdVectorOfVec3d>(); ///> Nodal acelerations
};
} // imstk
