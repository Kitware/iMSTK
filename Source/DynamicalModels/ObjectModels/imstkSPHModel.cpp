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
#include "imstkLogger.h"
#include "imstkParallelUtils.h"
#include "imstkPointSet.h"
#include "imstkTaskGraph.h"


namespace imstk
{
SPHModelConfig::SPHModelConfig(const Real particleRadius)
{
    if (std::abs(particleRadius) > Real(1.e-6))
    {
        LOG_IF(WARNING, (particleRadius < 0)) << "Particle radius supplied is negative! Using absolute value of the supplied radius.";
        m_particleRadius = std::abs(particleRadius);
    }
    else
    {
        LOG(WARNING) << "Particle radius too small! Setting to 1.e-6";
        m_particleRadius = 1.e-6;
    }
    initialize();
}

void
SPHModelConfig::initialize()
{
    // Compute the derived quantities
    m_particleRadiusSqr = m_particleRadius * m_particleRadius;

    m_particleMass   = Real(std::pow(Real(2.0) * m_particleRadius, 3)) * m_restDensity * m_particleMassScale;
    m_restDensitySqr = m_restDensity * m_restDensity;
    m_restDensityInv = Real(1) / m_restDensity;

    m_kernelRadius    = m_particleRadius * m_kernelOverParticleRadiusRatio;
    m_kernelRadiusSqr = m_kernelRadius * m_kernelRadius;
}

SPHModel::SPHModel() : DynamicalModel<SPHKinematicState>(DynamicalModelType::SmoothedParticleHydrodynamics)
{
    m_validGeometryTypes = { Geometry::Type::PointSet };

    m_findParticleNeighborsNode = m_taskGraph->addFunction("SPHModel_Partition", std::bind(&SPHModel::findParticleNeighbors, this));
    m_computeDensityNode = m_taskGraph->addFunction("SPHModel_ComputeDensity", [&]()
        {
            computeNeighborRelativePositions();
            computeDensity();
            normalizeDensity();
            collectNeighborDensity();
        });
    m_computePressureAccelNode =
        m_taskGraph->addFunction("SPHModel_ComputePressureAccel", std::bind(&SPHModel::computePressureAcceleration, this));
    m_computeSurfaceTensionNode =
        m_taskGraph->addFunction("SPHModel_ComputeSurfaceTensionAccel", std::bind(&SPHModel::computeSurfaceTension, this));
    m_computeTimeStepSizeNode =
        m_taskGraph->addFunction("SPHModel_ComputeTimestep", std::bind(&SPHModel::computeTimeStepSize, this));
    m_integrateNode =
        m_taskGraph->addFunction("SPHModel_Integrate", [&]()
        {
            sumAccels();
            //updateVelocity(getTimeStep());
            updateVelocityNoGravity(getTimeStep());
            computeViscosity();
            moveParticles(getTimeStep());
        });
}

bool
SPHModel::initialize()
{
    LOG_IF(FATAL, (!this->getModelGeometry())) << "Model geometry is not yet set! Cannot initialize without model geometry.";
    m_pointSetGeometry = std::dynamic_pointer_cast<PointSet>(m_geometry);

    // Initialize  positions and velocity of the particles
    this->m_initialState = std::make_shared<SPHKinematicState>();
    this->m_currentState = std::make_shared<SPHKinematicState>();

    // Set particle positions and zero default velocities
    /// \todo set particle data with given (non-zero) velocities
    this->m_initialState->setParticleData(m_pointSetGeometry->getVertexPositions(), m_initialVelocities);
    this->m_currentState->setState(this->m_initialState);

    // Attach current state to simulation state
    m_simulationState.setKinematicState(this->m_currentState);

    // Initialize (allocate memory for) simulation data such as density, acceleration etc.
    m_simulationState.initializeData();

    // Initialize simulation dependent parameters and kernel data
    m_kernels.initialize(m_modelParameters->m_kernelRadius);

    // Initialize neighbor searcher
    m_neighborSearcher = std::make_shared<NeighborSearch>(m_modelParameters->m_NeighborSearchMethod,
        m_modelParameters->m_kernelRadius);

    m_pressureAccels       = std::make_shared<StdVectorOfVec3d>(getState().getNumParticles());

    // initialize surface tension to 0 in case you remove the surface tension node
    m_surfaceTensionAccels = std::make_shared<StdVectorOfVec3d>(getState().getNumParticles(), Vec3d(0.0, 0.0, 0.0));

    m_totalTime = 0.0;
    m_timeStepCount = 0;
    //m_writeToCSVModulo = DBL_MAX;

    return true;
}

void
SPHModel::updatePhysicsGeometry()
{
    assert(m_pointSetGeometry);
    m_pointSetGeometry->setVertexPositions(this->m_currentState->getPositions());
}

void
SPHModel::initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink)
{
    // Setup graph connectivity
    m_taskGraph->addEdge(source, m_findParticleNeighborsNode);
    m_taskGraph->addEdge(m_findParticleNeighborsNode, m_computeDensityNode);

    // Pressure, Surface Tension, and time step size can be done in parallel
    m_taskGraph->addEdge(m_computeDensityNode, m_computePressureAccelNode);
    m_taskGraph->addEdge(m_computeDensityNode, m_computeSurfaceTensionNode);
    m_taskGraph->addEdge(m_computeDensityNode, m_computeTimeStepSizeNode);

    m_taskGraph->addEdge(m_computePressureAccelNode, m_integrateNode);
    m_taskGraph->addEdge(m_computeSurfaceTensionNode, m_integrateNode);
    m_taskGraph->addEdge(m_computeTimeStepSizeNode, m_integrateNode);

    m_taskGraph->addEdge(m_integrateNode, sink);
}

