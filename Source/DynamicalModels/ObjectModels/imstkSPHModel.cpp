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
#include "imstkLogger.h"
#include "imstkParallelUtils.h"
#include "imstkPointSet.h"
#include "imstkTaskGraph.h"
#include "imstkVTKMeshIO.h"

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

  SPHModelConfig::SPHModelConfig(const Real particleRadius, const Real speedOfSound, const Real restDensity)
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

    m_particleMass = Real(std::pow(Real(2.0) * m_particleRadius, 3)) * m_restDensity * m_particleMassScale;
    m_restDensitySqr = m_restDensity * m_restDensity;
    m_restDensityInv = Real(1) / m_restDensity;

    m_kernelRadius = m_particleRadius * m_kernelOverParticleRadiusRatio;
    m_kernelRadiusSqr = m_kernelRadius * m_kernelRadius;

    m_pressureStiffness = m_restDensity * m_speedOfSound * m_speedOfSound / 7;
  }

  SPHModel::SPHModel() : DynamicalModel<SPHKinematicState>(DynamicalModelType::SmoothedParticleHydrodynamics)
  {
    m_validGeometryTypes = { Geometry::Type::PointSet };

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
      m_taskGraph->addFunction("SPHModel_ComputeViscosity", [&]()
        {
          computeViscosity(getTimeStep());
        });
    m_integrateNode =
      m_taskGraph->addFunction("SPHModel_Integrate", [&]()
        {
          sumAccels();
        });
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

    m_pressureAccels = std::make_shared<StdVectorOfVec3d>(getState().getNumParticles(), Vec3d(0, 0, 0));

    // initialize surface tension to 0 in case you remove the surface tension node
    m_surfaceTensionAccels = std::make_shared<StdVectorOfVec3d>(getState().getNumParticles(), Vec3d(0, 0, 0));

    m_viscousAccels = std::make_shared<StdVectorOfVec3d>(getState().getNumParticles(), Vec3d(0, 0, 0));
    m_neighborVelContr = std::make_shared<StdVectorOfVec3d>(getState().getNumParticles(), Vec3d(0, 0, 0));
    m_particleShift = std::make_shared<StdVectorOfVec3d>(getState().getNumParticles(), Vec3d(0, 0, 0));

    if (m_geomUnstructuredGrid)
    {
        m_minIndices.resize(m_geomUnstructuredGrid->getNumVertices());
    }

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

  Real
    SPHModel::computeCFLTimeStepSize()
  {
    auto maxVel = ParallelUtils::findMaxL2Norm(getState().getFullStepVelocities());

    // dt = CFL * 2r / (speed of sound + max{|| v ||})
    Real timestep = maxVel > Real(1e-6) ?
      m_modelParameters->m_CFLFactor * (Real(2.0) * m_modelParameters->m_particleRadius / (m_modelParameters->m_speedOfSound + maxVel)) :
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
          const Vec3r  r = ppos - qpos;
          neighborInfo.push_back({ r, m_modelParameters->m_restDensity });
        }
    };

    ParallelUtils::parallelFor(getState().getNumParticles(),
      [&](const size_t p) {
        if (m_sphBoundaryConditions && m_sphBoundaryConditions->getParticleTypes()[p] == SPHBoundaryConditions::ParticleType::buffer)
        {
          return;
        }

        const auto& ppos = getState().getPositions()[p];
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
        if (m_sphBoundaryConditions && m_sphBoundaryConditions->getParticleTypes()[p] == SPHBoundaryConditions::ParticleType::buffer)
        {
          return;
        }

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
        if (m_sphBoundaryConditions && m_sphBoundaryConditions->getParticleTypes()[p] == SPHBoundaryConditions::ParticleType::buffer)
        {
          return;
        }

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

    getState().getNormalizedDensities().resize(getState().getNumParticles());
    ParallelUtils::parallelFor(getState().getNumParticles(),
      [&](const size_t p) {
        if (m_sphBoundaryConditions && m_sphBoundaryConditions->getParticleTypes()[p] == SPHBoundaryConditions::ParticleType::buffer)
        {
          return;
        }

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

        getState().getDensities()[p] /= (tmp * m_modelParameters->m_particleMass);
      });
  }

  void
    SPHModel::computePressureAcceleration()
  {
    StdVectorOfVec3d& pressureAccels = *m_pressureAccels;
    ParallelUtils::parallelFor(getState().getNumParticles(),
      [&](const size_t p) {
        if (m_sphBoundaryConditions && m_sphBoundaryConditions->getParticleTypes()[p] == SPHBoundaryConditions::ParticleType::buffer)
        {
          return;
        }

        Vec3r accel(0, 0, 0);
        const auto& neighborInfo = getState().getNeighborInfo()[p];
        if (neighborInfo.size() <= 1)
        {
          pressureAccels[p] = accel;
          return;
        }

        const auto pdensity = getState().getDensities()[p];
        const auto ppressure = particlePressure(pdensity);

        for (size_t idx = 0; idx < neighborInfo.size(); ++idx)
        {
          const auto& qInfo = neighborInfo[idx];
          const auto r = qInfo.xpq;
          const auto qdensity = qInfo.density;
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
    SPHModel::computeViscosity(Real timestep)
  {
    const StdVectorOfVec3d& pressureAccels = *m_pressureAccels;
    const StdVectorOfVec3d& surfaceTensionAccels = *m_surfaceTensionAccels;
    StdVectorOfVec3d& viscousAccels = *m_viscousAccels;
    StdVectorOfVec3d& neighborVelContr = *m_neighborVelContr;
    StdVectorOfVec3d& particleShift = *m_particleShift;
    ParallelUtils::parallelFor(getState().getNumParticles(),
      [&](const size_t p) {
        if (m_sphBoundaryConditions &&
          (m_sphBoundaryConditions->getParticleTypes()[p] == SPHBoundaryConditions::ParticleType::buffer ||
            m_sphBoundaryConditions->getParticleTypes()[p] == SPHBoundaryConditions::ParticleType::wall))
        {
          return;
        }

        const auto& neighborInfo = getState().getNeighborInfo()[p];
        if (neighborInfo.size() <= 1)
        {
          neighborVelContr[p] = Vec3r(0, 0, 0);
          viscousAccels[p] = Vec3r(0, 0, 0);
          return;
        }

        Vec3r neighborVelContributions(0, 0, 0);
        Vec3r neighborVelContributionsNumerator(0, 0, 0);
        Real neighborVelContributionsDenominator = 0;
        Vec3r particleShifts(0, 0, 0);

        const auto& pvel = getState().getHalfStepVelocities()[p];
        const auto& fluidNeighborList = getState().getFluidNeighborLists()[p];
        const auto& pdensity = getState().getDensities()[p];

        Vec3r diffuseFluid(0, 0, 0);
        for (size_t i = 0; i < fluidNeighborList.size(); ++i)
        {
          const auto q = fluidNeighborList[i];
          const auto& qvel = getState().getHalfStepVelocities()[q];
          const auto& qInfo = neighborInfo[i];
          const auto r = qInfo.xpq;
          const auto qdensity = qInfo.density;
          diffuseFluid += (Real(1.0) / qdensity) * m_kernels.laplace(r) * (qvel - pvel);

          neighborVelContributionsNumerator += (qvel - pvel) * m_kernels.W(r);
          neighborVelContributionsDenominator += m_kernels.W(r);
          particleShifts += m_kernels.gradW(r);
          //diffuseFluid       += (Real(1.0) / qdensity) * m_kernels.W(r) * (qvel - pvel);

        }
        //diffuseFluid *= m_modelParameters->m_dynamicViscosityCoeff / getState().getDensities()[p];
        particleShifts *= 4 / 3 * PI * std::pow(m_modelParameters->m_particleRadius, 3) * 0.5 * m_modelParameters->m_kernelRadius * getState().getHalfStepVelocities()[p].norm();
        diffuseFluid *= m_modelParameters->m_dynamicViscosityCoeff * m_modelParameters->m_particleMass;
        neighborVelContr[p] = neighborVelContributionsNumerator * m_modelParameters->m_eta / neighborVelContributionsDenominator;
        particleShift[p] = -particleShifts;

        viscousAccels[p] = diffuseFluid;
      });
  }

  void
    SPHModel::computeSurfaceTension()
  {
    // First, compute surface normal for all particles
    ParallelUtils::parallelFor(getState().getNumParticles(),
      [&](const size_t p) {
        if (m_sphBoundaryConditions && m_sphBoundaryConditions->getParticleTypes()[p] == SPHBoundaryConditions::ParticleType::buffer)
        {
          return;
        }

        Vec3r n(0, 0, 0);
        const auto& neighborInfo = getState().getNeighborInfo()[p];
        if (neighborInfo.size() <= 1)
        {
          getState().getNormals()[p] = n;
          return;
        }

        for (size_t i = 0; i < neighborInfo.size(); ++i)
        {
          const auto& qInfo = neighborInfo[i];
          const auto r = qInfo.xpq;
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
        if (m_sphBoundaryConditions &&
          (m_sphBoundaryConditions->getParticleTypes()[p] == SPHBoundaryConditions::ParticleType::buffer ||
            m_sphBoundaryConditions->getParticleTypes()[p] == SPHBoundaryConditions::ParticleType::wall))
        {
          return;
        }

        const auto& fluidNeighborList = getState().getFluidNeighborLists()[p];
        if (fluidNeighborList.size() <= 1)
        {
          return; // the particle has no neighbor
        }

        const auto ni = getState().getNormals()[p];
        const auto pdensity = getState().getDensities()[p];
        const auto& neighborInfo = getState().getNeighborInfo()[p];

        Vec3r accel(0, 0, 0);
        for (size_t i = 0; i < fluidNeighborList.size(); ++i)
        {
          const auto q = fluidNeighborList[i];
          if (p == q)
          {
            continue;
          }
          const auto& qInfo = neighborInfo[i];
          const auto qdensity = qInfo.density;

          // Correction factor
          const auto K_ij = Real(2) * m_modelParameters->m_restDensity / (pdensity + qdensity);

          // Cohesion acc
          const auto r = qInfo.xpq;
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
    SPHModel::sumAccels()
  {
    const StdVectorOfVec3d& pressureAccels = *m_pressureAccels;
    const StdVectorOfVec3d& surfaceTensionAccels = *m_surfaceTensionAccels;
    const StdVectorOfVec3d& viscousAccels = *m_viscousAccels;
    ParallelUtils::parallelFor(getState().getNumParticles(),
      [&](const size_t p) {
        if (m_sphBoundaryConditions &&
          (m_sphBoundaryConditions->getParticleTypes()[p] == SPHBoundaryConditions::ParticleType::buffer ||
            m_sphBoundaryConditions->getParticleTypes()[p] == SPHBoundaryConditions::ParticleType::wall))
        {
          return;
        }

        getState().getAccelerations()[p] = pressureAccels[p] + surfaceTensionAccels[p] + viscousAccels[p];
      });
  }

  void
    SPHModel::updateVelocity(Real timestep)
  {
    ParallelUtils::parallelFor(getState().getNumParticles(),
      [&](const size_t p) {
        if (m_sphBoundaryConditions &&
          (m_sphBoundaryConditions->getParticleTypes()[p] == SPHBoundaryConditions::ParticleType::buffer ||
            m_sphBoundaryConditions->getParticleTypes()[p] == SPHBoundaryConditions::ParticleType::wall))
        {
          return;
        }

        // todo - simply run SPH for half a time step to start to we don't need to perform this check at every time step
        if (m_timeStepCount == 0)
        {
          getState().getHalfStepVelocities()[p] = getState().getFullStepVelocities()[p] + (m_modelParameters->m_gravity + getState().getAccelerations()[p]) * timestep / 2;
          getState().getFullStepVelocities()[p] += (m_modelParameters->m_gravity + getState().getAccelerations()[p]) * timestep;
        }
        else
        {
          getState().getHalfStepVelocities()[p] += (m_modelParameters->m_gravity + getState().getAccelerations()[p]) * timestep;
          getState().getFullStepVelocities()[p] = getState().getHalfStepVelocities()[p] + (m_modelParameters->m_gravity + getState().getAccelerations()[p]) * timestep / 2;
        }
        if (m_sphBoundaryConditions && m_sphBoundaryConditions->getParticleTypes()[p] == SPHBoundaryConditions::ParticleType::inlet)
        {
          getState().getHalfStepVelocities()[p] = m_sphBoundaryConditions->computeParabolicInletVelocity(getState().getPositions()[p]);
          getState().getFullStepVelocities()[p] = m_sphBoundaryConditions->computeParabolicInletVelocity(getState().getPositions()[p]);
        }
      });
  }

  void
    SPHModel::moveParticles(Real timestep)
  {
    //ParallelUtils::parallelFor(getState().getNumParticles(),
    //  [&](const size_t p) {
  
    Vec3d averageVelThroughHemorrhage(0, 0, 0);
    int numParticlesAcrossHemorrhagePlane = 0;
    StdVectorOfVec3d& neighborVelContr = *m_neighborVelContr;
    StdVectorOfVec3d& particleShift = *m_particleShift;

    for (int p = 0; p < getState().getNumParticles(); p++)
    {
      if (m_sphBoundaryConditions &&
        (m_sphBoundaryConditions->getParticleTypes()[p] == SPHBoundaryConditions::ParticleType::buffer ||
          m_sphBoundaryConditions->getParticleTypes()[p] == SPHBoundaryConditions::ParticleType::wall))
      {
        continue;
      }

      Vec3r oldPosition = getState().getPositions()[p];
      Vec3r newPosition = oldPosition + particleShift[p] * timestep + (getState().getHalfStepVelocities()[p] + neighborVelContr[p]) * timestep;

      getState().getPositions()[p] = newPosition;

      if (m_sphBoundaryConditions)
      {
        std::vector<SPHBoundaryConditions::ParticleType>& particleTypes = m_sphBoundaryConditions->getParticleTypes();
        if (particleTypes[p] == SPHBoundaryConditions::ParticleType::inlet &&
          !m_sphBoundaryConditions->isInInletDomain(newPosition))
        {
          // change particle type to fluid
          particleTypes[p] = SPHBoundaryConditions::ParticleType::fluid;
          // insert particle into inlet domain from buffer domain
          // todo: come up with a better way to find buffer indices
          // right now, the buffer index is limiting the parallel ability of this function
          const size_t bufferParticleIndex = m_sphBoundaryConditions->getBufferIndices().back();
          m_sphBoundaryConditions->getBufferIndices().pop_back();
          particleTypes[bufferParticleIndex] = SPHBoundaryConditions::ParticleType::inlet;

          getState().getPositions()[bufferParticleIndex] = m_sphBoundaryConditions->placeParticleAtInlet(oldPosition);
          getState().getHalfStepVelocities()[bufferParticleIndex] = m_sphBoundaryConditions->computeParabolicInletVelocity(getState().getPositions()[bufferParticleIndex]);
          getState().getFullStepVelocities()[bufferParticleIndex] = m_sphBoundaryConditions->computeParabolicInletVelocity(getState().getPositions()[bufferParticleIndex]);
        }
        else if (particleTypes[p] == SPHBoundaryConditions::ParticleType::outlet &&
          !m_sphBoundaryConditions->isInOutletDomain(newPosition))
        {
          particleTypes[p] = SPHBoundaryConditions::ParticleType::buffer;
          // insert particle into buffer domain after it leaves outlet domain
          getState().getPositions()[p] = m_sphBoundaryConditions->getBufferCoord();
          m_sphBoundaryConditions->getBufferIndices().push_back(p);
        }
        else if (particleTypes[p] == SPHBoundaryConditions::ParticleType::fluid &&
          m_sphBoundaryConditions->isInOutletDomain(newPosition))
        {
          particleTypes[p] = SPHBoundaryConditions::ParticleType::outlet;
        }
        else if (particleTypes[p] == SPHBoundaryConditions::ParticleType::fluid &&
            !m_sphBoundaryConditions->isInFluidDomain(newPosition))
        {
            particleTypes[p] = SPHBoundaryConditions::ParticleType::buffer;
            getState().getPositions()[p] = m_sphBoundaryConditions->getBufferCoord();
            m_sphBoundaryConditions->getBufferIndices().push_back(p);
        }
      }

      if (m_SPHHemorrhage && m_SPHHemorrhage->pointCrossedHemorrhagePlane(oldPosition, newPosition))
      {
          averageVelThroughHemorrhage += m_SPHHemorrhage->getNormal() * getState().getFullStepVelocities()[p].dot(m_SPHHemorrhage->getNormal());
          numParticlesAcrossHemorrhagePlane++;
      }
    }

    if (m_SPHHemorrhage)
    {
        if (numParticlesAcrossHemorrhagePlane > 0)
        {
            averageVelThroughHemorrhage /= numParticlesAcrossHemorrhagePlane;
        }
        else
        {
            averageVelThroughHemorrhage = m_prevAvgVelThroughHemorrhage;
        }
        m_prevAvgVelThroughHemorrhage = averageVelThroughHemorrhage;
        const double hemorrhageFlowRate = averageVelThroughHemorrhage.norm() * m_SPHHemorrhage->getHemorrhagePlaneArea();
        m_SPHHemorrhage->setHemorrhageRate(hemorrhageFlowRate);
    }
    m_totalTime += m_dt;
    m_timeStepCount++;
  }

  Real SPHModel::particlePressure(const double density)
  {
    const Real error = m_modelParameters->m_pressureStiffness * (std::pow(density / m_modelParameters->m_restDensity, 7) - Real(1));
    // clamp pressure error to zero to maintain stability
    return error > Real(0) ? error : Real(0);
  }

  void SPHModel::setInitialVelocities(const int numParticles, const Vec3d& initialVelocity)
  {
      m_initialVelocities.reserve(numParticles);
      for (size_t p = 0; p < numParticles; p++)
      {
          if (m_sphBoundaryConditions &&
              (m_sphBoundaryConditions->getParticleTypes()[p] == SPHBoundaryConditions::ParticleType::buffer ||
                  m_sphBoundaryConditions->getParticleTypes()[p] == SPHBoundaryConditions::ParticleType::wall))
          {
              m_initialVelocities.push_back(Vec3d(0, 0, 0));
          }
          else
          {
              m_initialVelocities.push_back(initialVelocity);
          }
      }
  }

  void SPHModel::writeStateToCSV()
  {
    if (m_csvPreviousTime <= m_csvTimeModulo && m_totalTime >= m_csvTimeModulo)
    {
      LOG(INFO) << "Writing CSV at time: " << m_totalTime;
      std::ofstream outputFile;
      outputFile.open(std::string("sph_output_") + std::to_string(m_totalTime) + std::string(".csv"));
      outputFile << "X,Y,Z,Vx,Vy,Vz,Pressure\n";
      const auto& positions = getState().getPositions();
      const auto& velocities = getState().getFullStepVelocities();
      const auto& densities = getState().getDensities();
      for (int i = 0; i < getState().getNumParticles(); ++i)
      {
        outputFile << positions[i].x() << "," << positions[i].y() << "," << positions[i].z() << ",";
        outputFile << velocities[i].x() << "," << velocities[i].y() << "," << velocities[i].z() << ",";
        outputFile << particlePressure(densities[i]) << "\n";
      }
      outputFile.close();

      m_csvTimeModulo += m_writeToOutputModulo;
      m_csvPreviousTime = m_totalTime;
    }
  }

  void SPHModel::findNearestParticleToVertex(const StdVectorOfVec3d& points, const std::vector<std::vector<size_t>>& indices)
  {
    for (size_t i = 0; i < points.size(); i++)
    {
      double minDistance = 1e10;
      size_t minIndex = 0;
      for (const size_t j : indices[i])
      {
          const Vec3d p1 = getState().getPositions()[j];
          const double distance = (points[i] - p1).norm();
          if (distance < minDistance)
          {
            minDistance = distance;
            minIndex = j;
          }
      }
      m_minIndices[i] = minIndex;
    }
  }

  void SPHModel::writeStateToVtk()
  {
    if (!m_geomUnstructuredGrid)
    {
      return;
    }

    if (m_vtkPreviousTime <= m_vtkTimeModulo && m_totalTime >= m_vtkTimeModulo)
    {
      LOG(INFO) << "Writing VTK at time: " << m_totalTime;
      const auto& particleVelocities = getState().getFullStepVelocities();
      const auto& particleDensities = getState().getDensities();
      std::map<std::string, StdVectorOfVectorf> pointDataMap;
      StdVectorOfVectorf velocity;
      StdVectorOfVectorf pressure;
      StdVectorOfVectorf density;
      velocity.reserve(m_geomUnstructuredGrid->getNumVertices());
      pressure.reserve(m_geomUnstructuredGrid->getNumVertices());
      density.reserve(m_geomUnstructuredGrid->getNumVertices());

      Vectorf densityVec(1);
      Vectorf pressureVec(1);
      Vectorf velocityVec(3);

      std::vector<std::vector<size_t>> result;
      m_neighborSearcher->getNeighbors(result, m_geomUnstructuredGrid->getInitialVertexPositions(), getState().getPositions());
      findNearestParticleToVertex(m_geomUnstructuredGrid->getInitialVertexPositions(), result);

      for (auto i : m_minIndices)
      {
        velocityVec(0) = particleVelocities[i].x();
        velocityVec(1) = particleVelocities[i].y();
        velocityVec(2) = particleVelocities[i].z();
        velocity.push_back(velocityVec);

        densityVec(0) = particleDensities[i];
        density.push_back(densityVec);

        pressureVec(0) = particlePressure(particleDensities[i]);
        pressure.push_back(pressureVec);
      }
      pointDataMap.insert(std::pair<std::string, StdVectorOfVectorf>("velocity", velocity));
      pointDataMap.insert(std::pair<std::string, StdVectorOfVectorf>("pressure", pressure));
      pointDataMap.insert(std::pair<std::string, StdVectorOfVectorf>("density", density));
      m_geomUnstructuredGrid->setPointDataMap(pointDataMap);

      VTKMeshIO vtkWriter;
      std::string filePath = std::string("sph_output_") + std::to_string(m_totalTime) + std::string(".vtu");
      vtkWriter.write(m_geomUnstructuredGrid, filePath, VTU);

      m_vtkTimeModulo += m_writeToOutputModulo;
      m_vtkPreviousTime = m_totalTime;
    }
  }
} // end namespace imstk
