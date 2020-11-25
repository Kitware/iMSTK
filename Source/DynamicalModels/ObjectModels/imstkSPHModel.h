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

#include "imstkDynamicalModel.h"
#include "imstkSPHState.h"
#include "imstkSPHKernels.h"
#include "imstkNeighborSearch.h"
#include "imstkSPHBoundaryConditions.h"
#include "imstkSPHHemorrhage.h"
#include "imstkTetrahedralMesh.h"

namespace imstk
{
class PointSet;

///
/// \class SPHModelConfig
/// \brief Class that holds the SPH model parameters
///
class SPHModelConfig
{
private:
    void initialize();

public:
    explicit SPHModelConfig(const Real particleRadius);
    explicit SPHModelConfig(const Real particleRadius, const Real speedOfSound, const Real restDensity);

    /// \todo Move this to solver or time integrator in the future
    Real m_minTimestep = Real(1e-6);
    Real m_maxTimestep = Real(1e-3);
    Real m_CFLFactor   = Real(1.0);

    // particle parameters
    Real m_particleRadius    = Real(0);
    Real m_particleRadiusSqr = Real(0); ///> \note derived quantity

    // material parameters
    Real m_restDensity       = Real(1000);
    Real m_restDensitySqr    = Real(1000000.0);    ///> \note derived quantity
    Real m_restDensityInv    = Real(1.0 / 1000.0); ///> \note derived quantity
    Real m_particleMass      = Real(1);
    Real m_particleMassScale = Real(1.0);          ///> scale particle mass to a smaller value to maintain stability
    Real m_eta = Real(0.5);                        ///> proportion of position change due to neighbors velocity (XSPH method)

    bool m_bNormalizeDensity    = false;
    bool m_bDensityWithBoundary = false;

    // pressure
    Real m_pressureStiffness = Real(50000.0);

    // viscosity and surface tension/cohesion
    Real m_dynamicViscosityCoeff   = Real(1e-2);
    Real m_viscosityBoundary       = Real(1e-5);
    Real m_surfaceTensionStiffness = Real(1);
    Real m_frictionBoundary = Real(0.1);

    // kernel properties
    Real m_kernelOverParticleRadiusRatio = Real(4.0);
    Real m_kernelRadius;    ///> \note derived quantity
    Real m_kernelRadiusSqr; ///> \note derived quantity

    // gravity
    Vec3r m_gravity = Vec3r(0, -9.81, 0);

    // sound speed
    Real m_speedOfSound = 18.7;

    // neighbor search
    NeighborSearch::Method m_NeighborSearchMethod = NeighborSearch::Method::UniformGridBasedSearch;
};

///
/// \class SPHModel
/// \brief SPH fluid model
///
class SPHModel : public DynamicalModel<SPHState>
{
public:
    ///
    /// \brief Constructor
    ///
    SPHModel();

    ///
    /// \brief Destructor
    ///
    virtual ~SPHModel() override = default;

public:
    ///
    /// \brief Set simulation parameters
    ///
    void configure(const std::shared_ptr<SPHModelConfig>& params) { m_modelParameters = params; }

    ///
    /// \brief Initialize the dynamical model
    ///
    virtual bool initialize() override;

    ///
    /// \brief Reset the current state to the initial state
    ///
    virtual void resetToInitialState() override { this->m_currentState->setState(this->m_initialState); }

    ///
    /// \brief Get the simulation parameters
    ///
    const std::shared_ptr<SPHModelConfig>& getParameters() const
    {
        assert(m_modelParameters);
        return m_modelParameters;
    }

    ///
    /// \brief Set the default time step size,
    /// valid only if using a fixed time step for integration
    ///
    virtual void setTimeStep(const double timeStep) override { setDefaultTimeStep(timeStep); }

    ///
    /// \brief Set the default time step size,
    /// valid only if using a fixed time step for integration
    ///
    void setDefaultTimeStep(const double timeStep) { m_defaultDt = timeStep; }

    ///
    /// \brief Returns the time step size
    ///
    virtual double getTimeStep() const override { return m_dt; }

    void setInitialVelocities(const size_t numParticles, const Vec3d& initialVelocities);

    double particlePressure(const double density);

