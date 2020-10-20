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

#include "imstkAbstractDynamicalModel.h"/*
#include "properties/SEScalarVolumePerTime.h"
#include "compartment/fluid/SELiquidCompartment.h"*/

class PhysiologyEngine;
class SELiquidCompartment;
class SEDecimalFormat;
class SEPatientAction;
class VolumePerTimeUnit;
class SECompartment;

namespace imstk
{
using PhysiologyDataRequestPair = std::pair<std::string, SEDecimalFormat*>;

enum class PatientPhysiology
{
    StandardMale=0,
    StandardFemale
};

enum class PhysiologyCompartmentType
{
    Gas = 0,
    Liquid,
    Thermal,
    Tissue
};

class PhysiologyAction
{
public:
    virtual std::shared_ptr<SEPatientAction> getAction() = 0;
};

///
/// \struct PhysiologyModelConfig
/// \brief Contains physiology model settings
///
struct PhysiologyModelConfig
{
    PatientPhysiology m_basePatient = PatientPhysiology::StandardMale;

    double m_timeStep    = 0.02;  ///< Pulse time step
    bool m_enableLogging = false; ///< Enable Pulse engine logging
};

///
/// \class PhysiologyModel
/// \brief Human physiology dynamical model
///
class PhysiologyModel : public AbstractDynamicalModel
{
public:
    ///
    /// \brief Constructor
    ///
    PhysiologyModel();

    ///
    /// \brief Destructor
    ///
    virtual ~PhysiologyModel() override = default;

    ///
    /// \brief Set simulation parameters
    ///
    void configure(const std::shared_ptr<PhysiologyModelConfig>& params) { m_config = params; }

    ///
    /// \brief Initialize the dynamical model
    ///
    virtual bool initialize() override;

    ///
    /// \brief Get the solver task node
    ///
    std::shared_ptr<TaskNode> getSolveNode() const { return m_solveNode; }

    ///
    /// \brief Update states
    ///
    virtual void updateBodyStates(const Vectord&, const StateUpdateType) override {}

    ///
    /// \brief Set the default time step size,
    /// valid only if using a fixed time step for integration
    ///
    void setDefaultTimeStep(const Real) {}

    ///
    /// \brief Reset the physiology model to the initial state
    ///
    virtual void resetToInitialState() override {};

    ///
    /// \brief Add a data request that to output vitals to CSV files
    ///
    void addDataRequest(const std::string& property, SEDecimalFormat* dfault = nullptr);

    ///
    /// \brief Returns the time step size
    ///
    virtual double getTimeStep() const override { return m_config->m_timeStep; }

    ///
    /// \brief Set the time step of the pulse solver
    ///
    virtual void setTimeStep(const double t) override { m_config->m_timeStep = t; }

    ///
    /// \brief Add a new action
    ///
    void addAction(std::string name, std::shared_ptr<PhysiologyAction> action) { m_actions[name] = action; };

    std::shared_ptr<PhysiologyAction> getAction(std::string name) const { return (m_actions.count(name) == 0) ? nullptr : m_actions.at(name); }

    ///
    /// \brief Clear all actions
    ///
    void clearActions() { m_actions.clear(); }

    ///
    /// \brief Set the name of the file to write out the data requests
    ///
    void setDataWriteOutFileName(const std::string& filename) { m_dataWriteOutFile = filename; }

    ///
    /// \brief Get the physiology compartment model of the body
    ///
    const SECompartment* getCompartment(const PhysiologyCompartmentType type, const std::string& compartmentName) const;

protected:
    ///
    /// \brief Setup physiology compute graph connectivity
    ///
    virtual void initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink) override;

    ///
    /// \brief Advance one time step of the pulse engine solver
    ///
    void solve();

    std::shared_ptr<TaskNode> m_solveNode = nullptr;

    std::unordered_map<std::string, std::shared_ptr<PhysiologyAction>> m_actions; ///< container for all the actions
    std::vector<PhysiologyDataRequestPair> m_dataPairs;       ///< container for data requests

private:
    // main pulse object
    std::unique_ptr<PhysiologyEngine> m_pulseObj = nullptr;

    double m_currentTime = 0.;    ///< Current total time (incremented every solve)

    std::string m_dataWriteOutFile = "pulseVitals.csv";

    // Physiology Model parameters (must be set before simulation)
    // empty for now but can be set if  needed
    std::shared_ptr<PhysiologyModelConfig> m_config;
};
} // end namespace imstk
