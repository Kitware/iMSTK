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
    Vec3d relativePos;      ///> relative position
    double density;         ///> density of neighbor particle
};

///
/// \class SphSimulationState
/// \brief Simulation states of SPH particles
///
class SphState
{
public:
    ///
    /// \brief Default constructor/destructor
    ///
    SphState(const int numElements);
    virtual ~SphState() = default;

public:
    ///
    /// \brief Set positions of the boundary (solid) particles
    ///
    void setBoundaryParticlePositions(std::shared_ptr<VecDataArray<double, 3>> positions) { m_boundaryParticlePositions = positions; }

    ///
    /// \brief Get number of particles
    ///
    size_t getNumParticles() const;

    ///
    /// \brief Returns the vector of all particle positions
    ///
    std::shared_ptr<VecDataArray<double, 3>> getPositions() const { return m_positions; }
    void setPositions(std::shared_ptr<VecDataArray<double, 3>> positions) { m_positions = positions; }

    ///
    /// \brief Returns the vector of all particle velocities
    ///
    std::shared_ptr<VecDataArray<double, 3>> getFullStepVelocities() const { return m_fullStepVelocities; }

    ///
    /// \brief Returns the vector of all particle velocities
    ///
    std::shared_ptr<VecDataArray<double, 3>> getHalfStepVelocities() const { return m_halfStepVelocities; }

    ///
    /// \brief Returns the vector of all particle velocities
    ///
    std::shared_ptr<VecDataArray<double, 3>> getVelocities() const { return m_velocities; }
    void setVelocities(std::shared_ptr<VecDataArray<double, 3>> velocities) { m_velocities = velocities; }

    ///
    /// \brief Returns the vector of all particle positions
    ///
    std::shared_ptr<VecDataArray<double, 3>> getBoundaryParticlePositions() const { return m_boundaryParticlePositions; }

    ///
    /// \brief Returns the vector of all particle surface normals
    ///
    std::shared_ptr<VecDataArray<double, 3>> getNormals() const { return m_normals; }

    ///
    /// \brief Returns the vector of all particle densities
    ///
    std::shared_ptr<DataArray<double>> getDensities() const { return m_densities; }

    ///
    /// \brief Returns the vector of all particle accelerations
    ///
    std::shared_ptr<VecDataArray<double, 3>> getAccelerations() const { return m_acceleration; }

    ///
    /// \brief Returns the vector of all velocity diffusion
    ///
    std::shared_ptr<VecDataArray<double, 3>> getDiffuseVelocities() const { return m_diffuseVelocities; }

    ///
    /// \brief Returns the vector of neighbor fluid particles
    ///
    std::vector<std::vector<size_t>>& getFluidNeighborLists() { return m_neighborLists; }
    const std::vector<std::vector<size_t>>& getFluidNeighborLists() const { return m_neighborLists; }

    ///
    /// \brief Returns the vector of neighbor of boundary particles
    ///
    std::vector<std::vector<size_t>>& getBoundaryNeighborLists() { return m_boundaryParticleNeighborLists; }
    const std::vector<std::vector<size_t>>& getBoundaryNeighborLists() const { return m_boundaryParticleNeighborLists; }

    ///
    /// \brief Returns the vector of neighbor information ( {relative position, density} ), which is cached for other computation
    ///
    std::vector<std::vector<NeighborInfo>>& getNeighborInfo() { return m_neighborInfo; }
    const std::vector<std::vector<NeighborInfo>>& getNeighborInfo() const { return m_neighborInfo; }

    ///
    /// \brief Set the state to a given one
    ///
    void setState(std::shared_ptr<SphState> rhs);

private:
    std::shared_ptr<VecDataArray<double, 3>> m_positions;
    std::shared_ptr<VecDataArray<double, 3>> m_fullStepVelocities;
    std::shared_ptr<VecDataArray<double, 3>> m_halfStepVelocities;
    std::shared_ptr<VecDataArray<double, 3>> m_velocities;

    std::shared_ptr<VecDataArray<double, 3>> m_boundaryParticlePositions;   ///> positions of boundary particles, if generated

    std::shared_ptr<DataArray<double>>       m_densities;                   ///>  particle densities
    std::shared_ptr<VecDataArray<double, 3>> m_normals;                     ///>  surface normals
    std::shared_ptr<VecDataArray<double, 3>> m_acceleration;                ///>  acceleration
    std::shared_ptr<VecDataArray<double, 3>> m_diffuseVelocities;           ///>  velocity diffusion, used for computing viscosity

    std::vector<std::vector<size_t>>       m_neighborLists;                 ///>  store a list of neighbors for each particle, updated each time step
    std::vector<std::vector<size_t>>       m_boundaryParticleNeighborLists; ///>  store a list of boundary particle neighbors for each particle, updated each time step
    std::vector<std::vector<NeighborInfo>> m_neighborInfo;                  ///>  store a list of Vec4d(Vec3d(relative position), density) for neighbors, including boundary particle
};
} // end namespace imstk