    ///
    /// \brief Write the state to external file
    /// \todo move this out of this class
    ///
    void setWriteToOutputModulo(const double modulo) { m_writeToOutputModulo = modulo; }
    double getTotalTime() const { return m_totalTime; }
    int getTimeStepCount() const { return m_timeStepCount; }
    void writeStateToVtk();
    void setGeometryMesh(std::shared_ptr<TetrahedralMesh>& geometryMesh) { m_geomUnstructuredGrid = geometryMesh; }
    void findNearestParticleToVertex(const VecDataArray<double, 3>& points, const std::vector<std::vector<size_t>>& indices);

    void setBoundaryConditions(std::shared_ptr<SPHBoundaryConditions> sphBoundaryConditions) { m_sphBoundaryConditions = sphBoundaryConditions; }
    std::shared_ptr<SPHBoundaryConditions> getBoundaryConditions() { return m_sphBoundaryConditions; }

    void setHemorrhageModel(std::shared_ptr<SPHHemorrhage> sPHHemorrhage) { m_SPHHemorrhage = sPHHemorrhage; }
    std::shared_ptr<SPHHemorrhage> getHemorrhageModel() { return m_SPHHemorrhage; }

    void setRestDensity(const Real restDensity) { m_modelParameters->m_restDensity = restDensity; }

    std::shared_ptr<TaskNode> getFindParticleNeighborsNode() const { return m_findParticleNeighborsNode; }
    std::shared_ptr<TaskNode> getComputeDensityNode() const { return m_computeDensityNode; }
    std::shared_ptr<TaskNode> getComputePressureNode() const { return m_computePressureAccelNode; }
    std::shared_ptr<TaskNode> getComputeSurfaceTensionNode() const { return m_computeSurfaceTensionNode; }
    std::shared_ptr<TaskNode> getComputeTimeStepSizeNode() const { m_computeTimeStepSizeNode; }
    std::shared_ptr<TaskNode> getSumAccelsNode() const { m_sumAccelsNode; }
    std::shared_ptr<TaskNode> getIntegrateNode() const { return m_integrateNode; }
    std::shared_ptr<TaskNode> getComputeViscosityNode() const { return m_computeViscosityNode; }
    std::shared_ptr<TaskNode> getUpdateVelocityNode() const { return m_updateVelocityNode; }
    std::shared_ptr<TaskNode> getMoveParticlesNode() const { return m_moveParticlesNode; }

protected:
    ///
    /// \brief Setup SPH compute graph connectivity
    ///
    virtual void initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink) override;

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
    /// \brief Sum the forces computed in parallel
    ///
    void sumAccels();

    ///
    /// \brief Update particle velocities due to pressure, viscous, and surface tension forces
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

    double m_dt = 0.0;                                  ///> time step size
    double m_defaultDt;                                 ///> default time step size

    SPHSimulationKernels m_kernels;                     ///> SPH kernels (must be initialized during model initialization)
    std::shared_ptr<SPHModelConfig> m_modelParameters;  ///> SPH Model parameters (must be set before simulation)
    std::shared_ptr<NeighborSearch> m_neighborSearcher; ///> Neighbor Search (must be initialized during model initialization)

    std::shared_ptr<VecDataArray<double, 3>> m_pressureAccels       = nullptr;
    std::shared_ptr<VecDataArray<double, 3>> m_surfaceTensionAccels = nullptr;
    std::shared_ptr<VecDataArray<double, 3>> m_viscousAccels    = nullptr;
    std::shared_ptr<VecDataArray<double, 3>> m_neighborVelContr = nullptr;
    std::shared_ptr<VecDataArray<double, 3>> m_particleShift    = nullptr;

    std::shared_ptr<VecDataArray<double, 3>> m_initialVelocities = nullptr;
    std::shared_ptr<DataArray<double>>       m_initialDensities  = nullptr;

    double m_totalTime           = 0;
    int    m_timeStepCount       = 0;
    double m_writeToOutputModulo = 0;
    double m_vtkPreviousTime     = 0;
    double m_vtkTimeModulo       = 0;
    double m_csvPreviousTime     = 0;
    double m_csvTimeModulo       = 0;
    Vec3d  m_prevAvgVelThroughHemorrhage = Vec3d(0., 0., 0.);

    std::shared_ptr<TetrahedralMesh>       m_geomUnstructuredGrid  = nullptr;
    std::shared_ptr<SPHBoundaryConditions> m_sphBoundaryConditions = nullptr;
    // \todo: Should be refactored out of the base SPHModel
    std::shared_ptr<SPHHemorrhage> m_SPHHemorrhage = nullptr;

    std::vector<size_t> m_minIndices;
};
} // end namespace imstk
