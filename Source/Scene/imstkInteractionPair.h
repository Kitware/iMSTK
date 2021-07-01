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

#include <memory>
#include <vector>

namespace imstk
{
class TaskNode;

///
/// \class InteractionPair
///
/// \brief This class defines an interaction between 2 sets of nodes
/// Given a set of input and output nodes, the interactionFunction will be
/// inserted
///
class InteractionPair
{
public:
    using Inputs  = std::pair<std::vector<std::shared_ptr<TaskNode>>, std::vector<std::shared_ptr<TaskNode>>>;
    using Outputs = std::pair<std::vector<std::shared_ptr<TaskNode>>, std::vector<std::shared_ptr<TaskNode>>>;

public:
    InteractionPair() = default;
    virtual ~InteractionPair() = default;

public:
    const Inputs& getTaskNodeInputs() const { return m_taskNodeInputs; }
    const Outputs& getTaskNodeOutputs() const { return m_taskNodeOutputs; }

public:
///
/// \brief Modifies the computational graph
///
//void compute();

protected:
    Inputs  m_taskNodeInputs;                        ///> The interacting nodes
    Outputs m_taskNodeOutputs;                       ///> The interacting nodes
    std::shared_ptr<TaskNode> m_interactionFunction; ///> Function to execute on interaction
};
}
