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

/////////////////////////////////////////////////////////////////////////
// These includes are temporary for testing Pulse in iMSTK
#include "PulsePhysiologyEngine.h"
#include "engine/SEEngineTracker.h"
#include "engine/SEDataRequest.h"
#include "properties/SEScalarTime.h"
#include "CommonDataModel.h"
#include "engine/SEDataRequestManager.h"
#include "engine/SEEngineTracker.h"
#include "compartment/SECompartmentManager.h"
#include "patient/actions/SEHemorrhage.h"
#include "patient/actions/SESubstanceCompoundInfusion.h"
#include "system/physiology/SEBloodChemistrySystem.h"
#include "system/physiology/SECardiovascularSystem.h"
#include "system/physiology/SEEnergySystem.h"
#include "system/physiology/SERespiratorySystem.h"
#include "substance/SESubstanceManager.h"
#include "substance/SESubstanceCompound.h"
#include "properties/SEScalar0To1.h"
#include "properties/SEScalarFrequency.h"
#include "properties/SEScalarMass.h"
#include "properties/SEScalarMassPerVolume.h"
#include "properties/SEScalarPressure.h"
#include "properties/SEScalarTemperature.h"
#include "properties/SEScalarTime.h"
#include "properties/SEScalarVolume.h"
#include "properties/SEScalarVolumePerTime.h"
/////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// Temporary class for testing Pulse in iMSTK
class HowToTracker
{
private:
  double m_dT_s;  // Cached Engine Time Step
  PhysiologyEngine& m_Engine;
public:
  HowToTracker(PhysiologyEngine& engine) : m_Engine(engine) 
  {
    m_dT_s = m_Engine.GetTimeStep(TimeUnit::s);
  };
  ~HowToTracker() {};

