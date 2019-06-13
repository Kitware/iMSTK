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

#include "imstkGridBasedNeighborSearch.h"
#include "imstkSpatialHashTableSeparateChaining.h"

namespace imstk
{
///
/// \brief The NeighborSearchMethod enum
///
enum class NeighborSearchMethod
{
    GridBased,
    SpatialHashing
};

///
/// \brief The NeighborSearch struct
///
struct NeighborSearch
{
    GridBasedNeighborSearch gridSearch;
    SpatialHashTableSeparateChaining spatialHashSearch;
};

///
/// \class SPHModelConfig
/// \brief Parameters for SPH simulation
///
class SPHModelConfig
{
private:
    void initialize();

public:
    SPHModelConfig(const Real particleRadius);

    ////////////////////////////////////////////////////////////////////////////////
    // TODO: Move this to solver or time integrator in the future
    Real m_MinTimestep = Real(1e-6);
    Real m_MaxTimestep = Real(1e-3);
    Real m_CFLFactor   = Real(1.0);
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    // particle parameters
    Real m_ParticleRadius      = Real(0);
    Real m_ParticleRadiusSqr   = Real(0);
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    // material parameters
    Real m_RestDensity    = Real(1000.0);
    Real m_RestDensitySqr = Real(1000000.0);
    Real m_RestDensityInv = Real(1.0 / 1000.0);
    Real m_ParticleMass       = Real(1);
    Real m_ParticleMassScale  = Real(0.9);    // scale particle mass to a smaller value to maintain stability

    bool m_bNormalizeDensity    = false;
    bool m_bDensityWithBoundary = false;
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    // pressure
    Real m_PressureStiffness = Real(50000.0);
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    // viscosity and surface tension/cohesion
    Real m_ViscosityFluid          = Real(1e-2);
    Real m_ViscosityBoundary       = Real(1e-5);
    Real m_SurfaceTensionStiffness = Real(1);
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    // kernel data
    Real m_RatioKernelOverParticleRadius = Real(4.0);
    Real m_KernelRadius;
    Real m_KernelRadiusSqr;
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    // gravity
    Vec3r m_Gravity = Vec3r(0, -9.81, 0);
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    // neighbor search
    NeighborSearchMethod m_NeighborSearchMethod = NeighborSearchMethod::GridBased;
    ////////////////////////////////////////////////////////////////////////////////
};

///
/// \class SPHModel
/// \brief SPH simulation model
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
    virtual ~SPHModel() = default;

    ///
    /// \brief Set simulation parameters
    ///
    void configure(const std::shared_ptr<SPHModelConfig>& params) { m_Parameters = params; }

    ///
    /// \brief Set the geometry (particle positions)
    ///
    void setModelGeometry(const std::shared_ptr<PointSet>& geo) { m_Geometry = geo; }

    ///
    /// \brief Initialize the dynamical model
    ///
    virtual bool initialize() override;

    ///
    /// \brief Update states
    ///
    virtual void updateBodyStates(const Vectord&, const typename DynamicalModel<SPHKinematicState>::stateUpdateType) override {}

    ///
    /// \brief Update positions of point set geometry
    ///
    virtual void updatePhysicsGeometry() override { assert(m_Geometry); m_Geometry->setVertexPositions(this->m_currentState->getPositions()); }

    ///
    /// \brief Reset the current state to the initial state
    ///
    virtual void resetToInitialState() override
    {
        this->m_currentState->setState(this->m_initialState);
    }

    ///
    /// \brief Get the parameters
    ///
    std::shared_ptr<SPHModelConfig>& getParameters() { assert(m_Parameters); return m_Parameters; }
    const std::shared_ptr<SPHModelConfig>& getParameters() const { assert(m_Parameters); return m_Parameters; }

    ///
    /// \brief Get the kinematics particle data (positions + velocities)
    ///
    SPHKinematicState& getKinematicsState() { assert(this->m_currentState); return *this->m_currentState; }
    const SPHKinematicState& getKinematicsState() const { assert(this->m_currentState); return *this->m_currentState; }

    ///
    /// \brief Get particle simulation data
    ///
    SPHSimulationState& getState() { return m_SimulationState; }
    const SPHSimulationState& getState() const { return m_SimulationState; }

    ///
    /// \brief Set the default time step size, valid only if using a fixed time step for integration
    ///
    virtual void setTimeStep(const double timeStep) override { setDefaultTimeStep(static_cast<Real>(timeStep)); }

    ///
    /// \brief Set the default time step size, valid only if using a fixed time step for integration
    ///
    void setDefaultTimeStep(const Real timeStep) { m_DefaultDt = static_cast<Real>(timeStep); }

    ///
    /// \brief Returns the time step size
    ///
    virtual double getTimeStep() const override { return static_cast<double>(m_dt); }

    ///
    /// \brief Do one time step simulation
    ///
    void simulationTimeStep();


private:
    // time integration functions ===>

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
    /// \brief Collect the densities of neighbor particles, called after all density computation (after density normalizaton, if applicable)
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
    /// \brief Compute surface normal for each particle
    ///
    void computeNormal();

    ///
    /// \brief Compute surface tension and update velocities
    ///
    void computeSurfaceTension();

    ///
    /// \brief Advect particles
    ///
    void advect(const Real timestep);

    std::shared_ptr<PointSet> m_Geometry;
    SPHSimulationState m_SimulationState;

    Real m_dt;        ///> time step size
    Real m_DefaultDt; ///> default time step size

    std::shared_ptr<SPHModelConfig> m_Parameters; // must be set before simulation
    SPHSimulationKernels m_Kernels;               // must be initialized during model initialization
    NeighborSearch m_NeighborSearch;              // must be initialized during model initialization
};
} // end namespace imstk