void
SPHModel::computeTimeStepSize()
{
    m_dt = (this->m_timeStepSizeType == TimeSteppingType::Fixed) ? m_defaultDt : computeCFLTimeStepSize();
    m_totalTime += m_dt;
    m_timeStepCount++;
}

Real
SPHModel::computeCFLTimeStepSize()
{
    auto maxVel = ParallelUtils::findMaxL2Norm(getState().getVelocities());

    // dt = CFL * 2r / max{|| v ||}
    Real timestep = maxVel > Real(1e-6) ?
                    m_modelParameters->m_CFLFactor * (Real(2.0) * m_modelParameters->m_particleRadius / maxVel) :
                    m_modelParameters->m_maxTimestep;

    // clamp the time step size to be within a given range
    if (timestep > m_modelParameters->m_maxTimestep)
    {
        timestep = m_modelParameters->m_maxTimestep;
    }
    else if (timestep < m_modelParameters->m_minTimestep)
    {
        timestep = m_modelParameters->m_minTimestep;
    }

    return timestep;
}

void
SPHModel::findParticleNeighbors()
{
    m_neighborSearcher->getNeighbors(getState().getFluidNeighborLists(), getState().getPositions());
    if (m_modelParameters->m_bDensityWithBoundary)   // if considering boundary particles for computing fluid density
    {
        m_neighborSearcher->getNeighbors(getState().getBoundaryNeighborLists(),
            getState().getPositions(),
            getState().getBoundaryParticlePositions());
    }
}

void
SPHModel::computeNeighborRelativePositions()
{
    auto computeRelativePositions = [&](const Vec3r& ppos, const std::vector<size_t>& neighborList,
                                        const StdVectorOfVec3r& allPositions, std::vector<NeighborInfo>& neighborInfo) {
                                        for (const size_t q : neighborList)
                                        {
                                            const Vec3r& qpos = allPositions[q];
                                            const Vec3r  r    = ppos - qpos;
                                            neighborInfo.push_back({ r, m_modelParameters->m_restDensity });
                                        }
                                    };

    ParallelUtils::parallelFor(getState().getNumParticles(),
        [&](const size_t p) {
            const auto& ppos   = getState().getPositions()[p];
            auto& neighborInfo = getState().getNeighborInfo()[p];
            neighborInfo.resize(0);
            neighborInfo.reserve(48);

            computeRelativePositions(ppos, getState().getFluidNeighborLists()[p], getState().getPositions(), neighborInfo);
            // if considering boundary particles then also cache relative positions with them
            if (m_modelParameters->m_bDensityWithBoundary)
            {
                computeRelativePositions(ppos, getState().getBoundaryNeighborLists()[p], getState().getBoundaryParticlePositions(), neighborInfo);
            }
        });
}

