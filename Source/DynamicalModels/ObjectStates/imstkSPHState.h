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

#include <Eigen/Dense>
#include <vector>

namespace imstk
{
///
/// \brief This function is for later refactoring, we can replace the regular for loop with a parallel_for loop
///
template<class IndexType, class Function>
void runLoop(IndexType start, IndexType end, Function&& func)
{
    for(IndexType i = start; i < end; ++i)
    {
        func(i);
    }
}

///
/// \brief This function is for later refactoring, we can replace the regular for loop with a parallel_for loop
///
template<class IndexType, class Function>
void runLoop(IndexType size, Function&& func)
{
    runLoop<IndexType, Function>(0, size, std::forward<Function>(func));
}

///
/// \class SPHKinematicState
///
/// \brief State of the SPH fluid particles
///
template<class Real>
class SPHKinematicState
{
using Vec3r = Eigen::Matrix<Real, 3, 1>;
using StdVT_Vec3r = std::vector<Vec3r, Eigen::aligned_allocator<Vec3r>>;

public:
    ///
    /// \brief Default constructor/destructor
    ///
    SPHKinematicState() = default;
    virtual ~SPHKinematicState() = default;

    ///
    /// \brief Set particle data with given positions and velocities
    ///
    void setParticleData(const StdVT_Vec3r& positions, const StdVT_Vec3r& velocities = {});

    ///
    /// \brief Get number of particles
    ///
    size_t size() const { return m_Positions.size(); }

    ///
    /// \brief Returns the vector of all particle positions
    ///
    auto& getPositions() { return m_Positions; }
    const auto& getPositions() const { return m_Positions; }

    ///
    /// \brief Returns the vector of all particle velocities
    ///
    auto& getVelocities() { return m_Velocities; }
    const auto& getVelocities() const { return m_Velocities; }

    ///
    /// \brief Set the state to a given one
    ///
    void setState(const std::shared_ptr<SPHKinematicState<Real>>& rhs);

private:
    StdVT_Vec3r m_Positions;   // Particle positions
    StdVT_Vec3r m_Velocities;  // Particle velocities
};


///
/// \class SPHSimulationState
///
/// \brief Simulation states of SPH particles
///
template<class Real>
class SPHSimulationState
{
using Vec3r = Eigen::Matrix<Real, 3, 1>;
using Vec4r = Eigen::Matrix<Real, 4, 1>;

using StdVT_Vec3r = std::vector<Vec3r, Eigen::aligned_allocator<Vec3r>>;
using StdVT_Real  = std::vector<Real>;

public:
    ///
    /// \brief Default constructor/destructor
    ///
    SPHSimulationState() = default;
    virtual ~SPHSimulationState() = default;

    ///
    /// \brief Set the kinematic state: positions and velocities
    ///
    void setKinematicState(const std::shared_ptr<SPHKinematicState<Real>>& state) { m_KinematicState = state; }

    ///
    /// \brief Set positions of the boundary (solid) particles
    ///
    void setBoundaryParticlePositions(const StdVT_Vec3r& positions) { m_BDPositions = positions; }

    ///
    /// \brief Initialize simulation variables, must be called after setKinematicState and (if applicable) after setBoundaryParticlePositions
    ///
    void initializeData();

    ///
    /// \brief Get number of particles
    ///
    size_t size() const { assert(m_KinematicState); return m_KinematicState->size(); }

    ///
    /// \brief Returns the vector of all particle positions
    ///
    auto& getPositions() { assert(m_KinematicState); return m_KinematicState->getPositions(); }
    const auto& getPositions() const { assert(m_KinematicState); return m_KinematicState->getPositions(); }

    ///
    /// \brief Returns the vector of all particle velocities
    ///
    auto& getVelocities() { assert(m_KinematicState); return m_KinematicState->getVelocities(); }
    const auto& getVelocities() const { assert(m_KinematicState); return m_KinematicState->getVelocities(); }

    ///
    /// \brief Returns the vector of all particle positions
    ///
    auto& getBoundaryParticlePositions() { return m_BDPositions; }
    const auto& getBoundaryParticlePositions() const { return m_BDPositions; }

    ///
    /// \brief Returns the vector of all particle surface normals
    ///
    auto& getNormals() { return m_Normals; }
    const auto& getNormals() const { return m_Normals; }

    ///
    /// \brief Returns the vector of all particle densities
    ///
    auto& getDensities() { return m_Densities; }
    const auto& getDensities() const { return m_Densities; }

    ///
    /// \brief Returns the vector of all particle densities
    ///
    auto& getNormalizedDensities() { return m_NormalizedDensities; }
    const auto& getNormalizedDensities() const { return m_NormalizedDensities; }

    ///
    /// \brief Returns the vector of all particle accelerations
    ///
    auto& getAccelerations() { return m_Accels; }
    const auto& getAccelerations() const { return m_Accels; }

    ///
    /// \brief Returns the vector of all velocity diffusion
    ///
    auto& getDiffuseVelocities() { return m_DiffuseVelocities; }
    const auto& getDiffuseVelocities() const { return m_DiffuseVelocities; }

    ///
    /// \brief Returns the vector of neighbor fluid particles
    ///
    auto& getFluidNeighborLists() { return m_NeighborLists; }
    const auto& getFluidNeighborLists() const { return m_NeighborLists; }

    ///
    /// \brief Returns the vector of neighbor of boundary particles
    ///
    auto& getBoundaryNeighborLists() { return m_BDNeighborLists; }
    const auto& getBoundaryNeighborLists() const { return m_BDNeighborLists; }

    ///
    /// \brief Returns the vector of neighbor information ( {relative position, density} ), which is cached for other computation
    ///
    auto& getNeighborInfo() { return m_NeighborInfo; }
    const auto& getNeighborInfo() const { return m_NeighborInfo; }

private:
    std::shared_ptr<SPHKinematicState<Real>> m_KinematicState; // basic state: positions + velocities
    StdVT_Vec3r m_BDPositions; // positions of boundary particles, if generated

    StdVT_Real m_Densities;           // particle densities
    StdVT_Real m_NormalizedDensities; // variable for normalizing densities
    StdVT_Vec3r m_Normals;            // surface normals
    StdVT_Vec3r m_Accels;             // acceleration
    StdVT_Vec3r m_DiffuseVelocities;             // velocity diffusion, used for computing viscosity
    std::vector<std::vector<size_t>> m_NeighborLists;   // store a list of neighbors for each particle, updated each time step
    std::vector<std::vector<size_t>> m_BDNeighborLists; // store a list of boundary particle neighbors for each particle, updated each time step

    struct NeighborInfo
    {
        Vec3r xpq; // relative position: xpq = x_p - x_q
        Real density; // density of neighbor particle q
    };
    std::vector<std::vector<NeighborInfo>>  m_NeighborInfo;    // store a list of Vec4r(Vec3r(relative position), density) for neighbors, including boundary particle
};
} // end namespace imstk
