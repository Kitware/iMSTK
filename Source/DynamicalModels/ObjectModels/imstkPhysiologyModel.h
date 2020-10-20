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
class SEHemorrhage;
class SEDecimalFormat;
class SEPatientAction;
class VolumePerTimeUnit;
class SECompartment;


namespace imstk
{
    using PhysiologyDataRequestPair = std::pair<std::string, SEDecimalFormat*>;

    enum class patientPhysiology
    {
        StandardMale=0,
        StandardFemale
    };

    enum class physiologyCompartmentType
    {
        Gas = 0,
        Liquid,
        Thermal,
        Tissue
    };

  class PhysiologyAction
  {
  public:
      virtual std::shared_ptr<SEPatientAction> getAction()=0;
  };

  class Hemorrhage :  public PhysiologyAction
  {
  public:
      enum class Type{ External = 0, Internal };

      Hemorrhage(const Type t, const std::string& name) 
      {
          this->setType(t);
          this->SetCompartment(name);
      }

      ///
      /// \brief Set the rate of hemorrhage
      ///
      void setRate(double val /*in milliLiters/sec*/);

      ///
      /// \brief Set the rate of hemorrhage
      ///
      void setType(const Type t);

      ///
      /// \brief Set the vascular compartment for hemorrhage
      /// The string is expected to be pulse::VascularCompartment::<name_of_compartment>
      /// (Refer: PulsePhysiologyEngine.h)
      ///
      void SetCompartment(const std::string& name);

      ///
      /// \brief Get the rate of hemorrhage in milliLiters/sec
      ///
      double getRate() const;

      ///
      /// \brief 
      ///
      std::shared_ptr<SEPatientAction> getAction();

  protected:
      std::shared_ptr<SEHemorrhage> m_hemorrhage;
  };

  
  ///
  /// \struct PhysiologyModelConfig
  /// \brief Contains physiology model settings
  ///
  struct PhysiologyModelConfig
  {
      patientPhysiology m_basePatient = patientPhysiology::StandardMale;

      double m_timeStep = 0.02;     ///< Pulse time step
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
    void addAction(std::shared_ptr<PhysiologyAction> action) { m_actions.push_back(action); };

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
    const SECompartment* getCompartment(const physiologyCompartmentType type, const std::string& compartmentName);

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

    std::vector<std::shared_ptr<PhysiologyAction>> m_actions; ///< container for all the actions
    std::vector<PhysiologyDataRequestPair> m_dataPairs; ///< container for data requests

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
