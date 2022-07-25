/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkMath.h"

namespace imstk
{
///
/// \class SPHBoundaryConditions
///
/// \brief Class that holds methods and members for boundary conditions
///
class SphBoundaryConditions
{
public:
    enum class ParticleType
    {
        Fluid,
        Wall,
        Inlet,
        Outlet,
        Buffer
    };

public:
    SphBoundaryConditions(std::pair<Vec3d, Vec3d>& inletCoords, std::vector<std::pair<Vec3d, Vec3d>>& outletCoords, std::pair<Vec3d, Vec3d>& fluidCoords,
                          const Vec3d& inletNormal, const StdVectorOfVec3d& outletNormals, const double inletRadius, const Vec3d& inletCenterPt, const double inletFlowRate,
                          StdVectorOfVec3d& mainParticlePositions,
                          const StdVectorOfVec3d& wallParticlePositions);

public:
    bool isInInletDomain(const Vec3d& position);

    bool isInOutletDomain(const Vec3d& position);

    bool isInFluidDomain(const Vec3d& position);

    void setParticleTypes(const StdVectorOfVec3d& mainParticlePositions, const size_t numWallParticles);

    std::vector<ParticleType>& getParticleTypes() { return m_particleTypes; }

    Vec3d getBufferCoord() { return m_bufferCoord; }

    Vec3d computeParabolicInletVelocity(const Vec3d& position);

    void addBoundaryParticles(StdVectorOfVec3d& mainParticlePositions, const StdVectorOfVec3d& wallParticlePositions);

    void setInletVelocity(const double flowRate);

    Vec3d getInletCoord() { return m_inletDomain.first; }

    Vec3d getInletVelocity() { return m_inletVelocity; }

    Vec3d getInletNormal() { return m_inletNormal; }

    std::vector<size_t>& getBufferIndices() { return m_bufferIndices; }

    Vec3d placeParticleAtInlet(const Vec3d& position);

private:
    std::pair<Vec3d, Vec3d> m_inletDomain;
    std::vector<std::pair<Vec3d, Vec3d>> m_outletDomain;
    std::pair<Vec3d, Vec3d> m_fluidDomain;

    std::vector<ParticleType> m_particleTypes;

    Vec3d  m_bufferCoord;
    Vec3d  m_inletCenterPoint;
    double m_inletRadius;
    Vec3d  m_inletVelocity;
    Vec3d  m_inletNormal;

    double m_inletCrossSectionalArea;

    const size_t m_numBufferParticles = 10000;
    std::vector<size_t> m_bufferIndices;
};
} // namespace imstk