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

#include "imstkCollisionPair.h"

class SECompartment;

namespace imstk
{
  class SPHObject;
  class PhysiologyObject;
  class SPHModel;
  class PhysiologyModel;
  class Hemorrhage;

  ///
  /// \class SPHPhysiologyObjectInteractionPair
  ///
  /// \brief This class defines an interaction between SPH and physiology (Pulse)
  ///
  class SPHPhysiologyObjectInteractionPair : public ObjectInteractionPair
  {
  public:
    SPHPhysiologyObjectInteractionPair(std::shared_ptr<SPHObject> obj1, std::shared_ptr<PhysiologyObject> obj2);

    void apply() override;

    ///
    /// \brief Computes hemorrhage interaction between SPH and physiology (Pulse)
    ///
    void computeInteraction();

    ///
    /// \brief Set the Pulse hemorrhage action that should be connected to the SPH hemorrhage model
    ///
    void setHemorrhageAction(std::shared_ptr<Hemorrhage> h) { m_hemorrhageAction = h; };

    ///
    /// \brief Set the pulse compartment of the body where the hemorrhage is happening
    ///
    void setCompartment(const SECompartment* c) { m_compartment = c; };

  private:
    Inputs  m_solveNodeInputs;
    Outputs m_solveNodeOutputs;
    std::shared_ptr<SPHModel> m_sphModel = nullptr;
    std::shared_ptr<PhysiologyModel> m_physiologyModel = nullptr;
    std::shared_ptr<TaskNode> m_bcNode = nullptr;

    std::shared_ptr<Hemorrhage> m_hemorrhageAction;
    const SECompartment* m_compartment;
  };
}