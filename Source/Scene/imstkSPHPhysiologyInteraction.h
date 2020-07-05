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

namespace imstk
{
  class SPHObject;
  class PhysiologyObject;
  class PbdSolver;
  class SPHModel;
  class PhysiologyModel;

  ///
  /// \class PbdObjectCollisionPair
  ///
  /// \brief This class defines a collision interaction between two PbdObjects
  ///
  class SPHPhysiologyObjectInteractionPair : public ObjectInteractionPair
  {
  public:
    SPHPhysiologyObjectInteractionPair(std::shared_ptr<SPHObject> obj1, std::shared_ptr<PhysiologyObject> obj2);

    void apply() override;

    //void bcNode();

    void computeInletBCs();

  private:
    // Pbd defines two interactions (one at CD and one at solver)
    Inputs  m_solveNodeInputs;
    Outputs m_solveNodeOutputs;
    std::shared_ptr<SPHModel> m_sphModel1 = nullptr;
    std::shared_ptr<PhysiologyModel> m_physiologyModel1 = nullptr;

    // Define where the boundary condition happens
    //m_taskNodeInputs.first.push_back(sphModel1->getIntegrateNode());
    //m_taskNodeInputs.second.push_back(physiologyModel1->getSolveNode());

    std::shared_ptr<TaskNode> m_bcNode = nullptr;
    std::shared_ptr<TaskNode> m_boundaryConditionNode = nullptr;
  };
}