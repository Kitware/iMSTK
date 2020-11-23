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
template<typename T> class DataArray;
template<typename T, int N> class VecDataArray;

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
class SPHState
{
public:
    ///
    /// \brief Default constructor/destructor
    ///
    SPHState();
    virtual ~SPHState() = default;

public:
    ///
    /// \brief Set particle data with given positions and velocities
    ///
    void setParticleData(std::shared_ptr<VecDataArray<double, 3>> positions, std::shared_ptr<VecDataArray<double, 3>> velocities);

    ///
    /// \brief Set positions of the boundary (solid) particles
    ///
    void setBoundaryParticlePositions(std::shared_ptr<VecDataArray<double, 3>> positions) { m_BDPositions = positions; }

    ///
    /// \brief Initialize simulation variables, must be called after setKinematicState and (if applicable)
    /// after setBoundaryParticlePositions
    ///
    void initializeData();

    ///
    /// \brief Get number of particles
    ///
    size_t getNumParticles() const;

    ///
    /// \brief Returns the vector of all particle positions
    ///
    std::shared_ptr<VecDataArray<double, 3>> getPositions() { return m_positions; }
    void setPositions(std::shared_ptr<VecDataArray<double, 3>> positions) { m_positions = positions; }

    ///
    /// \brief Returns the vector of all particle velocities
    ///
    std::shared_ptr<VecDataArray<double, 3>> getFullStepVelocities() { return m_fullStepVelocities; }

    ///
    /// \brief Returns the vector of all particle velocities
    ///
    std::shared_ptr<VecDataArray<double, 3>> getHalfStepVelocities() { return m_halfStepVelocities; }

    ///
    /// \brief Returns the vector of all particle velocities
    ///
    std::shared_ptr<VecDataArray<double, 3>> getVelocities() { return m_velocities; }

    ///
    /// \brief Returns the vector of all particle positions
    ///
    std::shared_ptr<VecDataArray<double, 3>> getBoundaryParticlePositions() { return m_BDPositions; }

    ///
    /// \brief Returns the vector of all particle surface normals
    ///
    std::shared_ptr<VecDataArray<double, 3>> getNormals() { return m_Normals; }

    ///
    /// \brief Returns the vector of all particle densities
    ///
    std::shared_ptr<DataArray<double>> getDensities() { return m_Densities; }

    ///
    /// \brief Returns the vector of all particle accelerations
    ///
    std::shared_ptr<VecDataArray<double, 3>> getAccelerations() { return m_Accels; }

    ///
    /// \brief Returns the vector of all velocity diffusion
    ///
    std::shared_ptr<VecDataArray<double, 3>> getDiffuseVelocities() { return m_DiffuseVelocities; }

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

    ///
    /// \brief Set the state to a given one
    ///
    void setState(std::shared_ptr<SPHState> rhs);

private:
    std::shared_ptr<VecDataArray<double, 3>> m_positions;
    std::shared_ptr<VecDataArray<double, 3>> m_fullStepVelocities;
    std::shared_ptr<VecDataArray<double, 3>> m_halfStepVelocities;
    std::shared_ptr<VecDataArray<double, 3>> m_velocities;

    std::shared_ptr<VecDataArray<double, 3>> m_BDPositions;       ///> positions of boundary particles, if generated

    std::shared_ptr<DataArray<double>>       m_Densities;         ///>  particle densities
    std::shared_ptr<VecDataArray<double, 3>> m_Normals;           ///>  surface normals
    std::shared_ptr<VecDataArray<double, 3>> m_Accels;            ///>  acceleration
    std::shared_ptr<VecDataArray<double, 3>> m_DiffuseVelocities; ///>  velocity diffusion, used for computing viscosity
    std::vector<std::vector<size_t>>       m_NeighborLists;       ///>  store a list of neighbors for each particle, updated each time step
    std::vector<std::vector<size_t>>       m_BDNeighborLists;     ///>  store a list of boundary particle neighbors for each particle, updated each time step
    std::vector<std::vector<NeighborInfo>> m_NeighborInfo;        ///>  store a list of Vec4r(Vec3r(relative position), density) for neighbors, including boundary particle
};
} // end namespace imstk