void
SPHModel::collectNeighborDensity()
{
    // after computing particle densities, cache them into neighborInfo variable, next to relative positions
    // this is useful because relative positions and densities are accessed together multiple times
    // caching relative positions and densities therefore can reduce computation time significantly (tested)
    ParallelUtils::parallelFor(getState().getNumParticles(),
        [&](const size_t p) {
            auto& neighborInfo = getState().getNeighborInfo()[p];
            if (neighborInfo.size() <= 1)
            {
                return; // the particle has no neighbor
            }

            const auto& fluidNeighborList = getState().getFluidNeighborLists()[p];
            for (size_t i = 0; i < fluidNeighborList.size(); ++i)
            {
                auto q = fluidNeighborList[i];
                neighborInfo[i].density = getState().getDensities()[q];
            }
        });
}

void
SPHModel::computeDensity()
{
    ParallelUtils::parallelFor(getState().getNumParticles(),
        [&](const size_t p) {
            const auto& neighborInfo = getState().getNeighborInfo()[p];
            if (neighborInfo.size() <= 1)
            {
                return; // the particle has no neighbor
            }

            Real pdensity = 0;
            for (const auto& qInfo : neighborInfo)
            {
                pdensity += m_kernels.W(qInfo.xpq);
            }
            pdensity *= m_modelParameters->m_particleMass;
            getState().getDensities()[p] = pdensity;
        });
}

void
SPHModel::normalizeDensity()
{
    if (!m_modelParameters->m_bNormalizeDensity)
    {
        return;
    }

    getState().getNormalizedDensities().resize(getState().getNumParticles());
    ParallelUtils::parallelFor(getState().getNumParticles(),
        [&](const size_t p) {
            auto& neighborInfo = getState().getNeighborInfo()[p];
            if (neighborInfo.size() <= 1)
            {
                return; // the particle has no neighbor
            }

            const auto& fluidNeighborList = getState().getFluidNeighborLists()[p];
            Real tmp = 0;

            for (size_t i = 0; i < fluidNeighborList.size(); ++i)
            {
                const auto& qInfo = neighborInfo[i];

                // because we're not done with density computation, qInfo does not contain desity of particle q yet
                const auto q = fluidNeighborList[i];
                const auto qdensity = getState().getDensities()[q];
                tmp += m_kernels.W(qInfo.xpq) / qdensity;
            }

            if (m_modelParameters->m_bDensityWithBoundary)
            {
#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
                const auto& BDNeighborList = getState().getBoundaryNeighborLists()[p];
                LOG_IF(FATAL, (fluidNeighborList.size() + BDNeighborList.size() != neighborInfo.size()))
                << "Invalid neighborInfo computation";
#endif
                for (size_t i = fluidNeighborList.size(); i < neighborInfo.size(); ++i)
                {
                    const auto& qInfo = neighborInfo[i];
                    tmp += m_kernels.W(qInfo.xpq) / m_modelParameters->m_restDensity;     // density of boundary particle is set to rest density
                }
            }

            getState().getNormalizedDensities()[p] = getState().getDensities()[p] / (tmp * m_modelParameters->m_particleMass);
        });

    // put normalized densities to densities
    std::swap(getState().getDensities(), getState().getNormalizedDensities());
}

void
SPHModel::computePressureAcceleration()
{
    //auto particlePressure = [&](const Real density) {
    //                            const Real error = std::pow(density / m_modelParameters->m_restDensity, 7) - Real(1);
    //                            // clamp pressure error to zero to maintain stability
    //                            return error > Real(0) ? error : Real(0);
    //                        };

    StdVectorOfVec3d& pressureAccels = *m_pressureAccels;
    ParallelUtils::parallelFor(getState().getNumParticles(),
        [&](const size_t p) {
            Vec3r accel(0, 0, 0);
            const auto& neighborInfo = getState().getNeighborInfo()[p];
            if (neighborInfo.size() <= 1)
            {
                getState().getAccelerations()[p] = accel;
                return;
            }

            const auto pdensity  = getState().getDensities()[p];
            const auto ppressure = particlePressure(pdensity);

            for (size_t idx = 0; idx < neighborInfo.size(); ++idx)
            {
                const auto& qInfo    = neighborInfo[idx];
                const auto r         = qInfo.xpq;
                const auto qdensity  = qInfo.density;
                const auto qpressure = particlePressure(qdensity);

                // pressure forces
                accel += -(ppressure / (pdensity * pdensity) + qpressure / (qdensity * qdensity)) * m_kernels.gradW(r);
            }

            accel *= m_modelParameters->m_pressureStiffness * m_modelParameters->m_particleMass;
            //getState().getAccelerations()[p] = accel;
            pressureAccels[p] = accel;
        });
}