  // This class will operate on seconds
  void AdvanceModelTime(double time_s) {
    // This samples the engine at each time step
    int count = static_cast<int>(time_s / m_dT_s);
    for (int i = 0; i < count; i++)
    {
      m_Engine.AdvanceModelTime();  // Compute 1 time step

      // Pull Track will pull data from the engine and append it to the file
      m_Engine.GetEngineTracker()->TrackData(m_Engine.GetSimulationTime(TimeUnit::s));
    };
  }
};
////////////////////////////////////////////////////////////////

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
    ///////////////////////////////////////////////////////////////////////////////////
    // Temporary method to test that Pulse is working in iMSTK with hemorrhage model

    // Create the engine and load the patient
    std::unique_ptr<PhysiologyEngine> pe = CreatePulseEngine();
    pe->GetLogger()->SetLogFile("./test_results/HowTo_Hemorrhage.log");
    pe->GetLogger()->Info("HowTo_Hemorrhage");
    if (!pe->SerializeFromFile(iMSTK_DATA_ROOT "/states/StandardMale@0s.json", JSON))
    {
      pe->GetLogger()->Error("Could not load state, check the error");
      return;
    }

    // The tracker is responsible for advancing the engine time and outputting the data requests below at each time step
    HowToTracker tracker(*pe);

    // Create data requests for each value that should be written to the output log as the engine is executing
    pe->GetEngineTracker()->GetDataRequestManager().CreatePhysiologyDataRequest("HeartRate", FrequencyUnit::Per_min);
    pe->GetEngineTracker()->GetDataRequestManager().SetResultsFilename("HowToHemorrhage.csv");

    pe->GetLogger()->Info("The patient is nice and healthy");
    pe->GetLogger()->Info(std::stringstream() << "Cardiac Output : " << pe->GetCardiovascularSystem()->GetCardiacOutput(VolumePerTimeUnit::mL_Per_min) << VolumePerTimeUnit::mL_Per_min);
    pe->GetLogger()->Info(std::stringstream() << "Hemoglobin Content : " << pe->GetBloodChemistrySystem()->GetHemoglobinContent(MassUnit::g) << MassUnit::g);
    pe->GetLogger()->Info(std::stringstream() << "Blood Volume : " << pe->GetCardiovascularSystem()->GetBloodVolume(VolumeUnit::mL) << VolumeUnit::mL);
    pe->GetLogger()->Info(std::stringstream() << "Mean Arterial Pressure : " << pe->GetCardiovascularSystem()->GetMeanArterialPressure(PressureUnit::mmHg) << PressureUnit::mmHg);
    pe->GetLogger()->Info(std::stringstream() << "Systolic Pressure : " << pe->GetCardiovascularSystem()->GetSystolicArterialPressure(PressureUnit::mmHg) << PressureUnit::mmHg);
    pe->GetLogger()->Info(std::stringstream() << "Diastolic Pressure : " << pe->GetCardiovascularSystem()->GetDiastolicArterialPressure(PressureUnit::mmHg) << PressureUnit::mmHg);
    pe->GetLogger()->Info(std::stringstream() << "Heart Rate : " << pe->GetCardiovascularSystem()->GetHeartRate(FrequencyUnit::Per_min) << "bpm");

    // Hemorrhage Starts - instantiate a hemorrhage action and have the engine process it
    SEHemorrhage hemorrhageLeg;
    hemorrhageLeg.SetType(eHemorrhage_Type::External);
    hemorrhageLeg.SetCompartment(pulse::VascularCompartment::RightLeg);//the location of the hemorrhage
    hemorrhageLeg.GetRate().SetValue(250, VolumePerTimeUnit::mL_Per_min);//the rate of hemorrhage
    pe->ProcessAction(hemorrhageLeg);

    // Advance some time to let the body bleed out a bit
    tracker.AdvanceModelTime(3);

    pe->GetLogger()->Info("The patient has been hemorrhaging for 3s");
    pe->GetLogger()->Info(std::stringstream() << "Cardiac Output : " << pe->GetCardiovascularSystem()->GetCardiacOutput(VolumePerTimeUnit::mL_Per_min) << VolumePerTimeUnit::mL_Per_min);
    pe->GetLogger()->Info(std::stringstream() << "Hemoglobin Content : " << pe->GetBloodChemistrySystem()->GetHemoglobinContent(MassUnit::g) << MassUnit::g);
    pe->GetLogger()->Info(std::stringstream() << "Blood Volume : " << pe->GetCardiovascularSystem()->GetBloodVolume(VolumeUnit::mL) << VolumeUnit::mL);
    pe->GetLogger()->Info(std::stringstream() << "Mean Arterial Pressure : " << pe->GetCardiovascularSystem()->GetMeanArterialPressure(PressureUnit::mmHg) << PressureUnit::mmHg);
    pe->GetLogger()->Info(std::stringstream() << "Systolic Pressure : " << pe->GetCardiovascularSystem()->GetSystolicArterialPressure(PressureUnit::mmHg) << PressureUnit::mmHg);
    pe->GetLogger()->Info(std::stringstream() << "Diastolic Pressure : " << pe->GetCardiovascularSystem()->GetDiastolicArterialPressure(PressureUnit::mmHg) << PressureUnit::mmHg);
    pe->GetLogger()->Info(std::stringstream() << "Heart Rate : " << pe->GetCardiovascularSystem()->GetHeartRate(FrequencyUnit::Per_min) << "bpm");;

    // Hemorrhage is sealed
    hemorrhageLeg.SetType(eHemorrhage_Type::External);
    hemorrhageLeg.SetCompartment(pulse::VascularCompartment::RightLeg);//location of hemorrhage
    hemorrhageLeg.GetRate().SetValue(0, VolumePerTimeUnit::mL_Per_min);//rate is set to 0 to close the bleed
    pe->ProcessAction(hemorrhageLeg);

    // Advance some time while the medic gets the drugs ready
    tracker.AdvanceModelTime(5);

    pe->GetLogger()->Info("The patient has NOT been hemorrhaging for 100s");
    pe->GetLogger()->Info(std::stringstream() << "Cardiac Output : " << pe->GetCardiovascularSystem()->GetCardiacOutput(VolumePerTimeUnit::mL_Per_min) << VolumePerTimeUnit::mL_Per_min);
    pe->GetLogger()->Info(std::stringstream() << "Hemoglobin Content : " << pe->GetBloodChemistrySystem()->GetHemoglobinContent(MassUnit::g) << MassUnit::g);
    pe->GetLogger()->Info(std::stringstream() << "Blood Volume : " << pe->GetCardiovascularSystem()->GetBloodVolume(VolumeUnit::mL) << VolumeUnit::mL);
    pe->GetLogger()->Info(std::stringstream() << "Mean Arterial Pressure : " << pe->GetCardiovascularSystem()->GetMeanArterialPressure(PressureUnit::mmHg) << PressureUnit::mmHg);
    pe->GetLogger()->Info(std::stringstream() << "Systolic Pressure : " << pe->GetCardiovascularSystem()->GetSystolicArterialPressure(PressureUnit::mmHg) << PressureUnit::mmHg);
    pe->GetLogger()->Info(std::stringstream() << "Diastolic Pressure : " << pe->GetCardiovascularSystem()->GetDiastolicArterialPressure(PressureUnit::mmHg) << PressureUnit::mmHg);
    pe->GetLogger()->Info(std::stringstream() << "Heart Rate : " << pe->GetCardiovascularSystem()->GetHeartRate(FrequencyUnit::Per_min) << "bpm");;

    // Patient is stabilizing, but not great

    // Let's administer a saline drip, we need to get saline from the substance maganer
    SESubstanceCompound* saline = pe->GetSubstanceManager().GetCompound("Saline");
    SESubstanceCompoundInfusion iVSaline(*saline);
    iVSaline.GetBagVolume().SetValue(500, VolumeUnit::mL);//the total volume in the bag of Saline
    iVSaline.GetRate().SetValue(100, VolumePerTimeUnit::mL_Per_min);//The rate to admnister the compound in the bag in this case saline
    pe->ProcessAction(iVSaline);

    tracker.AdvanceModelTime(5);

    pe->GetLogger()->Info("The patient has been getting fluids for the past 5s");
    pe->GetLogger()->Info(std::stringstream() << "Cardiac Output : " << pe->GetCardiovascularSystem()->GetCardiacOutput(VolumePerTimeUnit::mL_Per_min) << VolumePerTimeUnit::mL_Per_min);
    pe->GetLogger()->Info(std::stringstream() << "Hemoglobin Content : " << pe->GetBloodChemistrySystem()->GetHemoglobinContent(MassUnit::g) << MassUnit::g);
    pe->GetLogger()->Info(std::stringstream() << "Blood Volume : " << pe->GetCardiovascularSystem()->GetBloodVolume(VolumeUnit::mL) << VolumeUnit::mL);
    pe->GetLogger()->Info(std::stringstream() << "Mean Arterial Pressure : " << pe->GetCardiovascularSystem()->GetMeanArterialPressure(PressureUnit::mmHg) << PressureUnit::mmHg);
    pe->GetLogger()->Info(std::stringstream() << "Systolic Pressure : " << pe->GetCardiovascularSystem()->GetSystolicArterialPressure(PressureUnit::mmHg) << PressureUnit::mmHg);
    pe->GetLogger()->Info(std::stringstream() << "Diastolic Pressure : " << pe->GetCardiovascularSystem()->GetDiastolicArterialPressure(PressureUnit::mmHg) << PressureUnit::mmHg);
    pe->GetLogger()->Info(std::stringstream() << "Heart Rate : " << pe->GetCardiovascularSystem()->GetHeartRate(FrequencyUnit::Per_min) << "bpm");;
    pe->GetLogger()->Info("Finished");

    // end Pulse hemorrhage test
    /////////////////////////////////////////////////////////////////////////////////////////////////

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
            updateVelocity(getTimeStep());
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
    this->m_initialState->setParticleData(m_pointSetGeometry->getVertexPositions());
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
    m_surfaceTensionAccels = std::make_shared<StdVectorOfVec3d>(getState().getNumParticles());

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
    auto particlePressure = [&](const Real density) {
                                const Real error = std::pow(density / m_modelParameters->m_restDensity, 7) - Real(1);
                                // clamp pressure error to zero to maintain stability
                                return error > Real(0) ? error : Real(0);
                            };

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
        });
}
} // end namespace imstk
