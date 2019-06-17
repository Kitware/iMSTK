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

#include "imstkSPHModel.h"
#include <g3log/g3log.hpp>

namespace imstk
{
// SPHModelConfig implementation ===>

SPHModelConfig::SPHModelConfig(const Real particleRadius)
{
    m_ParticleRadius = particleRadius;
    initialize();
}


void SPHModelConfig::initialize()
{
    LOG_IF(FATAL, (std::abs(m_ParticleRadius) < Real(1e-6))) << "Particle radius was not set properly";

    m_ParticleRadiusSqr = m_ParticleRadius * m_ParticleRadius;

    m_ParticleMass   = Real(std::pow(Real(2.0) * m_ParticleRadius, 3)) * m_RestDensity * m_ParticleMassScale;
    m_RestDensitySqr = m_RestDensity * m_RestDensity;
    m_RestDensityInv = Real(1) / m_RestDensity;

    m_KernelRadius    = m_ParticleRadius * m_RatioKernelOverParticleRadius;
    m_KernelRadiusSqr = m_KernelRadius * m_KernelRadius;
}

// SPHModel implementation ===>

bool SPHModel::initialize()
{
    LOG_IF(FATAL, !m_Geometry) << "Model geometry is not yet set! Cannot initialize without model geometry.";

    // initialize positions and velocity of the particles
    this->m_initialState = std::make_shared<SPHKinematicState>();
    this->m_currentState = std::make_shared<SPHKinematicState>();

    // Set particle positions and zero default velocities
    // TODO: set particle data with given (non-zero) velocities
    this->m_initialState->setParticleData(m_Geometry->getVertexPositions());
    this->m_currentState->setState(this->m_initialState);

    // attach current state to simulation state
    m_SimulationState.setKinematicState(this->m_currentState);

    // initialize (allocate memory for) simulation data such as density, acceleration etc.
    m_SimulationState.initializeData();

    // initialize simulation dependent parameters and kernel data
    m_Kernels.initialize(m_Parameters->m_KernelRadius);

    // intialize neighbor search, only one method is used
    if(m_Parameters->m_NeighborSearchMethod == NeighborSearchMethod::GridBased)
    {
        m_NeighborSearch.gridSearch.setSearchRadius(m_Parameters->m_KernelRadius);
    }
    else
    {
        m_NeighborSearch.spatialHashSearch.setCellSize(m_Parameters->m_KernelRadius,
                                                       m_Parameters->m_KernelRadius,
                                                       m_Parameters->m_KernelRadius);
    }

    return true;
}


void SPHModel::simulationTimeStep()
{
    findParticleNeighbors();
    computeNeighborRelativePositions();
    computeDensity();
    normalizeDensity();
    collectNeighborDensity();
    computePressureAcceleration();
    computeSurfaceTension();
    computeTimeStepSize();
    updateVelocity(getTimeStep());
    computeViscosity();
    advect(getTimeStep());
}


void SPHModel::computeTimeStepSize()
{
    m_dt = (this->m_timeStepSizeType == TimeSteppingType::fixed) ? m_DefaultDt : computeCFLTimeStepSize();
}


Real SPHModel::computeCFLTimeStepSize()
{
    Real maxVel = 0;
    for(const auto& vel : getState().getVelocities())
    {
        auto mag2 = vel.squaredNorm();
        if(mag2 > maxVel)
        {
            maxVel = mag2;
        }
    }
    maxVel = std::sqrt(maxVel); //  maximum of || velocities ||

    // dt = CFL * 2r / max{|| v ||}
    Real timestep = maxVel > Real(1e-6) ? m_Parameters->m_CFLFactor * (Real(2.0) * m_Parameters->m_ParticleRadius / maxVel) : m_Parameters->m_MaxTimestep;

    // clamp the time step size to be within a given range
    if(timestep > m_Parameters->m_MaxTimestep)
    {
        timestep = m_Parameters->m_MaxTimestep;
    }
    else if(timestep < m_Parameters->m_MinTimestep)
    {
        timestep = m_Parameters->m_MinTimestep;
    }

    return timestep;
}


void SPHModel::findParticleNeighbors()
{
    if(m_Parameters->m_NeighborSearchMethod == NeighborSearchMethod::GridBased)
    {
        auto& searcher = m_NeighborSearch.gridSearch;
        searcher.getNeighbors(getState().getFluidNeighborLists(), getState().getPositions());

        if(m_Parameters->m_bDensityWithBoundary)   // if considering boundary particles for computing fluid density
        {
            searcher.getNeighbors(getState().getBoundaryNeighborLists(), getState().getPositions(), getState().getBoundaryParticlePositions());
        }
    }
    else
    {
        auto& searcher = m_NeighborSearch.spatialHashSearch;
        searcher.clear();
        searcher.insertPoints(getState().getPositions());

        runLoop(getState().size(),
            [&] (const size_t p) {
                const auto& ppos = getState().getPositions()[p];
                searcher.getPointsInSphere(getState().getFluidNeighborLists()[p], ppos, m_Parameters->m_KernelRadius);
            });

        if(m_Parameters->m_bDensityWithBoundary)   // if considering boundary particles for computing fluid density
        {
            searcher.clear();
            searcher.insertPoints(getState().getBoundaryParticlePositions());

            runLoop(getState().size(),
                [&] (const size_t p) {
                    const auto& ppos = getState().getPositions()[p];
                    searcher.getPointsInSphere(getState().getBoundaryNeighborLists()[p], ppos, m_Parameters->m_KernelRadius);
                });
        }
    }
}


void SPHModel::computeNeighborRelativePositions()
{
    auto computeRelativePositions = [&](const Vec3r& ppos, const std::vector<size_t>& neighborList,
                                        const StdVectorOfVec3r& allPositions, std::vector<NeighborInfo>& neighborInfo) {
                                        for(size_t q : neighborList)
                                        {
                                            const Vec3r& qpos = allPositions[q];
                                            const Vec3r r     = ppos - qpos;
                                            neighborInfo.push_back({ r, m_Parameters->m_RestDensity });
                                        }
                                    };
    ////////////////////////////////////////////////////////////////////////////////
    runLoop(getState().size(),
        [&] (const size_t p) {
            const auto& ppos   = getState().getPositions()[p];
            auto& neighborInfo = getState().getNeighborInfo()[p];
            neighborInfo.resize(0);
            neighborInfo.reserve(48);
            computeRelativePositions(ppos, getState().getFluidNeighborLists()[p], getState().getPositions(), neighborInfo);

            // if considering boundary particles then also cache relative positions with them
            if(m_Parameters->m_bDensityWithBoundary)
            {
                computeRelativePositions(ppos, getState().getBoundaryNeighborLists()[p], getState().getBoundaryParticlePositions(), neighborInfo);
            }
        });
}


void SPHModel::collectNeighborDensity()
{
    // after computing particle densities, cache them into neighborInfo variable, next to relative positions
    // this is usefull because relative positions and densities are accessed together multiple times
    // caching relative positions and densities therefore can reduce computation time significantly (tested)
    runLoop(getState().size(),
        [&] (const size_t p) {
            auto& neighborInfo = getState().getNeighborInfo()[p];
            assert(neighborInfo.size() >= 1);     // neighbor of particle p should contain at least p index
            if(neighborInfo.size() <= 1)
            {
                return; // the particle has no neighbor
            }

            const auto& fluidNeighborList = getState().getFluidNeighborLists()[p];
            for(size_t i = 0; i < fluidNeighborList.size(); ++i)
            {
                auto q = fluidNeighborList[i];
                neighborInfo[i].density = getState().getDensities()[q];
            }
        });
}


void SPHModel::computeDensity()
{
    runLoop(getState().size(),
        [&] (const size_t p) {
            const auto& neighborInfo = getState().getNeighborInfo()[p];
            assert(neighborInfo.size() >= 1);     // neighbor of particle p should contain at least p index
            if(neighborInfo.size() <= 1)
            {
                return; // the particle has no neighbor
            }

            Real pdensity = 0;
            for(const auto& qInfo : neighborInfo)
            {
                pdensity += m_Kernels.W(qInfo.xpq);
            }
            pdensity *= m_Parameters->m_ParticleMass;
            getState().getDensities()[p] = pdensity;
        });
}


void SPHModel::normalizeDensity()
{
    if(!m_Parameters->m_bNormalizeDensity)
    {
        return;
    }
    getState().getNormalizedDensities().resize(getState().size());
    ////////////////////////////////////////////////////////////////////////////////
    runLoop(getState().size(),
        [&] (const size_t p) {
            auto& neighborInfo = getState().getNeighborInfo()[p];
            assert(neighborInfo.size() >= 1);     // neighbor of particle p should contain at least p index
            if(neighborInfo.size() <= 1)
            {
                return; // the particle has no neighbor
            }

            const auto& fluidNeighborList = getState().getFluidNeighborLists()[p];
            Real tmp = 0;
            for(size_t i = 0; i < fluidNeighborList.size(); ++i)
            {
                const auto& qInfo = neighborInfo[i];

                // because we're not done with density computation, qInfo does not contain desity of particle q yet
                const auto q        = fluidNeighborList[i];
                const auto qdensity = getState().getDensities()[q];

                tmp += m_Kernels.W(qInfo.xpq) / qdensity;
            }

            if(m_Parameters->m_bDensityWithBoundary)
            {
                const auto& BDNeighborList = getState().getBoundaryNeighborLists()[p];
                assert(fluidNeighborList.size() + BDNeighborList.size() == neighborInfo.size());
                for(size_t i = fluidNeighborList.size(); i < neighborInfo.size(); ++i)
                {
                    const auto& qInfo = neighborInfo[i];
                    tmp += m_Kernels.W(qInfo.xpq) / m_Parameters->m_RestDensity;     // density of boundary particle is set to rest density
                }
            }

            getState().getNormalizedDensities()[p] = getState().getDensities()[p] / (tmp * m_Parameters->m_ParticleMass);
        });

    // put normalized densities to densities
    std::swap(getState().getDensities(), getState().getNormalizedDensities());
}


void SPHModel::computePressureAcceleration()
{
    auto particlePressure = [&](Real density) {
                                Real error = std::pow(density / m_Parameters->m_RestDensity, 7) - Real(1);

                                // clamp pressure error to zero to maintain stability
                                return error > Real(0) ? error : Real(0);
                            };

    runLoop(getState().size(),
        [&] (const size_t p) {
            Vec3r accel(0, 0, 0);
            const auto& neighborInfo = getState().getNeighborInfo()[p];
            assert(neighborInfo.size() >= 1);     // neighbor of particle p should contain at least p index
            if(neighborInfo.size() <= 1)
            {
                getState().getAccelerations()[p] = accel;
                return;
            }

            const auto pdensity  = getState().getDensities()[p];
            const auto ppressure = particlePressure(pdensity);

            for(size_t idx = 0; idx < neighborInfo.size(); ++idx)
            {
                const auto& qInfo    = neighborInfo[idx];
                const auto r         = qInfo.xpq;
                const auto qdensity  = qInfo.density;
                const auto qpressure = particlePressure(qdensity);

                // pressure forces
                accel += -(ppressure / (pdensity * pdensity) + qpressure / (qdensity * qdensity)) * m_Kernels.gradW(r);
            }
            accel *= m_Parameters->m_PressureStiffness * m_Parameters->m_ParticleMass;
            getState().getAccelerations()[p] = accel;
        });
}


void SPHModel::updateVelocity(const Real timestep)
{
    runLoop(getState().size(),
        [&] (const size_t p) {
            getState().getVelocities()[p] += (m_Parameters->m_Gravity + getState().getAccelerations()[p]) * timestep;
        });
}


void SPHModel::computeViscosity()
{
    runLoop(getState().size(),
        [&] (const size_t p) {
            const auto& neighborInfo = getState().getNeighborInfo()[p];
            assert(neighborInfo.size() >= 1);     // neighbor of particle p should contain at least p index
            if(neighborInfo.size() <= 1)
            {
                getState().getDiffuseVelocities()[p] = Vec3r(0, 0, 0);
                return;
            }
            const auto& pvel = getState().getVelocities()[p];
            ////////////////////////////////////////////////////////////////////////////////
            const auto& fluidNeighborList = getState().getFluidNeighborLists()[p];
            Vec3r diffuseFluid(0, 0, 0);
            for(size_t i = 0; i < fluidNeighborList.size(); ++i)
            {
                const auto q        = fluidNeighborList[i];
                const auto& qvel    = getState().getVelocities()[q];
                const auto& qInfo   = neighborInfo[i];
                const auto r        = qInfo.xpq;
                const auto qdensity = qInfo.density;
                diffuseFluid       += (Real(1.0) / qdensity) * m_Kernels.W(r) * (qvel - pvel);
            }
            diffuseFluid *= m_Parameters->m_ViscosityFluid;
            ////////////////////////////////////////////////////////////////////////////////
            Vec3r diffuseBoundary(0, 0, 0);
            if(m_Parameters->m_bDensityWithBoundary)
            {
                for(size_t i = fluidNeighborList.size(); i < neighborInfo.size(); ++i)
                {
                    const auto& qInfo   = neighborInfo[i];
                    const auto r        = qInfo.xpq;
                    diffuseBoundary    -= m_Parameters->m_RestDensityInv * m_Kernels.W(r) * pvel;
                }
                diffuseBoundary *= m_Parameters->m_ViscosityBoundary;
            }
            ////////////////////////////////////////////////////////////////////////////////
            getState().getDiffuseVelocities()[p] = (diffuseFluid + diffuseBoundary) * m_Parameters->m_ParticleMass;
        });

    // add diffused velocity back to velocity, causing viscosity
    runLoop(getState().size(),
        [&] (const size_t p) {
            getState().getVelocities()[p] += getState().getDiffuseVelocities()[p];
    });
}


void SPHModel::computeNormal()
{
    runLoop(getState().size(),
        [&] (const size_t p) {
            Vec3r n(0, 0, 0);
            const auto& neighborInfo = getState().getNeighborInfo()[p];
            assert(neighborInfo.size() >= 1);     // neighbor of particle p should contain at least p index
            if(neighborInfo.size() <= 1)
            {
                getState().getNormals()[p] = n;
                return;
            }

            for(size_t i = 0; i < neighborInfo.size(); ++i)
            {
                const auto& qInfo   = neighborInfo[i];
                const auto r        = qInfo.xpq;
                const auto qdensity = qInfo.density;
                n += (Real(1.0) / qdensity) * m_Kernels.gradW(r);
            }

            n *= m_Parameters->m_KernelRadius * m_Parameters->m_ParticleMass;
            getState().getNormals()[p] = n;
        });
}


void SPHModel::computeSurfaceTension()
{
    // Firstly compute surface normal for all particles
    computeNormal();

    // Compute surface tension for each particle, using Akinci et at. 2013 model
    runLoop(getState().size(),
        [&] (const size_t p) {
            const auto& fluidNeighborList = getState().getFluidNeighborLists()[p];
            assert(fluidNeighborList.size() >= 1);     // Neighbor of particle p should contain at least p index
            if(fluidNeighborList.size() <= 1)
            {
                return; // the particle has no neighbor
            }

            const auto ni            = getState().getNormals()[p];
            const auto pdensity      = getState().getDensities()[p];
            const auto& neighborInfo = getState().getNeighborInfo()[p];
            Vec3r accel(0, 0, 0);

            for(size_t i = 0; i < fluidNeighborList.size(); ++i)
            {
                const auto q  = fluidNeighborList[i];
                if(p == q)
                {
                    continue;
                }
                const auto& qInfo = neighborInfo[i];
                const auto qdensity = qInfo.density;

                // Correction factor
                const auto K_ij = Real(2) * m_Parameters->m_RestDensity / (pdensity + qdensity);

                // Cohesion acc
                auto r = qInfo.xpq;
                const auto d2 = r.squaredNorm();
                if(d2 > Real(1e-20))
                {
                    accel -= K_ij * m_Parameters->m_ParticleMass * (r / std::sqrt(d2)) * m_Kernels.cohesionW(r);
                }

                // Curvature acc
                const auto nj = getState().getNormals()[q];
                accel -= K_ij * (ni - nj);
            }

            accel *= m_Parameters->m_SurfaceTensionStiffness;
            getState().getAccelerations()[p] += accel;
        });
}


void SPHModel::advect(const Real timestep)
{
    runLoop(getState().size(),
        [&] (const size_t p) {
            getState().getPositions()[p] += getState().getVelocities()[p] * timestep;
    });
}
} // end namespace imstk