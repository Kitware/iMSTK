/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkDynamicalSystem.h"
#include "imstkSphState.h"
#include "imstkSPHKernels.h"
#include "imstkNeighborSearch.h"
#include "imstkSphBoundaryConditions.h"

namespace imstk
{
class PointSet;

///
/// \class SphSystemConfig
///
/// \brief Class that holds the SPH model parameters
///
class SphSystemConfig
{
private:
    void initialize();

public:
    SphSystemConfig(const double particleRadius);
    SphSystemConfig(const double particleRadius, const double speedOfSound, const double restDensity);

    /// \todo Move this to solver or time integrator in the future
    double m_minTimestep = 1.0e-6;
    double m_maxTimestep = 1.0e-3;
    double m_cflFactor   = 1.0;

    // particle parameters
    double m_particleRadius    = 0.0;
    double m_particleRadiusSqr = 0.0; ///< \note derived quantity

    // material parameters
    double m_restDensity       = 1000.0;
    double m_restDensitySqr    = 1000000.0;    ///< \note derived quantity
    double m_restDensityInv    = 1.0 / 1000.0; ///< \note derived quantity
    double m_particleMass      = 1.0;
    double m_particleMassScale = 1.0;          ///< scale particle mass to a smaller value to maintain stability
    double m_eta = 0.5;                        ///< proportion of position change due to neighbors velocity (XSPH method)

    bool m_bNormalizeDensity    = false;
    bool m_bDensityWithBoundary = false;

    // pressure
    double m_pressureStiffness = 50000.0;

    // viscosity and surface tension/cohesion
    double m_dynamicViscosityCoeff   = 1.0e-2;
    double m_viscosityBoundary       = 1.0e-5;
    double m_surfaceTensionStiffness = 1.0;
    double m_frictionBoundary = 0.1;

    // kernel properties
    double m_kernelOverParticleRadiusRatio = 4.0;
    double m_kernelRadius;    ///< \note derived quantity
    double m_kernelRadiusSqr; ///< \note derived quantity

    // gravity
    Vec3d m_gravity = Vec3d(0.0, -9.81, 0.0);

    // sound speed
    double m_speedOfSound = 18.7;

    // neighbor search
    NeighborSearch::Method m_neighborSearchMethod = NeighborSearch::Method::UniformGridBasedSearch;
};

///
/// \class SPHModel
/// \brief SPH fluid model
///
class SphSystem : public DynamicalSystem<SphState>
{
public:
    using AbstractDynamicalSystem::initGraphEdges;

    SphSystem();
    ~SphSystem() override = default;

    ///
    /// \brief Set simulation parameters
    ///
    void configure(const std::shared_ptr<SphSystemConfig>& params) { m_modelParameters = params; }

    ///
    /// \brief Initialize the dynamical model
    ///
    bool initialize() override;

    ///
    /// \brief Reset the current state to the initial state
    ///
    void resetToInitialState() override { this->m_currentState->setState(this->m_initialState); }

    ///
    /// \brief Get the simulation parameters
    ///
    const std::shared_ptr<SphSystemConfig>& getParameters() const
    {
        assert(m_modelParameters);
        return m_modelParameters;
    }

    ///
    /// \brief Set the default time step size,
    /// valid only if using a fixed time step for integration
    ///
    void setTimeStep(const double timeStep) override { setDefaultTimeStep(timeStep); }

    ///
    /// \brief Set the default time step size,
    /// valid only if using a fixed time step for integration
    ///
    void setDefaultTimeStep(const double timeStep) { m_defaultDt = timeStep; }

    ///
    /// \brief Returns the time step size
    ///
    double getTimeStep() const override { return m_dt; }

    void setInitialVelocities(const size_t numParticles, const Vec3d& initialVelocities);

    double getParticlePressure(const double density);

    ///
    /// \brief Write the state to external file
    /// \todo move this out of this class
    ///
    void findNearestParticleToVertex(const VecDataArray<double, 3>& points, const std::vector<std::vector<size_t>>& indices);

    void setBoundaryConditions(std::shared_ptr<SphBoundaryConditions> sphBoundaryConditions) { m_sphBoundaryConditions = sphBoundaryConditions; }
    std::shared_ptr<SphBoundaryConditions> getBoundaryConditions() { return m_sphBoundaryConditions; }

    void setRestDensity(const double restDensity) { m_modelParameters->m_restDensity = restDensity; }

