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

#include "imstkPointSet.h"
#include "imstkDynamicalModel.h"
#include "imstkSPHState.h"
#include "imstkSPHKernels.h"
#include "imstkNeighborSearch.h"

namespace imstk
{
///
/// \class SPHModelConfig
/// \brief Class that holds the SPH model parameters
///
class SPHModelConfig
{
private:
    void initialize();

public:
    SPHModelConfig(const Real particleRadius);

    /// \todo Move this to solver or time integrator in the future
    Real m_minTimestep = Real(1e-6);
    Real m_maxTimestep = Real(1e-3);
    Real m_CFLFactor   = Real(1.0);

    // particle parameters
    Real m_particleRadius    = Real(0);
    Real m_particleRadiusSqr = Real(0); ///> \note derived quantity

    // material parameters
    Real m_restDensity       = Real(1000.0);
    Real m_restDensitySqr    = Real(1000000.0);    ///> \note derived quantity
    Real m_restDensityInv    = Real(1.0 / 1000.0); ///> \note derived quantity
    Real m_particleMass      = Real(1);
    Real m_particleMassScale = Real(0.95);         ///> scale particle mass to a smaller value to maintain stability

    bool m_bNormalizeDensity    = false;
    bool m_bDensityWithBoundary = false;

    // pressure
    Real m_pressureStiffness = Real(50000.0);

    // viscosity and surface tension/cohesion
    Real m_viscosityCoeff          = Real(1e-2);
    Real m_viscosityBoundary       = Real(1e-5);
    Real m_surfaceTensionStiffness = Real(1);
    Real m_frictionBoundary        = Real(0.1);

    // kernel properties
    Real m_kernelOverParticleRadiusRatio = Real(4.0);
    Real m_kernelRadius;    ///> \note derived quantity
    Real m_kernelRadiusSqr; ///> \note derived quantity

    // gravity
    Vec3r m_gravity = Vec3r(0, -9.81, 0);

    // neighbor search
    NeighborSearch::Method m_NeighborSearchMethod = NeighborSearch::Method::UniformGridBasedSearch;
};

///
/// \class SPHModel
/// \brief SPH fluid model
///
class SPHModel : public DynamicalModel<SPHKinematicState>
{
public:
    ///
    /// \brief Constructor
    ///
    SPHModel() : DynamicalModel<SPHKinematicState>(DynamicalModelType::SPH) {}

    ///
    /// \brief Destructor
    ///
    virtual ~SPHModel() override = default;

    ///
    /// \brief Set simulation parameters
    ///
    void configure(const std::shared_ptr<SPHModelConfig>& params) { m_modelParameters = params; }

    ///
    /// \brief Set the geometry (particle positions)
    ///
    void setModelGeometry(const std::shared_ptr<PointSet>& geo) { m_geometry = geo; }

    ///
    /// \brief Initialize the dynamical model
    ///
    virtual bool initialize() override;

    ///
    /// \brief Update states
    ///
    virtual void updateBodyStates(const Vectord&, const stateUpdateType) override {}

    ///
    /// \brief Update positions of point set geometry
    ///
    virtual void updatePhysicsGeometry() override
    { assert(m_geometry); m_geometry->setVertexPositions(this->m_currentState->getPositions()); }

    ///
    /// \brief Reset the current state to the initial state
    ///
    virtual void resetToInitialState() override 
    { this->m_currentState->setState(this->m_initialState); }

    ///
    /// \brief Get the simulation parameters
    ///
    const std::shared_ptr<SPHModelConfig>& getParameters() const
    { assert(m_modelParameters); return m_modelParameters; }

    ///
    /// \brief Get the kinematics particle data (positions + velocities)
    ///
    SPHKinematicState& getKinematicsState()
    {
        assert(this->m_currentState);
        return *this->m_currentState;
    }

    const SPHKinematicState& getKinematicsState() const
    {
        assert(this->m_currentState);
        return *this->m_currentState;
    }

    ///
    /// \brief Get particle simulation data
    ///
    SPHSimulationState& getState() { return m_simulationState; }
    const SPHSimulationState& getState() const { return m_simulationState; }

    ///
    /// \brief Set the default time step size,
    /// valid only if using a fixed time step for integration
    ///
    virtual void setTimeStep(const double timeStep) override
    { setDefaultTimeStep(static_cast<Real>(timeStep)); }

    ///
    /// \brief Set the default time step size,
    /// valid only if using a fixed time step for integration
    ///
    void setDefaultTimeStep(const Real timeStep)
    { m_defaultDt = static_cast<Real>(timeStep); }

    ///
    /// \brief Returns the time step size
    ///
    virtual double getTimeStep() const override
    { return static_cast<double>(m_dt); }

    ///
    /// \brief Do one time step simulation
    ///
    void advanceTimeStep();

private:
    ///
    /// \brief Compute time step size, do nothing if using a fixed time step size for integration
    ///
    void computeTimeStepSize();

    ///
    /// \brief Compute time step size based on CFL condition
    ///
    Real computeCFLTimeStepSize();

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
    /// \brief Update particle velocities due to pressure
    ///
    void updateVelocity(const Real timestep);

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
    void moveParticles(const Real timestep);

    std::shared_ptr<PointSet> m_geometry;
    SPHSimulationState        m_simulationState;

    Real m_dt;                                          ///> time step size
    Real m_defaultDt;                                   ///> default time step size

    SPHSimulationKernels m_kernels;                     ///> SPH kernels (must be initialized during model initialization)
    std::shared_ptr<SPHModelConfig> m_modelParameters;  ///> SPH Model parameters (must be set before simulation)
    std::shared_ptr<NeighborSearch> m_neighborSearcher; ///> Neighbor Search (must be initialized during model initialization)
};
} // end namespace imstk
