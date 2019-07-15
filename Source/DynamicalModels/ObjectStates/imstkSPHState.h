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

#include <memory>
#include "imstkMath.h"

#include <g3log/g3log.hpp>

namespace imstk
{
///
/// \class SPHKinematicState
/// \brief State of the SPH fluid particles
///
class SPHKinematicState
{
public:
    ///
    /// \brief Default constructor/destructor
    ///
    SPHKinematicState()          = default;
    virtual ~SPHKinematicState() = default;

    ///
    /// \brief Set particle data with given positions and velocities
    ///
    void setParticleData(const StdVectorOfVec3r& positions, const StdVectorOfVec3r& velocities = {});

    ///
    /// \brief Get number of particles
    ///
    size_t getNumParticles() const { return m_Positions.size(); }

    ///
    /// \brief Returns the vector of all particle positions
    ///
    StdVectorOfVec3r& getPositions() { return m_Positions; }
    const StdVectorOfVec3r& getPositions() const { return m_Positions; }

    ///
    /// \brief Returns the vector of all particle velocities
    ///
    StdVectorOfVec3r& getVelocities() { return m_Velocities; }
    const StdVectorOfVec3r& getVelocities() const { return m_Velocities; }

    ///
    /// \brief Set the state to a given one
    ///
    void setState(const std::shared_ptr<SPHKinematicState>& rhs);

private:
    StdVectorOfVec3r m_Positions;   ///> Particle positions
    StdVectorOfVec3r m_Velocities;  ///> Particle velocities
};

///
/// \struct NeighborInfo
/// \brief The helper struct to store relative positions and densities of neighbor particlcles
///
struct NeighborInfo
{
    Vec3r xpq;     ///> relative position: xpq = x_p - x_q
    Real density;  ///> density of neighbor particle q
};

///
/// \class SPHSimulationState
/// \brief Simulation states of SPH particles
///
class SPHSimulationState
{
public:
    ///
    /// \brief Default constructor/destructor
    ///
    SPHSimulationState()          = default;
    virtual ~SPHSimulationState() = default;

    ///
    /// \brief Set the kinematic state: positions and velocities
    ///
    void setKinematicState(const std::shared_ptr<SPHKinematicState>& state) { m_KinematicState = state; }

    ///
    /// \brief Set positions of the boundary (solid) particles
    ///
    void setBoundaryParticlePositions(const StdVectorOfVec3r& positions) { m_BDPositions = positions; }

    ///
    /// \brief Initialize simulation variables, must be called after setKinematicState and (if applicable) after setBoundaryParticlePositions
    ///
    void initializeData();

    ///
    /// \brief Get number of particles
    ///
    size_t getNumParticles() const
    {
#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
        LOG_IF(FATAL, (!m_KinematicState)) << "Particle kinematic state has not been initialized";
#endif
        return m_KinematicState->getNumParticles();
    }

    ///
    /// \brief Returns the vector of all particle positions
    ///
    StdVectorOfVec3r& getPositions()
    {
#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
        LOG_IF(FATAL, (!m_KinematicState)) << "Particle kinematic state has not been initialized";
#endif
        return m_KinematicState->getPositions();
    }

    ///
    /// \brief Returns the vector of all particle positions
    ///
    const StdVectorOfVec3r& getPositions() const
    {
#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
        LOG_IF(FATAL, (!m_KinematicState)) << "Particle kinematic state has not been initialized";
#endif
        return m_KinematicState->getPositions();
    }

    ///
    /// \brief Returns the vector of all particle velocities
    ///
    StdVectorOfVec3r& getVelocities()
    {
#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
        LOG_IF(FATAL, (!m_KinematicState)) << "Particle kinematic state has not been initialized";
#endif
        return m_KinematicState->getVelocities();
    }

    ///
    /// \brief Returns the vector of all particle velocities
    ///
    const StdVectorOfVec3r& getVelocities() const
    {
#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
        LOG_IF(FATAL, (!m_KinematicState)) << "Particle kinematic state has not been initialized";
#endif
        return m_KinematicState->getVelocities();
    }

    ///
    /// \brief Returns the vector of all particle positions
    ///
    StdVectorOfVec3r& getBoundaryParticlePositions() { return m_BDPositions; }
    const StdVectorOfVec3r& getBoundaryParticlePositions() const { return m_BDPositions; }

    ///
    /// \brief Returns the vector of all particle surface normals
    ///
    StdVectorOfVec3r& getNormals() { return m_Normals; }
    const StdVectorOfVec3r& getNormals() const { return m_Normals; }

    ///
    /// \brief Returns the vector of all particle densities
    ///
    StdVectorOfReal& getDensities() { return m_Densities; }
    const StdVectorOfReal& getDensities() const { return m_Densities; }

    ///
    /// \brief Returns the vector of all particle densities
    ///
    StdVectorOfReal& getNormalizedDensities() { return m_NormalizedDensities; }
    const StdVectorOfReal& getNormalizedDensities() const { return m_NormalizedDensities; }

    ///
    /// \brief Returns the vector of all particle accelerations
    ///
    StdVectorOfVec3r& getAccelerations() { return m_Accels; }
    const StdVectorOfVec3r& getAccelerations() const { return m_Accels; }

    ///
    /// \brief Returns the vector of all velocity diffusion
    ///
    StdVectorOfVec3r& getDiffuseVelocities() { return m_DiffuseVelocities; }
    const StdVectorOfVec3r& getDiffuseVelocities() const { return m_DiffuseVelocities; }

    ///
    /// \brief Returns the vector of neighbor fluid particles
    ///
    std::vector<std::vector<size_t>>& getFluidNeighborLists() { return m_NeighborLists; }
    const std::vector<std::vector<size_t>>& getFluidNeighborLists() const { return m_NeighborLists; }

    ///
    /// \brief Returns the vector of neighbor of boundary particles
    ///
    std::vector<std::vector<size_t>>& getBoundaryNeighborLists() { return m_BDNeighborLists; }
    const std::vector<std::vector<size_t>>& getBoundaryNeighborLists() const { return m_BDNeighborLists; }

    ///
    /// \brief Returns the vector of neighbor information ( {relative position, density} ), which is cached for other computation
    ///
    std::vector<std::vector<NeighborInfo>>& getNeighborInfo() { return m_NeighborInfo; }
    const std::vector<std::vector<NeighborInfo>>& getNeighborInfo() const { return m_NeighborInfo; }

private:
    std::shared_ptr<SPHKinematicState> m_KinematicState;      ///> basic state: positions + velocities
    StdVectorOfVec3r m_BDPositions;                           ///> positions of boundary particles, if generated

    StdVectorOfReal  m_Densities;                             ///>  particle densities
    StdVectorOfReal  m_NormalizedDensities;                   ///>  variable for normalizing densities
    StdVectorOfVec3r m_Normals;                               ///>  surface normals
    StdVectorOfVec3r m_Accels;                                ///>  acceleration
    StdVectorOfVec3r m_DiffuseVelocities;                     ///>  velocity diffusion, used for computing viscosity
    std::vector<std::vector<size_t>>       m_NeighborLists;   ///>  store a list of neighbors for each particle, updated each time step
    std::vector<std::vector<size_t>>       m_BDNeighborLists; ///>  store a list of boundary particle neighbors for each particle, updated each time step
    std::vector<std::vector<NeighborInfo>> m_NeighborInfo;    ///>  store a list of Vec4r(Vec3r(relative position), density) for neighbors, including boundary particle
};
} // end namespace imstk
