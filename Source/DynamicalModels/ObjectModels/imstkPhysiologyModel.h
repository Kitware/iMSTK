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

#include "imstkAbstractDynamicalModel.h"


class PhysiologyEngine;
class SELiquidCompartment;
class SEHemorrhage;
//class SEScalarTime;


namespace imstk
{
///
/// \class PhysiologyModelConfig
/// \brief Empty for now but we can populate config params if needed
///
class PhysiologyModelConfig
{
private:
    void initialize();

public:
    explicit PhysiologyModelConfig();
};

///
/// \class PhysiologyModel
/// \brief PhysiologyModel model
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
    void configure(const std::shared_ptr<PhysiologyModelConfig>& params) { m_modelParameters = params; }

    ///
    /// \brief Initialize the dynamical model
    ///
    virtual bool initialize() override;

    std::shared_ptr<TaskNode> getSolveNode() const { return m_solveNode; }

    virtual void setTimeStep(const double) override {}

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
    /// \brief Returns the time step size
    ///
    virtual double getTimeStep() const override { return m_dT_s; }

    virtual void resetToInitialState() override {};

    ///
    /// \brief Returns the flow rate in the femoral artery
    ///
    const double getFemoralFlowRate() { return m_femoralFlowRate; }
    //const std::shared_ptr<SELiquidCompartment> getFemoralCompartment();
    //const std::shared_ptr<SEHemorrhage> getHemorrhageModel();

    ///
    /// \brief Returns the hemorrhage flow rate
    ///
    const double getHemorrhageRate() { return m_hemorrhageRate; }
    void setHemorrhageRate(const double hemorrhageRate) { m_hemorrhageRate = hemorrhageRate; }

    void setPulseTimeStep(const double timeStep) { m_dT_s = timeStep; }

    //virtual SEScalarTime& GetTimeStep();
    //virtual double GetTimeStep(const TimeUnit& unit) const;

protected:
    ///
    /// \brief Setup physiology compute graph connectivity
    ///
    virtual void initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink) override;

    void solvePulse();

    std::shared_ptr<TaskNode> m_solveNode = nullptr;

private:
    // Physiology Model parameters (must be set before simulation)
    // empty for now but can be set if  needed
    std::shared_ptr<PhysiologyModelConfig> m_modelParameters;

    // main pulse object
    std::unique_ptr<PhysiologyEngine> m_pulseObj = nullptr;
    
    // Pulse hemorrhage action
    // here, we can add any Pulse actions that we want
    std::shared_ptr<SEHemorrhage> m_hemorrhageLeg = nullptr;

    // Pulse time step
    double m_dT_s = 0.02;

    //SEScalarTime* m_timeStep;

    // sample the femoral compartment
    const SELiquidCompartment* m_femoralCompartment;

    double m_femoralFlowRate = 0;
    double m_hemorrhageRate = 0;

};
} // end namespace imstk
