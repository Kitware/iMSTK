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

#include "imstkSPHState.h"
#include "imstkLogger.h"

namespace imstk
{
// SPHKinematicState implementation ===>

void
SPHKinematicState::setParticleData(const StdVectorOfVec3r& positions, const StdVectorOfVec3r& velocities)
{
    if ((positions.size() != velocities.size()) && (velocities.size() != 0))
    {
        LOG(FATAL) << "Invalid input";
    }

    m_Positions  = positions;
    m_Velocities = velocities;

    if (m_Velocities.size() != m_Positions.size())
    {
        m_Velocities.resize(m_Positions.size(), Vec3r(0, 0, 0));
    }
}

void
SPHKinematicState::setState(const std::shared_ptr<SPHKinematicState>& rhs)
{
    m_Positions  = rhs->m_Positions;
    m_Velocities = rhs->m_Velocities;
}

// SPHSimulationState implementation ===>

void
SPHSimulationState::initializeData()
{
    CHECK(m_KinematicState != nullptr) << "SPH basic state has not been initialized";

    size_t numParticles = m_KinematicState->getNumParticles();

    m_Normals.resize(numParticles);
    m_Densities.resize(numParticles, 1000.0);
    m_Accels.resize(numParticles);
    m_DiffuseVelocities.resize(numParticles);
    m_NeighborInfo.resize(numParticles);
    m_NeighborLists.resize(numParticles);
}

///
/// \brief Get number of particles
///
size_t
SPHSimulationState::getNumParticles() const
{
#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
    LOG_IF(FATAL, (!m_KinematicState)) << "Particle kinematic state has not been initialized";
#endif
    return m_KinematicState->getNumParticles();
}

///
/// \brief Returns the vector of all particle positions
///
StdVectorOfVec3r&
SPHSimulationState::getPositions()
{
#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
    LOG_IF(FATAL, (!m_KinematicState)) << "Particle kinematic state has not been initialized";
#endif
    return m_KinematicState->getPositions();
}

///
/// \brief Returns the vector of all particle positions
///
const StdVectorOfVec3r&
SPHSimulationState::getPositions() const
{
#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
    LOG_IF(FATAL, (!m_KinematicState)) << "Particle kinematic state has not been initialized";
#endif
    return m_KinematicState->getPositions();
}

///
/// \brief Returns the vector of all particle velocities
///
StdVectorOfVec3r&
SPHSimulationState::getVelocities()
{
#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
    LOG_IF(FATAL, (!m_KinematicState)) << "Particle kinematic state has not been initialized";
#endif
    return m_KinematicState->getVelocities();
}

///
/// \brief Returns the vector of all particle velocities
///
const StdVectorOfVec3r&
SPHSimulationState::getVelocities() const
{
#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
    LOG_IF(FATAL, (!m_KinematicState)) << "Particle kinematic state has not been initialized";
#endif
    return m_KinematicState->getVelocities();
}
} // end namespace imstk
