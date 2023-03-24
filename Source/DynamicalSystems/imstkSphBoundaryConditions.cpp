/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkSphBoundaryConditions.h"

#include <numeric>

namespace imstk
{
SphBoundaryConditions::SphBoundaryConditions(std::pair<Vec3d, Vec3d>& inletCoords, std::vector<std::pair<Vec3d, Vec3d>>& outletCoords, std::pair<Vec3d, Vec3d>& fluidCoords,
                                             const Vec3d& inletNormal, const StdVectorOfVec3d&, const double inletRadius, const Vec3d& inletCenterPt, const double inletFlowRate,
                                             StdVectorOfVec3d& mainParticlePositions, const StdVectorOfVec3d& wallParticlePositions) :
    m_inletDomain(inletCoords), m_outletDomain(outletCoords),
    m_fluidDomain(fluidCoords),
    m_bufferCoord(Vec3d(100.0, 0.0, 0.0)),
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
SphBoundaryConditions::isInInletDomain(const Vec3d& position)
{
    if (position.x() >= m_inletDomain.first.x() && position.y() >= m_inletDomain.first.y() && position.z() >= m_inletDomain.first.z()
        && position.x() <= m_inletDomain.second.x() && position.y() <= m_inletDomain.second.y() && position.z() <= m_inletDomain.second.z())
    {
        return true;
    }

    return false;
}

bool
SphBoundaryConditions::isInOutletDomain(const Vec3d& position)
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
SphBoundaryConditions::isInFluidDomain(const Vec3d& position)
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
SphBoundaryConditions::setParticleTypes(const StdVectorOfVec3d& mainParticlePositions, const size_t numWallParticles)
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

Vec3d
SphBoundaryConditions::computeParabolicInletVelocity(const Vec3d& particlePosition)
{
    // compute distance of point
    const Vec3d  inletRegionCenterPoint = (Vec3d(1.0, 1.0, 1.0) + m_inletNormal).array() * m_inletCenterPoint.array() + particlePosition.dot(m_inletNormal) * m_inletNormal.array();
    const double distance = (particlePosition - inletRegionCenterPoint).norm();
    Vec3d        inletParabolicVelocity;
    if (distance > m_inletRadius)
    {
        inletParabolicVelocity = Vec3d::Zero();
    }
    else
    {
        inletParabolicVelocity = m_inletVelocity * (1.0 - (distance / m_inletRadius) * (distance / m_inletRadius));
    }
    return inletParabolicVelocity;
}

void
SphBoundaryConditions::addBoundaryParticles(StdVectorOfVec3d& mainParticlePositions, const StdVectorOfVec3d& wallParticlePositions)
{
    mainParticlePositions.insert(mainParticlePositions.end(), wallParticlePositions.begin(), wallParticlePositions.end());
    mainParticlePositions.insert(mainParticlePositions.end(), m_numBufferParticles, Vec3d(100.0, 0.0, 0.0));
}

void
SphBoundaryConditions::setInletVelocity(const double flowRate)
{
    m_inletVelocity = -m_inletNormal * (flowRate / m_inletCrossSectionalArea * 2.0);
}

Vec3d
SphBoundaryConditions::placeParticleAtInlet(const Vec3d& position)
{
    const Vec3d inletPosition = (Vec3d(1.0, 1.0, 1.0) + m_inletNormal).cwiseProduct(position) - m_inletCenterPoint.cwiseProduct(m_inletNormal);
    return inletPosition;
}
} // namespace imstk