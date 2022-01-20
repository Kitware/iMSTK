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
///
/// \class RigidBodyState2
///
/// \brief Kinematic state of rigid bodies within a system
///
class RigidBodyState2
{
public:
    void resize(const size_t size)
    {
        m_invMasses.resize(size);
        m_invIntertiaTensors.resize(size);
        m_positions.resize(size);
        m_orientations.resize(size);
        m_velocities.resize(size);
        m_angularVelocities.resize(size);
        m_tentativeVelocities.resize(size);
        m_tentativeAngularVelocities.resize(size);
        m_forces.resize(size);
        m_torques.resize(size);
        m_isStatic.resize(size);
    }

    size_t size() const { return m_invMasses.size(); }

    const std::vector<bool>& getIsStatic() const { return m_isStatic; }
    std::vector<bool>& getIsStatic() { return m_isStatic; }

    const std::vector<double>& getInvMasses() const { return m_invMasses; }
    std::vector<double>& getInvMasses() { return m_invMasses; }
    const StdVectorOfMat3d& getInvIntertiaTensors() const { return m_invIntertiaTensors; }
    StdVectorOfMat3d& getInvIntertiaTensors() { return m_invIntertiaTensors; }

    const StdVectorOfVec3d& getPositions() const { return m_positions; }
    StdVectorOfVec3d& getPositions() { return m_positions; }
    const StdVectorOfQuatd& getOrientations() const { return m_orientations; }
    StdVectorOfQuatd& getOrientations() { return m_orientations; }

    const StdVectorOfVec3d& getVelocities() const { return m_velocities; }
    StdVectorOfVec3d& getVelocities() { return m_velocities; }
    const StdVectorOfVec3d& getAngularVelocities() const { return m_angularVelocities; }
    StdVectorOfVec3d& getAngularVelocities() { return m_angularVelocities; }

    const StdVectorOfVec3d& getTentatveVelocities() const { return m_tentativeVelocities; }
    StdVectorOfVec3d& getTentatveVelocities() { return m_tentativeVelocities; }
    const StdVectorOfVec3d& getTentativeAngularVelocities() const { return m_tentativeAngularVelocities; }
    StdVectorOfVec3d& getTentativeAngularVelocities() { return m_tentativeAngularVelocities; }

    const StdVectorOfVec3d& getForces() const { return m_forces; }
    StdVectorOfVec3d& getForces() { return m_forces; }
    const StdVectorOfVec3d& getTorques() const { return m_torques; }
    StdVectorOfVec3d& getTorques() { return m_torques; }

    ///
    /// \brief Set the state to a given one, copies
    ///
    void setState(std::shared_ptr<RigidBodyState2> state)
    {
        m_invMasses = state->getInvMasses();
        m_invIntertiaTensors = state->getInvIntertiaTensors();

        m_positions    = state->getPositions();
        m_orientations = state->getOrientations();

        m_velocities = state->getVelocities();
        m_angularVelocities = state->getAngularVelocities();

        m_forces  = state->getForces();
        m_torques = state->getTorques();
    }

private:
    std::vector<double> m_invMasses;
    StdVectorOfMat3d    m_invIntertiaTensors;

    StdVectorOfVec3d m_positions;
    StdVectorOfQuatd m_orientations;

    StdVectorOfVec3d m_velocities;
    StdVectorOfVec3d m_angularVelocities;
    StdVectorOfVec3d m_tentativeVelocities; ///> Tentative velocities for mid update
    StdVectorOfVec3d m_tentativeAngularVelocities;

    StdVectorOfVec3d m_forces;
    StdVectorOfVec3d m_torques;

    std::vector<bool> m_isStatic;
};
}