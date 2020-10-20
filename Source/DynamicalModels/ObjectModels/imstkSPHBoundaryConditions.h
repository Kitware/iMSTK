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
///
/// \class SPHBoundaryConditions
/// \brief Class that holds methods and members for boundary conditions
///
class SPHBoundaryConditions
{
public:

    enum class ParticleType
    {
        fluid,
        wall,
        inlet,
        outlet,
        buffer
    };

    SPHBoundaryConditions(std::pair<Vec3d, Vec3d>& inletCoords, std::vector<std::pair<Vec3d, Vec3d>>& outletCoords, std::pair<Vec3d, Vec3d>& fluidCoords,
                          const Vec3d& inletNormal, const StdVectorOfVec3d& outletNormals, const Real inletRadius, const Vec3d& inletCenterPt, const double inletFlowRate,
                          StdVectorOfVec3d& mainParticlePositions,
                          const StdVectorOfVec3d& wallParticlePositions);

    bool isInInletDomain(const Vec3d& position);

    bool isInOutletDomain(const Vec3d& position);

    bool isInFluidDomain(const Vec3d& position);

    void setParticleTypes(const StdVectorOfVec3d& mainParticlePositions, const size_t numWallParticles);

    std::vector<ParticleType>& getParticleTypes() { return m_particleTypes; }

    Vec3d getBufferCoord() { return m_bufferCoord; }

    Vec3r computeParabolicInletVelocity(const Vec3d& position);

    void addBoundaryParticles(StdVectorOfVec3d& mainParticlePositions, const StdVectorOfVec3d& wallParticlePositions);

    void setInletVelocity(const Real flowRate);

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

    Vec3d m_bufferCoord;
    Vec3d m_inletCenterPoint;
    Real  m_inletRadius;
    Vec3r m_inletVelocity;
    Vec3d m_inletNormal;

    Real m_inletCrossSectionalArea;

    const size_t m_numBufferParticles = 10000;
    std::vector<size_t> m_bufferIndices;
};
} // end namespace imstk