void
SPHModel::sumAccels()
{
    const StdVectorOfVec3d& pressureAccels       = *m_pressureAccels;
    const StdVectorOfVec3d& surfaceTensionAccels = *m_surfaceTensionAccels;
    ParallelUtils::parallelFor(getState().getNumParticles(),
        [&](const size_t p) {
            getState().getAccelerations()[p] = pressureAccels[p] + surfaceTensionAccels[p];
        });
}

void
SPHModel::updateVelocity(Real timestep)
{
    ParallelUtils::parallelFor(getState().getNumParticles(),
        [&](const size_t p) {
            getState().getVelocities()[p] += (m_modelParameters->m_gravity + getState().getAccelerations()[p]) * timestep;
        });
}

void
SPHModel::updateVelocityNoGravity(Real timestep)
{
  ParallelUtils::parallelFor(getState().getNumParticles(),
    [&](const size_t p) {
      getState().getVelocities()[p] += getState().getAccelerations()[p] * timestep;
    });
}

void
SPHModel::computeViscosity()
{
    ParallelUtils::parallelFor(getState().getNumParticles(),
        [&](const size_t p) {
            const auto& neighborInfo = getState().getNeighborInfo()[p];
            if (neighborInfo.size() <= 1)
            {
                getState().getDiffuseVelocities()[p] = Vec3r(0, 0, 0);
                return;
            }

            const auto& pvel = getState().getVelocities()[p];
            const auto& fluidNeighborList = getState().getFluidNeighborLists()[p];

            Vec3r diffuseFluid(0, 0, 0);
            for (size_t i = 0; i < fluidNeighborList.size(); ++i)
            {
                const auto q        = fluidNeighborList[i];
                const auto& qvel    = getState().getVelocities()[q];
                const auto& qInfo   = neighborInfo[i];
                const auto r        = qInfo.xpq;
                const auto qdensity = qInfo.density;
                diffuseFluid       += (Real(1.0) / qdensity) * m_kernels.W(r) * (qvel - pvel);
            }
            diffuseFluid *= m_modelParameters->m_viscosityCoeff;

            Vec3r diffuseBoundary(0, 0, 0);
            if (m_modelParameters->m_bDensityWithBoundary)
            {
                for (size_t i = fluidNeighborList.size(); i < neighborInfo.size(); ++i)
                {
                    const auto& qInfo = neighborInfo[i];
                    const auto r      = qInfo.xpq;
                    diffuseBoundary  -= m_modelParameters->m_restDensityInv * m_kernels.W(r) * pvel;
                }
                diffuseBoundary *= m_modelParameters->m_viscosityBoundary;
            }

            getState().getDiffuseVelocities()[p] = (diffuseFluid + diffuseBoundary) * m_modelParameters->m_particleMass;
        });

    // add diffused velocity back to velocity, causing viscosity
    ParallelUtils::parallelFor(getState().getNumParticles(),
        [&](const size_t p) {
            getState().getVelocities()[p] += getState().getDiffuseVelocities()[p];
        });
}