    std::shared_ptr<TaskNode> getFindParticleNeighborsNode() const { return m_findParticleNeighborsNode; }
    std::shared_ptr<TaskNode> getComputeDensityNode() const { return m_computeDensityNode; }
    std::shared_ptr<TaskNode> getComputePressureNode() const { return m_computePressureAccelNode; }
    std::shared_ptr<TaskNode> getComputeSurfaceTensionNode() const { return m_computeSurfaceTensionNode; }
    std::shared_ptr<TaskNode> getComputeTimeStepSizeNode() const { return m_computeTimeStepSizeNode; }
    std::shared_ptr<TaskNode> getSumAccelsNode() const { return m_sumAccelsNode; }
    std::shared_ptr<TaskNode> getIntegrateNode() const { return m_integrateNode; }
    std::shared_ptr<TaskNode> getComputeViscosityNode() const { return m_computeViscosityNode; }
    std::shared_ptr<TaskNode> getUpdateVelocityNode() const { return m_updateVelocityNode; }
    std::shared_ptr<TaskNode> getMoveParticlesNode() const { return m_moveParticlesNode; }

protected:
    ///
    /// \brief Setup SPH compute graph connectivity
    ///
    void initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink) override;

private:
    ///
    /// \brief Compute time step size, do nothing if using a fixed time step size for integration
    ///
    void computeTimeStepSize();

    ///
    /// \brief Compute time step size based on CFL condition
    ///
    double computeCFLTimeStepSize();

    ///
    /// \brief Find the neighbors for each particle
    ///
    void findParticleNeighbors();

    ///
    /// \brief Pre-compute relative positions with neighbor particles
    ///
    void computeNeighborRelativePositions();

    ///
    /// \brief Collect the densities of neighbor particles,
    /// called after all density computation (after density normalization, if applicable)
    ///
    void collectNeighborDensity();

    ///
    /// \brief Compute particle densities
    ///
    void computeDensity();

    ///
    /// \brief Normalize densities, producing smoother density field
    ///
    void normalizeDensity();

    ///
    /// \brief Compute particle acceleration due to pressure
    ///
    void computePressureAcceleration();

    ///
    /// \brief Sum the forces computed in parallel
    ///
    void sumAccels();

    ///
    /// \brief Update particle velocities due to pressure, viscous, and surface tension forces
    ///
    void updateVelocity(const double timestep);

    ///
    /// \brief Compute viscosity
    ///
    void computeViscosity();

    ///
    /// \brief Compute surface tension and update velocities
    /// Compute surface tension using Akinci et at. 2013 model
    /// (Versatile Surface Tension and Adhesion for SPH Fluids)
    ///
    void computeSurfaceTension();

    ///
    /// \brief Move particles
    ///
    void moveParticles(const double timestep);

//void computePressureOutlet();

protected:
    std::shared_ptr<TaskNode> m_findParticleNeighborsNode = nullptr;
    std::shared_ptr<TaskNode> m_computeDensityNode        = nullptr;
    std::shared_ptr<TaskNode> m_computePressureAccelNode  = nullptr;
    std::shared_ptr<TaskNode> m_computeSurfaceTensionNode = nullptr;
    std::shared_ptr<TaskNode> m_computeTimeStepSizeNode   = nullptr;
    std::shared_ptr<TaskNode> m_sumAccelsNode              = nullptr;
    std::shared_ptr<TaskNode> m_integrateNode              = nullptr;
    std::shared_ptr<TaskNode> m_updateVelocityNode         = nullptr;
    std::shared_ptr<TaskNode> m_computeViscosityNode       = nullptr;
    std::shared_ptr<TaskNode> m_moveParticlesNode          = nullptr;
    std::shared_ptr<TaskNode> m_normalizeDensityNode       = nullptr;
    std::shared_ptr<TaskNode> m_collectNeighborDensityNode = nullptr;

private:
    std::shared_ptr<PointSet> m_pointSetGeometry;

    double m_dt = 0.0;                                   ///< time step size
    double m_defaultDt;                                  ///< default time step size

    SphSimulationKernels m_kernels;                      ///< SPH kernels (must be initialized during model initialization)
    std::shared_ptr<SphSystemConfig> m_modelParameters;  ///< SPH Model parameters (must be set before simulation)
    std::shared_ptr<NeighborSearch>  m_neighborSearcher; ///< Neighbor Search (must be initialized during model initialization)

    std::shared_ptr<VecDataArray<double, 3>> m_pressureAccels       = nullptr;
    std::shared_ptr<VecDataArray<double, 3>> m_surfaceTensionAccels = nullptr;
    std::shared_ptr<VecDataArray<double, 3>> m_viscousAccels    = nullptr;
    std::shared_ptr<VecDataArray<double, 3>> m_neighborVelContr = nullptr;
    std::shared_ptr<VecDataArray<double, 3>> m_particleShift    = nullptr;

    std::shared_ptr<VecDataArray<double, 3>> m_initialVelocities = nullptr;
    std::shared_ptr<DataArray<double>>       m_initialDensities  = nullptr;

    int m_timeStepCount = 0;

    std::shared_ptr<SphBoundaryConditions> m_sphBoundaryConditions = nullptr;

    std::vector<size_t> m_minIndices;
};
} // namespace imstk