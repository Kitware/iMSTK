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
WITHOUT WARRANTIES OR CONinitiDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

=========================================================================*/

#include "imstkSPHModel.h"
#include "imstkParallelUtils.h"
#include "imstkPointSet.h"
#include "imstkTaskGraph.h"
#include "imstkVTKMeshIO.h"

namespace imstk
{
SPHModelConfig::SPHModelConfig(const double particleRadius)
{
    // \todo Warning in all paths?
    if (std::abs(particleRadius) > 1.0e-6)
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

SPHModelConfig::SPHModelConfig(const double particleRadius, const double speedOfSound, const double restDensity)
{
    if (std::abs(particleRadius) > 1.0e-6)
    {
        LOG_IF(WARNING, (particleRadius < 0)) << "Particle radius supplied is negative! Using absolute value of the supplied radius.";
        m_particleRadius = std::abs(particleRadius);
    }
    else
    {
        LOG(WARNING) << "Particle radius too small! Setting to 1.e-6";
        m_particleRadius = 1.e-6;
    }

    if (speedOfSound < 0)
    {
        LOG(WARNING) << "Speed of sound is negative! Setting speed of sound to default value.";
    }
    else
    {
        m_speedOfSound = speedOfSound;
    }

    if (restDensity < 0)
    {
        LOG(WARNING) << "Rest density is negative! Setting rest density to default value.";
    }
    else
    {
        m_restDensity = restDensity;
    }
    initialize();
}

void
SPHModelConfig::initialize()
{
    // Compute the derived quantities
    m_particleRadiusSqr = m_particleRadius * m_particleRadius;

    m_particleMass   = std::pow(2.0 * m_particleRadius, 3) * m_restDensity * m_particleMassScale;
    m_restDensitySqr = m_restDensity * m_restDensity;
    m_restDensityInv = 1.0 / m_restDensity;

    m_kernelRadius    = m_particleRadius * m_kernelOverParticleRadiusRatio;
    m_kernelRadiusSqr = m_kernelRadius * m_kernelRadius;

    m_pressureStiffness = m_restDensity * m_speedOfSound * m_speedOfSound / 7.0;
}

SPHModel::SPHModel() : DynamicalModel<SPHState>(DynamicalModelType::SmoothedParticleHydrodynamics)
{
    m_validGeometryTypes = { "PointSet" };

    m_findParticleNeighborsNode = m_taskGraph->addFunction("SPHModel_Partition", std::bind(&SPHModel::findParticleNeighbors, this));
    m_computeDensityNode = m_taskGraph->addFunction("SPHModel_ComputeDensity", [&]()
        {
            computeNeighborRelativePositions();
            computeDensity();
        });

    m_normalizeDensityNode = m_taskGraph->addFunction("SPHModel_NormalizeDensity", std::bind(&SPHModel::normalizeDensity, this));

    m_collectNeighborDensityNode = m_taskGraph->addFunction("SPHModel_CollectNeighborDensity", std::bind(&SPHModel::collectNeighborDensity, this));

    m_computeTimeStepSizeNode =
        m_taskGraph->addFunction("SPHModel_ComputeTimestep", std::bind(&SPHModel::computeTimeStepSize, this));

    m_computePressureAccelNode =
        m_taskGraph->addFunction("SPHModel_ComputePressureAccel", std::bind(&SPHModel::computePressureAcceleration, this));

    m_computeSurfaceTensionNode =
        m_taskGraph->addFunction("SPHModel_ComputeSurfaceTensionAccel", std::bind(&SPHModel::computeSurfaceTension, this));

    m_computeViscosityNode =
        m_taskGraph->addFunction("SPHModel_ComputeViscosity", std::bind(&SPHModel::computeViscosity, this));

    m_integrateNode =
        m_taskGraph->addFunction("SPHModel_Integrate", std::bind(&SPHModel::sumAccels, this));

    m_updateVelocityNode =
        m_taskGraph->addFunction("SPHModel_UpdateVelocity", [&]()
            {
                updateVelocity(getTimeStep());
        });

    m_moveParticlesNode =
        m_taskGraph->addFunction("SPHModel_MoveParticles", [&]()
            {
                moveParticles(getTimeStep());
        });

    //m_computePositionNode =
    //    m_taskGraph->addFunction("SPHModel_ComputePositions", [&]()
    //    {
    //        moveParticles(getTimeStep());
    //        });
}

bool
SPHModel::initialize()
{
    LOG_IF(FATAL, (!this->getModelGeometry())) << "Model geometry is not yet set! Cannot initialize without model geometry.";
    m_pointSetGeometry = std::dynamic_pointer_cast<PointSet>(m_geometry);
    const int numParticles = m_pointSetGeometry->getNumVertices();

    // Allocate init and current state
    m_initialState = std::make_shared<SPHState>(numParticles);
    m_currentState = std::make_shared<SPHState>(numParticles);

    // If there were initial velocities (set them)
    if (m_initialVelocities != nullptr)
    {
        m_currentState->setVelocities(m_initialVelocities);
    }

    // Copy current to initial
    m_initialState->setState(m_currentState);

    // Share geometry and state position arrays
    m_currentState->setPositions(m_pointSetGeometry->getVertexPositions());
    m_initialState->setPositions(m_pointSetGeometry->getInitialVertexPositions());

    // Initialize simulation dependent parameters and kernel data
    m_kernels.initialize(m_modelParameters->m_kernelRadius);

    // Initialize neighbor searcher
    m_neighborSearcher = std::make_shared<NeighborSearch>(m_modelParameters->m_NeighborSearchMethod,
      m_modelParameters->m_kernelRadius);

    m_pressureAccels = std::make_shared<VecDataArray<double, 3>>(numParticles);
    std::fill_n(m_pressureAccels->getPointer(), m_pressureAccels->size(), Vec3d(0, 0, 0));

    // initialize surface tension to 0 in case you remove the surface tension node
    m_surfaceTensionAccels = std::make_shared<VecDataArray<double, 3>>(numParticles);
    std::fill_n(m_surfaceTensionAccels->getPointer(), m_surfaceTensionAccels->size(), Vec3d(0, 0, 0));

    m_viscousAccels = std::make_shared<VecDataArray<double, 3>>(numParticles);
    std::fill_n(m_viscousAccels->getPointer(), m_viscousAccels->size(), Vec3d(0, 0, 0));

    m_neighborVelContr = std::make_shared<VecDataArray<double, 3>>(numParticles);
    std::fill_n(m_neighborVelContr->getPointer(), m_neighborVelContr->size(), Vec3d(0, 0, 0));

    m_particleShift = std::make_shared<VecDataArray<double, 3>>(numParticles);
    std::fill_n(m_particleShift->getPointer(), m_particleShift->size(), Vec3d(0, 0, 0));

    // Add all the attributes to the geometry
    m_pointSetGeometry->setVertexAttribute("Pressure Accels", m_pressureAccels);
    m_pointSetGeometry->setVertexAttribute("Surface Tension Accels", m_surfaceTensionAccels);
    m_pointSetGeometry->setVertexAttribute("Viscous Accels", m_viscousAccels);
    m_pointSetGeometry->setVertexAttribute("Densities", m_currentState->getDensities());
    m_pointSetGeometry->setVertexAttribute("Velocities", m_currentState->getVelocities());
    m_pointSetGeometry->setVertexAttribute("Diffuse Velocities", m_currentState->getDiffuseVelocities());
    m_pointSetGeometry->setVertexAttribute("Normals", m_currentState->getNormals());
    m_pointSetGeometry->setVertexAttribute("Accels", m_currentState->getAccelerations());

    return true;
}

void
SPHModel::initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink)
{
    // Setup graph connectivity
    m_taskGraph->addEdge(source, m_findParticleNeighborsNode);
    m_taskGraph->addEdge(m_findParticleNeighborsNode, m_computeDensityNode);
    m_taskGraph->addEdge(m_computeDensityNode, m_normalizeDensityNode);
    m_taskGraph->addEdge(m_normalizeDensityNode, m_collectNeighborDensityNode);

    // Pressure, Surface Tension, and time step size can be done in parallel
    m_taskGraph->addEdge(m_collectNeighborDensityNode, m_computePressureAccelNode);
    m_taskGraph->addEdge(m_collectNeighborDensityNode, m_computeSurfaceTensionNode);
    m_taskGraph->addEdge(m_collectNeighborDensityNode, m_computeViscosityNode);
    m_taskGraph->addEdge(m_collectNeighborDensityNode, m_computeTimeStepSizeNode);

    m_taskGraph->addEdge(m_computePressureAccelNode, m_integrateNode);
    m_taskGraph->addEdge(m_computeSurfaceTensionNode, m_integrateNode);
    m_taskGraph->addEdge(m_computeViscosityNode, m_integrateNode);
    m_taskGraph->addEdge(m_computeTimeStepSizeNode, m_integrateNode);

    m_taskGraph->addEdge(m_integrateNode, m_updateVelocityNode);
    m_taskGraph->addEdge(m_updateVelocityNode, m_moveParticlesNode);
    m_taskGraph->addEdge(m_moveParticlesNode, sink);
}

void
SPHModel::computeTimeStepSize()
{
    m_dt = (this->m_timeStepSizeType == TimeSteppingType::Fixed) ? m_defaultDt : computeCFLTimeStepSize();
}

double
SPHModel::computeCFLTimeStepSize()
{
    auto maxVel = ParallelUtils::findMaxL2Norm(*getCurrentState()->getFullStepVelocities());

    // dt = CFL * 2r / (speed of sound + max{|| v ||})
    double timestep = maxVel > 1.0e-6 ?
                      m_modelParameters->m_CFLFactor * (2.0 * m_modelParameters->m_particleRadius / (m_modelParameters->m_speedOfSound + maxVel)) :
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
    m_neighborSearcher->getNeighbors(getCurrentState()->getFluidNeighborLists(), *getCurrentState()->getPositions());

    if (m_modelParameters->m_bDensityWithBoundary)   // if considering boundary particles for computing fluid density
    {
        m_neighborSearcher->getNeighbors(getCurrentState()->getBoundaryNeighborLists(),
            *getCurrentState()->getPositions(),
            *getCurrentState()->getBoundaryParticlePositions());
    }
}

void
SPHModel::computeNeighborRelativePositions()
{
    auto computeRelativePositions = [&](const Vec3d& ppos, const std::vector<size_t>& neighborList,
                                        const VecDataArray<double, 3>& allPositions, std::vector<NeighborInfo>& neighborInfo)
                                    {
                                        for (const size_t q : neighborList)
                                        {
                                            const Vec3d& qpos = allPositions[q];
                                            const Vec3d  r    = ppos - qpos;
                                            neighborInfo.push_back({ r, m_modelParameters->m_restDensity });
                                        }
                                    };

    std::shared_ptr<VecDataArray<double, 3>> positionsPtr = getCurrentState()->getPositions();
    const VecDataArray<double, 3>&           positions = *positionsPtr;

    std::vector<std::vector<NeighborInfo>>& neighborInfos = getCurrentState()->getNeighborInfo();

    ParallelUtils::parallelFor(getCurrentState()->getNumParticles(),
        [&](const size_t p)
        {
            if (m_sphBoundaryConditions
                && m_sphBoundaryConditions->getParticleTypes()[p] == SPHBoundaryConditions::ParticleType::Buffer)
            {
                return;
            }

            const Vec3d& ppos = positions[p];
            std::vector<NeighborInfo>& neighborInfo = neighborInfos[p];
            neighborInfo.resize(0);
            neighborInfo.reserve(48);

            computeRelativePositions(ppos, getCurrentState()->getFluidNeighborLists()[p], *getCurrentState()->getPositions(), neighborInfo);
            // if considering boundary particles then also cache relative positions with them
            if (m_modelParameters->m_bDensityWithBoundary)
            {
                computeRelativePositions(ppos, getCurrentState()->getBoundaryNeighborLists()[p], *getCurrentState()->getBoundaryParticlePositions(), neighborInfo);
            }
      });
}

void
SPHModel::collectNeighborDensity()
{
    // After computing particle densities, cache them into neighborInfo variable, next to relative positions
    // this is useful because relative positions and densities are accessed together multiple times
    // caching relative positions and densities therefore can reduce computation time significantly (tested)
    std::shared_ptr<DataArray<double>> densitiesPtr = getCurrentState()->getDensities();
    DataArray<double>&                 densities    = *densitiesPtr;

    const std::vector<std::vector<size_t>>&                 neighborLists = getCurrentState()->getFluidNeighborLists();
    const std::vector<SPHBoundaryConditions::ParticleType>& particleTypes = m_sphBoundaryConditions->getParticleTypes();

    ParallelUtils::parallelFor(getCurrentState()->getNumParticles(),
        [&](const size_t p)
        {
            if (m_sphBoundaryConditions && particleTypes[p] == SPHBoundaryConditions::ParticleType::Buffer)
            {
                return;
            }

            auto& neighborInfo = getCurrentState()->getNeighborInfo()[p];
            if (neighborInfo.size() <= 1)
            {
                return; // the particle has no neighbor
            }

            const auto& fluidNeighborList = neighborLists[p];
            for (size_t i = 0; i < fluidNeighborList.size(); ++i)
            {
                auto q = fluidNeighborList[i];
                neighborInfo[i].density = densities[q];
            }
      });
}

void
SPHModel::computeDensity()
{
    std::shared_ptr<DataArray<double>> densitiesPtr = getCurrentState()->getDensities();
    DataArray<double>&                 densities    = *densitiesPtr;

    const std::vector<std::vector<NeighborInfo>>& neighborInfos = getCurrentState()->getNeighborInfo();

    ParallelUtils::parallelFor(getCurrentState()->getNumParticles(),
        [&](const size_t p)
        {
            if (m_sphBoundaryConditions && m_sphBoundaryConditions->getParticleTypes()[p] == SPHBoundaryConditions::ParticleType::Buffer)
            {
                return;
            }

            const std::vector<NeighborInfo>& neighborInfo = neighborInfos[p];
            if (neighborInfo.size() <= 1)
            {
                return; // the particle has no neighbor
            }

            double pdensity = 0.0;
            for (const auto& qInfo : neighborInfo)
            {
                pdensity += m_kernels.W(qInfo.xpq);
            }
            pdensity    *= m_modelParameters->m_particleMass;
            densities[p] = pdensity;
      });
}

//void
//SPHModel::computePressureOutlet()
//{
//  ParallelUtils::parallelFor(getState().getNumParticles(),
//    [&](const size_t p) {
//      if (getState().getPositions()[p].x() > m_outletRegionXCoord && getState().getPositions()[p].x() < m_maxXCoord)
//      {
//        getState().getDensities()[p] = m_outletDensity;
//      }
//    });
//}

void
SPHModel::normalizeDensity()
{
    if (!m_modelParameters->m_bNormalizeDensity)
    {
        return;
    }

    std::shared_ptr<DataArray<double>> densitiesPtr = getCurrentState()->getDensities();
    DataArray<double>&                 densities    = *densitiesPtr;

    const std::vector<std::vector<size_t>>&                 neighborLists = getCurrentState()->getFluidNeighborLists();
    const std::vector<std::vector<NeighborInfo>>&           neighborInfos = getCurrentState()->getNeighborInfo();
    const std::vector<SPHBoundaryConditions::ParticleType>& particleTypes = m_sphBoundaryConditions->getParticleTypes();

    ParallelUtils::parallelFor(getCurrentState()->getNumParticles(),
        [&](const size_t p)
        {
            if (m_sphBoundaryConditions && particleTypes[p] == SPHBoundaryConditions::ParticleType::Buffer)
            {
                return;
            }

            const std::vector<NeighborInfo>& neighborInfo = neighborInfos[p];
            if (neighborInfo.size() <= 1)
            {
                return; // the particle has no neighbor
            }

            const auto& fluidNeighborList = neighborLists[p];
            double tmp = 0.0;

            for (size_t i = 0; i < fluidNeighborList.size(); ++i)
            {
                const auto& qInfo = neighborInfo[i];

                // because we're not done with density computation, qInfo does not contain desity of particle q yet
                const auto q = fluidNeighborList[i];
                const auto qdensity = densities[q];
                tmp += m_kernels.W(qInfo.xpq) / qdensity;
            }

            densities[p] /= (tmp * m_modelParameters->m_particleMass);
      });
}

void
SPHModel::computePressureAcceleration()
{
    std::shared_ptr<DataArray<double>> densitiesPtr   = getCurrentState()->getDensities();
    const DataArray<double>&           densities      = *densitiesPtr;
    VecDataArray<double, 3>&           pressureAccels = *m_pressureAccels;

    const std::vector<std::vector<NeighborInfo>>&           neighborInfos = getCurrentState()->getNeighborInfo();
    const std::vector<SPHBoundaryConditions::ParticleType>& particleTypes = m_sphBoundaryConditions->getParticleTypes();

    ParallelUtils::parallelFor(getCurrentState()->getNumParticles(),
        [&](const size_t p)
        {
            if (m_sphBoundaryConditions && particleTypes[p] == SPHBoundaryConditions::ParticleType::Buffer)
            {
                return;
            }

            Vec3d accel = Vec3d::Zero();
            const std::vector<NeighborInfo>& neighborInfo = neighborInfos[p];
            if (neighborInfo.size() <= 1)
            {
                pressureAccels[p] = accel;
                return;
            }

            const auto pdensity  = densities[p];
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

            accel *= m_modelParameters->m_particleMass;

            //getState().getAccelerations()[p] = accel;
            pressureAccels[p] = accel;
      });
}

void
SPHModel::computeViscosity()
{
    VecDataArray<double, 3>&       viscousAccels      = *m_viscousAccels;
    VecDataArray<double, 3>&       neighborVelContr   = *m_neighborVelContr;
    VecDataArray<double, 3>&       particleShift      = *m_particleShift;
    const VecDataArray<double, 3>& halfStepVelocities = *getCurrentState()->getHalfStepVelocities();

    const std::vector<std::vector<NeighborInfo>>& neighborInfos = getCurrentState()->getNeighborInfo();
    const std::vector<std::vector<size_t>>&       neighborLists = getCurrentState()->getFluidNeighborLists();

    ParallelUtils::parallelFor(getCurrentState()->getNumParticles(),
        [&](const size_t p)
        {
            if (m_sphBoundaryConditions
                && (m_sphBoundaryConditions->getParticleTypes()[p] == SPHBoundaryConditions::ParticleType::Buffer
                    || m_sphBoundaryConditions->getParticleTypes()[p] == SPHBoundaryConditions::ParticleType::Wall))
            {
                return;
            }

            const std::vector<NeighborInfo>& neighborInfo = neighborInfos[p];
            if (neighborInfo.size() <= 1)
            {
                neighborVelContr[p] = Vec3d::Zero();
                viscousAccels[p]    = Vec3d::Zero();
                return;
            }

            Vec3d neighborVelContributionsNumerator    = Vec3d::Zero();
            double neighborVelContributionsDenominator = 0.0;
            Vec3d particleShifts = Vec3d::Zero();

            const Vec3d& pvel = halfStepVelocities[p];
            const std::vector<size_t>& fluidNeighborList = neighborLists[p];

            Vec3d diffuseFluid = Vec3d::Zero();
            for (size_t i = 0; i < fluidNeighborList.size(); ++i)
            {
                const auto q        = fluidNeighborList[i];
                const auto& qvel    = halfStepVelocities[q];
                const auto& qInfo   = neighborInfo[i];
                const auto r        = qInfo.xpq;
                const auto qdensity = qInfo.density;
                diffuseFluid       += (1.0 / qdensity) * m_kernels.laplace(r) * (qvel - pvel);

                neighborVelContributionsNumerator   += (qvel - pvel) * m_kernels.W(r);
                neighborVelContributionsDenominator += m_kernels.W(r);
                particleShifts += m_kernels.gradW(r);
                //diffuseFluid       += (Real(1.0) / qdensity) * m_kernels.W(r) * (qvel - pvel);
            }
            //diffuseFluid *= m_modelParameters->m_dynamicViscosityCoeff / getState().getDensities()[p];
            const double particleRadius = m_modelParameters->m_particleRadius;
            particleShifts     *= 4 / 3 * PI * particleRadius * particleRadius * particleRadius * 0.5 * m_modelParameters->m_kernelRadius * halfStepVelocities[p].norm();
            diffuseFluid       *= m_modelParameters->m_dynamicViscosityCoeff * m_modelParameters->m_particleMass;
            neighborVelContr[p] = neighborVelContributionsNumerator * m_modelParameters->m_eta / neighborVelContributionsDenominator;
            particleShift[p]    = -particleShifts;

            viscousAccels[p] = diffuseFluid;
      });
}

void
SPHModel::computeSurfaceTension()
{
    VecDataArray<double, 3>& surfaceNormals = *getCurrentState()->getNormals();

    const std::vector<std::vector<NeighborInfo>>& neighborInfos = getCurrentState()->getNeighborInfo();

    // First, compute surface normal for all particles
    ParallelUtils::parallelFor(getCurrentState()->getNumParticles(),
        [&](const size_t p)
        {
            if (m_sphBoundaryConditions && m_sphBoundaryConditions->getParticleTypes()[p] == SPHBoundaryConditions::ParticleType::Buffer)
            {
                return;
            }

            Vec3d n(0.0, 0.0, 0.0);
            const std::vector<NeighborInfo>& neighborInfo = neighborInfos[p];
            if (neighborInfo.size() <= 1)
            {
                surfaceNormals[p] = n;
                return;
            }

            for (size_t i = 0; i < neighborInfo.size(); ++i)
            {
                const auto& qInfo   = neighborInfo[i];
                const auto r        = qInfo.xpq;
                const auto qdensity = qInfo.density;
                n += (1.0 / qdensity) * m_kernels.gradW(r);
            }

            n *= m_modelParameters->m_kernelRadius * m_modelParameters->m_particleMass;
            surfaceNormals[p] = n;
        });

    VecDataArray<double, 3>& surfaceTensionAccels = *m_surfaceTensionAccels;
    const DataArray<double>& densities = *getCurrentState()->getDensities();

    const std::vector<std::vector<size_t>>& neighborLists = getCurrentState()->getFluidNeighborLists();

    // Second, compute surface tension acceleration
    ParallelUtils::parallelFor(getCurrentState()->getNumParticles(),
        [&](const size_t p)
        {
            if (m_sphBoundaryConditions
                && (m_sphBoundaryConditions->getParticleTypes()[p] == SPHBoundaryConditions::ParticleType::Buffer
                    || m_sphBoundaryConditions->getParticleTypes()[p] == SPHBoundaryConditions::ParticleType::Wall))
            {
                return;
            }

            const std::vector<size_t>& fluidNeighborList = neighborLists[p];
            if (fluidNeighborList.size() <= 1)
            {
                return; // the particle has no neighbor
            }

            const Vec3d& ni          = surfaceNormals[p];
            const double pdensity    = densities[p];
            const auto& neighborInfo = neighborInfos[p];

            Vec3d accel = Vec3d::Zero();
            for (size_t i = 0; i < fluidNeighborList.size(); ++i)
            {
                const size_t q = fluidNeighborList[i];
                if (p == q)
                {
                    continue;
                }
                const NeighborInfo& qInfo = neighborInfo[i];
                const double qdensity     = qInfo.density;

                // Correction factor
                const double K_ij = 2.0 * m_modelParameters->m_restDensity / (pdensity + qdensity);

                // Cohesion acc
                const Vec3d& r  = qInfo.xpq;
                const double d2 = r.squaredNorm();
                if (d2 > 1.0e-20)
                {
                    accel -= K_ij * m_modelParameters->m_particleMass * (r / std::sqrt(d2)) * m_kernels.cohesionW(r);
                }

                // Curvature acc
                const Vec3d& nj = surfaceNormals[q];
                accel -= K_ij * (ni - nj);
            }

            accel *= m_modelParameters->m_surfaceTensionStiffness;
            //getState().getAccelerations()[p] += accel;
            surfaceTensionAccels[p] = accel;
        });
}

void
SPHModel::sumAccels()
{
    const VecDataArray<double, 3>& pressureAccels       = *m_pressureAccels;
    const VecDataArray<double, 3>& surfaceTensionAccels = *m_surfaceTensionAccels;
    const VecDataArray<double, 3>& viscousAccels = *m_viscousAccels;
    VecDataArray<double, 3>&       accels = *getCurrentState()->getAccelerations();

    ParallelUtils::parallelFor(getCurrentState()->getNumParticles(),
        [&](const size_t p)
        {
            if (m_sphBoundaryConditions
                && (m_sphBoundaryConditions->getParticleTypes()[p] == SPHBoundaryConditions::ParticleType::Buffer
                    || m_sphBoundaryConditions->getParticleTypes()[p] == SPHBoundaryConditions::ParticleType::Wall))
            {
                return;
            }

            accels[p] = pressureAccels[p] + surfaceTensionAccels[p] + viscousAccels[p];
      });
}

void
SPHModel::updateVelocity(const double timestep)
{
    VecDataArray<double, 3>&       halfStepVelocities = *getCurrentState()->getHalfStepVelocities();
    VecDataArray<double, 3>&       fullStepVelocities = *getCurrentState()->getFullStepVelocities();
    const VecDataArray<double, 3>& positions = *getCurrentState()->getPositions();
    const VecDataArray<double, 3>& accels    = *getCurrentState()->getAccelerations();

    ParallelUtils::parallelFor(getCurrentState()->getNumParticles(),
        [&](const size_t p)
        {
            if (m_sphBoundaryConditions
                && (m_sphBoundaryConditions->getParticleTypes()[p] == SPHBoundaryConditions::ParticleType::Buffer
                    || m_sphBoundaryConditions->getParticleTypes()[p] == SPHBoundaryConditions::ParticleType::Wall))
            {
                return;
            }

            // todo - simply run SPH for half a time step to start to we don't need to perform this check at every time step
            if (m_timeStepCount == 0)
            {
                halfStepVelocities[p]  = fullStepVelocities[p] + (m_modelParameters->m_gravity + accels[p]) * timestep * 0.5;
                fullStepVelocities[p] += (m_modelParameters->m_gravity + accels[p]) * timestep;
            }
            else
            {
                halfStepVelocities[p] += (m_modelParameters->m_gravity + accels[p]) * timestep;
                fullStepVelocities[p]  = halfStepVelocities[p] + (m_modelParameters->m_gravity + accels[p]) * timestep * 0.5;
            }
            if (m_sphBoundaryConditions && m_sphBoundaryConditions->getParticleTypes()[p] == SPHBoundaryConditions::ParticleType::Inlet)
            {
                halfStepVelocities[p] = m_sphBoundaryConditions->computeParabolicInletVelocity(positions[p]);
                fullStepVelocities[p] = m_sphBoundaryConditions->computeParabolicInletVelocity(positions[p]);
            }
      });
}

void
SPHModel::moveParticles(const double timestep)
{
    //ParallelUtils::parallelFor(getState().getNumParticles(),
    //  [&](const size_t p) {

    VecDataArray<double, 3>& neighborVelContr = *m_neighborVelContr;
    VecDataArray<double, 3>& particleShift    = *m_particleShift;
    VecDataArray<double, 3>& positions = *getCurrentState()->getPositions();
    VecDataArray<double, 3>& halfStepVelocities = *getCurrentState()->getHalfStepVelocities();
    VecDataArray<double, 3>& fullStepVelocities = *getCurrentState()->getFullStepVelocities();

    for (int p = 0; p < static_cast<int>(getCurrentState()->getNumParticles()); p++)
    {
        if (m_sphBoundaryConditions
            && (m_sphBoundaryConditions->getParticleTypes()[p] == SPHBoundaryConditions::ParticleType::Buffer
                || m_sphBoundaryConditions->getParticleTypes()[p] == SPHBoundaryConditions::ParticleType::Wall))
        {
            continue;
        }

        Vec3d oldPosition = positions[p];
        Vec3d newPosition = oldPosition + particleShift[p] * timestep + (halfStepVelocities[p] + neighborVelContr[p]) * timestep;

        positions[p] = newPosition;

        if (m_sphBoundaryConditions)
        {
            std::vector<SPHBoundaryConditions::ParticleType>& particleTypes = m_sphBoundaryConditions->getParticleTypes();
            if (particleTypes[p] == SPHBoundaryConditions::ParticleType::Inlet
                && !m_sphBoundaryConditions->isInInletDomain(newPosition))
            {
                // change particle type to fluid
                particleTypes[p] = SPHBoundaryConditions::ParticleType::Fluid;
                // insert particle into inlet domain from buffer domain
                // todo: come up with a better way to find buffer indices
                // right now, the buffer index is limiting the parallel ability of this function
                const size_t bufferParticleIndex = m_sphBoundaryConditions->getBufferIndices().back();
                m_sphBoundaryConditions->getBufferIndices().pop_back();
                particleTypes[bufferParticleIndex] = SPHBoundaryConditions::ParticleType::Inlet;

                positions[bufferParticleIndex] = m_sphBoundaryConditions->placeParticleAtInlet(oldPosition);
                halfStepVelocities[bufferParticleIndex] = m_sphBoundaryConditions->computeParabolicInletVelocity(positions[bufferParticleIndex]);
                fullStepVelocities[bufferParticleIndex] = m_sphBoundaryConditions->computeParabolicInletVelocity(positions[bufferParticleIndex]);
            }
            else if (particleTypes[p] == SPHBoundaryConditions::ParticleType::Outlet
                     && !m_sphBoundaryConditions->isInOutletDomain(newPosition))
            {
                particleTypes[p] = SPHBoundaryConditions::ParticleType::Buffer;
                // insert particle into buffer domain after it leaves outlet domain
                positions[p] = m_sphBoundaryConditions->getBufferCoord();
                m_sphBoundaryConditions->getBufferIndices().push_back(p);
            }
            else if (particleTypes[p] == SPHBoundaryConditions::ParticleType::Fluid
                     && m_sphBoundaryConditions->isInOutletDomain(newPosition))
            {
                particleTypes[p] = SPHBoundaryConditions::ParticleType::Outlet;
            }
            else if (particleTypes[p] == SPHBoundaryConditions::ParticleType::Fluid
                     && !m_sphBoundaryConditions->isInFluidDomain(newPosition))
            {
                particleTypes[p] = SPHBoundaryConditions::ParticleType::Buffer;
                positions[p]     = m_sphBoundaryConditions->getBufferCoord();
                m_sphBoundaryConditions->getBufferIndices().push_back(p);
            }
        }
    }
    m_timeStepCount++;
}

double
SPHModel::particlePressure(const double density)
{
    const double d     = density / m_modelParameters->m_restDensity;
    const double d2    = d * d;
    const double d4    = d2 * d2;
    const double error = m_modelParameters->m_pressureStiffness * (d4 * d2 * d - 1.0);
    // clamp pressure error to zero to maintain stability
    return error > 0.0 ? error : 0.0;
}

void
SPHModel::setInitialVelocities(const size_t numParticles, const Vec3d& initialVelocity)
{
    m_initialVelocities->clear();
    m_initialVelocities->reserve(static_cast<int>(numParticles));
    for (size_t p = 0; p < numParticles; p++)
    {
        if (m_sphBoundaryConditions
            && (m_sphBoundaryConditions->getParticleTypes()[p] == SPHBoundaryConditions::ParticleType::Buffer
                || m_sphBoundaryConditions->getParticleTypes()[p] == SPHBoundaryConditions::ParticleType::Wall))
        {
            m_initialVelocities->push_back(Vec3d::Zero());
        }
        else
        {
            m_initialVelocities->push_back(initialVelocity);
        }
    }
}

void
SPHModel::findNearestParticleToVertex(const VecDataArray<double, 3>& points, const std::vector<std::vector<size_t>>& indices)
{
    const VecDataArray<double, 3>& positions = *getCurrentState()->getPositions();
    for (size_t i = 0; i < static_cast<size_t>(points.size()); i++)
    {
        double minDistance = 1e10;
        size_t minIndex    = 0;
        for (const size_t j : indices[i])
        {
            const Vec3d  p1       = positions[j];
            const double distance = (points[i] - p1).norm();
            if (distance < minDistance)
            {
                minDistance = distance;
                minIndex    = j;
            }
        }
        m_minIndices[i] = minIndex;
    }
}
} // end namespace imstk