void
SPHModel::computeSurfaceTension()
{
    // First, compute surface normal for all particles
    ParallelUtils::parallelFor(getState().getNumParticles(),
        [&](const size_t p) {
            Vec3r n(0, 0, 0);
            const auto& neighborInfo = getState().getNeighborInfo()[p];
            if (neighborInfo.size() <= 1)
            {
                getState().getNormals()[p] = n;
                return;
            }

            for (size_t i = 0; i < neighborInfo.size(); ++i)
            {
                const auto& qInfo   = neighborInfo[i];
                const auto r        = qInfo.xpq;
                const auto qdensity = qInfo.density;
                n += (Real(1.0) / qdensity) * m_kernels.gradW(r);
            }

            n *= m_modelParameters->m_kernelRadius * m_modelParameters->m_particleMass;
            getState().getNormals()[p] = n;
        });

    // Second, compute surface tension acceleration
    StdVectorOfVec3d& surfaceTensionAccels = *m_surfaceTensionAccels;
    ParallelUtils::parallelFor(getState().getNumParticles(),
        [&](const size_t p) {
            const auto& fluidNeighborList = getState().getFluidNeighborLists()[p];
            if (fluidNeighborList.size() <= 1)
            {
                return; // the particle has no neighbor
            }

            const auto ni            = getState().getNormals()[p];
            const auto pdensity      = getState().getDensities()[p];
            const auto& neighborInfo = getState().getNeighborInfo()[p];

            Vec3r accel(0, 0, 0);
            for (size_t i = 0; i < fluidNeighborList.size(); ++i)
            {
                const auto q = fluidNeighborList[i];
                if (p == q)
                {
                    continue;
                }
                const auto& qInfo   = neighborInfo[i];
                const auto qdensity = qInfo.density;

                // Correction factor
                const auto K_ij = Real(2) * m_modelParameters->m_restDensity / (pdensity + qdensity);

                // Cohesion acc
                const auto r  = qInfo.xpq;
                const auto d2 = r.squaredNorm();
                if (d2 > Real(1e-20))
                {
                    accel -= K_ij * m_modelParameters->m_particleMass * (r / std::sqrt(d2)) * m_kernels.cohesionW(r);
                }

                // Curvature acc
                const auto nj = getState().getNormals()[q];
                accel -= K_ij * (ni - nj);
            }

            accel *= m_modelParameters->m_surfaceTensionStiffness;
            //getState().getAccelerations()[p] += accel;
            surfaceTensionAccels[p] = accel;
        });
}

void
SPHModel::moveParticles(Real timestep)
{
    ParallelUtils::parallelFor(getState().getNumParticles(),
        [&](const size_t p) {
            getState().getPositions()[p] += getState().getVelocities()[p] * timestep;
            periodicBCs(p);
        });

    if (!m_wallPointIndices.empty())
    {
      ParallelUtils::parallelFor(m_wallPointIndices.size(),
        [&](const size_t p) {
          getState().getPositions()[m_wallPointIndices[p]] -= getState().getVelocities()[m_wallPointIndices[p]] * timestep;
          getState().getVelocities()[m_wallPointIndices[p]] = Vec3r(0.0, 0.0, 0.0);
        });
    }
}

Real SPHModel::particlePressure(const double density)
{
  const Real error = std::pow(density / m_modelParameters->m_restDensity, 7) - Real(1);
  // clamp pressure error to zero to maintain stability
  return error > Real(0) ? error : Real(0);
}

void SPHModel::periodicBCs(const size_t p)
{
   if (getState().getPositions()[p].x() > m_maxXCoord)
   {
     getState().getPositions()[p].x() = m_minXCoord;
   }
}

void SPHModel::setInitialVelocities(StdVectorOfVec3d& initialVelocities)
{
  m_initialVelocities = initialVelocities;
}
StdVectorOfVec3d SPHModel::getInitialVelocities()
{
  return m_initialVelocities;
}

void SPHModel::writeStateToCSV()
{
  if (std::fmod(m_totalTime, m_writeToCSVModulo) < 1e-8 && m_totalTime > 0)
  {
    std::cout << "Writing CSV at time: " << m_totalTime << std::endl;
    std::ofstream outputFile;
    outputFile.open(iMSTK_DATA_ROOT + std::string("sph_output_") + std::to_string(m_totalTime) + std::string(".csv"));
    outputFile << "X,Y,Z,Vx,Vy,Vz,Pressure\n";
    auto positions = getState().getPositions();
    auto velocities = getState().getVelocities();
    auto densities = getState().getDensities();
    for (int i = 0; i < getState().getNumParticles(); ++i)
    {
      outputFile << positions[i].x() << "," << positions[i].y() << "," << positions[i].z() << ",";
      outputFile << velocities[i].x() << "," << velocities[i].y() << "," << velocities[i].z() << ",";
      outputFile << particlePressure(densities[i]) << "\n";
    }
    outputFile.close();
  }
}

} // end namespace imstk
