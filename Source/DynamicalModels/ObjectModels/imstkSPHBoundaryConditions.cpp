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

#include "imstkSPHBoundaryConditions.h"


namespace imstk
{
SPHBoundaryConditions::SPHBoundaryConditions(const std::pair<Vec3d, Vec3d>& inletDomain, const Vec3d& inletNormal, const std::pair<Vec3d, Vec3d>& outletDomain,
  const double inletFlowRate, StdVectorOfVec3d& mainParticlePositions, const StdVectorOfVec3d& wallParticlePositions):
  m_inletDomain(inletDomain), m_outletDomain(outletDomain)
{
    m_bufferCoord = Vec3d(100, 0, 0);
    m_inletCenterPoint = (inletDomain.first + inletDomain.second) / 2;
    m_inletRadius = Vec2d(inletDomain.first.y() - inletDomain.second.y(), inletDomain.first.z() - inletDomain.second.z()).norm() / 2;
    m_inletCrossSectionalArea = PI * m_inletRadius * m_inletRadius;
    m_inletNormal = inletNormal.normalized();

    setInletVelocity(inletFlowRate);

    setParticleTypes(mainParticlePositions, wallParticlePositions.size());

    addBoundaryParticles(mainParticlePositions, wallParticlePositions);
}

bool SPHBoundaryConditions::isInInletDomain(Vec3d position)
{
    if (position.x() >= m_inletDomain.first.x() && position.y() >= m_inletDomain.first.y() && position.z() >= m_inletDomain.first.z() &&
      position.x() <= m_inletDomain.second.x() && position.y() <= m_inletDomain.second.y() && position.z() <= m_inletDomain.second.z())
    {
        return true;
    }

    return false;
}

bool SPHBoundaryConditions::isInOutletDomain(Vec3d position)
{
    if (position.x() >= m_outletDomain.first.x() && position.y() >= m_outletDomain.first.y() && position.z() >= m_outletDomain.first.z() &&
      position.x() <= m_outletDomain.second.x() && position.y() <= m_outletDomain.second.y() && position.z() <= m_outletDomain.second.z())
    {
        return true;
    }

    return false;
}

///
/// \brief set particle type (fluid, wall, inlet, outlet, buffer)
///
void SPHBoundaryConditions::setParticleTypes(const StdVectorOfVec3d& mainParticlePositions, const size_t numWallParticles)
{
    //CHECK(particleTypes.size() != m_KinematicState->getNumParticles()) << "Incorrect number of particle types";

    m_particleTypes.reserve(mainParticlePositions.size() + numWallParticles + m_numBufferParticles);

    for (auto const& i : mainParticlePositions)
    {
        ParticleType type;
        if (isInInletDomain(i))
        {
            type = ParticleType::inlet;
        }
        else if (isInOutletDomain(i))
        {
            type = ParticleType::outlet;
        }
        else
        {
            type = ParticleType::fluid;
        }
        m_particleTypes.push_back(type);
    }

    m_particleTypes.insert(m_particleTypes.end(), numWallParticles, ParticleType::wall);
    m_particleTypes.insert(m_particleTypes.end(), m_numBufferParticles, ParticleType::buffer);
}

Vec3r SPHBoundaryConditions::computeParabolicInletVelocity(const Vec3d& particlePosition)
{
    // compute distance of point
    const Vec3d inletRegionCenterPoint = Vec3d(particlePosition.x(), m_inletCenterPoint.y(), m_inletCenterPoint.z());
    const double distance = (particlePosition - inletRegionCenterPoint).norm();
    const double inletParabolicVelocityX = m_inletVelocity.x() * (1 - (distance / m_inletRadius) * (distance / m_inletRadius));
    return Vec3r(inletParabolicVelocityX, 0, 0);
}

void SPHBoundaryConditions::addBoundaryParticles(StdVectorOfVec3d& mainParticlePositions, const StdVectorOfVec3d& wallParticlePositions)
{
    mainParticlePositions.insert(mainParticlePositions.end(), wallParticlePositions.begin(), wallParticlePositions.end());
    mainParticlePositions.insert(mainParticlePositions.end(), m_numBufferParticles, Vec3d(100, 0, 0));
}

void SPHBoundaryConditions::setInletVelocity(const double flowRate)
{
    m_inletVelocity = m_inletNormal * (flowRate / m_inletCrossSectionalArea * 2);
}
} // end namespace imstk
