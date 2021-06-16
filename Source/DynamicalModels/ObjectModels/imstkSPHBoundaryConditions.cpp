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

#include <numeric>

namespace imstk
{
SPHBoundaryConditions::SPHBoundaryConditions(std::pair<Vec3d, Vec3d>& inletCoords, std::vector<std::pair<Vec3d, Vec3d>>& outletCoords, std::pair<Vec3d, Vec3d>& fluidCoords,
                                             const Vec3d& inletNormal, const StdVectorOfVec3d&, const Real inletRadius, const Vec3d& inletCenterPt, const double inletFlowRate,
                                             StdVectorOfVec3d& mainParticlePositions, const StdVectorOfVec3d& wallParticlePositions) :
    m_inletDomain(inletCoords), m_outletDomain(outletCoords),
    m_fluidDomain(fluidCoords),
    m_bufferCoord(Vec3d(100, 0, 0)),
    m_inletCenterPoint(inletCenterPt),
    m_inletRadius(inletRadius),
    m_inletNormal(inletNormal.normalized()),
    m_inletCrossSectionalArea(PI * m_inletRadius * m_inletRadius)
{
    setInletVelocity(inletFlowRate);
    setParticleTypes(mainParticlePositions, wallParticlePositions.size());

    addBoundaryParticles(mainParticlePositions, wallParticlePositions);
}

bool
SPHBoundaryConditions::isInInletDomain(const Vec3d& position)
{
    if (position.x() >= m_inletDomain.first.x() && position.y() >= m_inletDomain.first.y() && position.z() >= m_inletDomain.first.z()
        && position.x() <= m_inletDomain.second.x() && position.y() <= m_inletDomain.second.y() && position.z() <= m_inletDomain.second.z())
    {
        return true;
    }

    return false;
}

bool
SPHBoundaryConditions::isInOutletDomain(const Vec3d& position)
{
    for (const auto& i : m_outletDomain)
    {
        if (position.x() >= i.first.x() && position.y() >= i.first.y() && position.z() >= i.first.z()
            && position.x() <= i.second.x() && position.y() <= i.second.y() && position.z() <= i.second.z())
        {
            return true;
        }
    }

    return false;
}

bool
SPHBoundaryConditions::isInFluidDomain(const Vec3d& position)
{
    const double error = 0.1;
    if (position.x() >= m_fluidDomain.first.x() - error && position.y() >= m_fluidDomain.first.y() - error && position.z() >= m_fluidDomain.first.z() - error
        && position.x() <= m_fluidDomain.second.x() + error && position.y() <= m_fluidDomain.second.y() + error && position.z() <= m_fluidDomain.second.z() + error)
    {
        return true;
    }

    return false;
}

///
/// \brief set particle type (fluid, wall, inlet, outlet, buffer)
///
void
SPHBoundaryConditions::setParticleTypes(const StdVectorOfVec3d& mainParticlePositions, const size_t numWallParticles)
{
    m_particleTypes.reserve(mainParticlePositions.size() + numWallParticles + m_numBufferParticles);

    for (auto const& i : mainParticlePositions)
    {
        ParticleType type;
        if (isInInletDomain(i))
        {
            type = ParticleType::Inlet;
        }
        else if (isInOutletDomain(i))
        {
            type = ParticleType::Outlet;
        }
        else
        {
            type = ParticleType::Fluid;
        }
        m_particleTypes.push_back(type);
    }

    m_particleTypes.insert(m_particleTypes.end(), numWallParticles, ParticleType::Wall);
    m_particleTypes.insert(m_particleTypes.end(), m_numBufferParticles, ParticleType::Buffer);
    m_bufferIndices.resize(m_numBufferParticles);
    std::iota(std::begin(m_bufferIndices), std::end(m_bufferIndices), m_particleTypes.size() - m_numBufferParticles);
}

Vec3r
SPHBoundaryConditions::computeParabolicInletVelocity(const Vec3d& particlePosition)
{
    // compute distance of point
    const Vec3d  inletRegionCenterPoint = (Vec3d(1, 1, 1) + m_inletNormal).array() * m_inletCenterPoint.array() + particlePosition.dot(m_inletNormal) * m_inletNormal.array();
    const double distance = (particlePosition - inletRegionCenterPoint).norm();
    Vec3d        inletParabolicVelocity;
    if (distance > m_inletRadius)
    {
        inletParabolicVelocity = Vec3d(0, 0, 0);
    }
    else
    {
        inletParabolicVelocity = m_inletVelocity * (1 - (distance / m_inletRadius) * (distance / m_inletRadius));
    }
    return inletParabolicVelocity;
}

void
SPHBoundaryConditions::addBoundaryParticles(StdVectorOfVec3d& mainParticlePositions, const StdVectorOfVec3d& wallParticlePositions)
{
    mainParticlePositions.insert(mainParticlePositions.end(), wallParticlePositions.begin(), wallParticlePositions.end());
    mainParticlePositions.insert(mainParticlePositions.end(), m_numBufferParticles, Vec3d(100, 0, 0));
}

void
SPHBoundaryConditions::setInletVelocity(const Real flowRate)
{
    m_inletVelocity = -m_inletNormal * (flowRate / m_inletCrossSectionalArea * 2);
}

Vec3d
SPHBoundaryConditions::placeParticleAtInlet(const Vec3d& position)
{
    const Vec3d inletPosition = (Vec3d(1, 1, 1) + m_inletNormal).cwiseProduct(position) - m_inletCenterPoint.cwiseProduct(m_inletNormal);
    return inletPosition;
}
} // end namespace imstk
