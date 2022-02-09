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

#include "imstkSphState.h"
#include "imstkLogger.h"
#include "imstkVecDataArray.h"

namespace imstk
{
SphState::SphState(const int numElements) :
    m_positions(std::make_shared<VecDataArray<double, 3>>(numElements)),
    m_fullStepVelocities(std::make_shared<VecDataArray<double, 3>>(numElements)),
    m_halfStepVelocities(std::make_shared<VecDataArray<double, 3>>(numElements)),
    m_velocities(std::make_shared<VecDataArray<double, 3>>(numElements)),
    m_boundaryParticlePositions(std::make_shared<VecDataArray<double, 3>>()),
    m_densities(std::make_shared<DataArray<double>>(numElements)),
    m_normals(std::make_shared<VecDataArray<double, 3>>(numElements)),
    m_acceleration(std::make_shared<VecDataArray<double, 3>>(numElements)),
    m_diffuseVelocities(std::make_shared<VecDataArray<double, 3>>(numElements))
{
    std::fill_n(m_densities->getPointer(), m_densities->size(), 1.0);
    std::fill_n(m_acceleration->getPointer(), m_acceleration->size(), Vec3d(0, 0, 0));
    std::fill_n(m_diffuseVelocities->getPointer(), m_diffuseVelocities->size(), Vec3d(0, 0, 0));
    std::fill_n(m_velocities->getPointer(), m_velocities->size(), Vec3d(0.0, 0.0, 0.0));
    std::fill_n(m_halfStepVelocities->getPointer(), m_halfStepVelocities->size(), Vec3d(0.0, 0.0, 0.0));
    std::fill_n(m_fullStepVelocities->getPointer(), m_fullStepVelocities->size(), Vec3d(0.0, 0.0, 0.0));

    m_neighborInfo.resize(static_cast<size_t>(numElements));
    m_neighborLists.resize(static_cast<size_t>(numElements));
}

void
SphState::setState(std::shared_ptr<SphState> rhs)
{
    *m_positions  = *rhs->getPositions();
    *m_velocities = *rhs->getVelocities();
    *m_halfStepVelocities = *rhs->getHalfStepVelocities();
    *m_fullStepVelocities = *rhs->getFullStepVelocities();
    *m_boundaryParticlePositions = *rhs->getBoundaryParticlePositions();
    *m_densities         = *rhs->getDensities();
    *m_normals           = *rhs->getNormals();
    *m_acceleration      = *rhs->getAccelerations();
    *m_diffuseVelocities = *rhs->getDiffuseVelocities();

    m_neighborLists = rhs->getFluidNeighborLists();
    m_boundaryParticleNeighborLists = rhs->getBoundaryNeighborLists();
    m_neighborInfo = rhs->getNeighborInfo();

    m_positions->postModified();
}

size_t
SphState::getNumParticles() const
{
    return m_positions->size();
}
} // namespace imstk