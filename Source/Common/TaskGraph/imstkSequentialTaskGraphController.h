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

#include "imstkTaskGraphController.h"
#include <list>

namespace imstk
{
class TaskNode;

///
/// \class SequentialTaskGraphController
///
/// \brief This class executes a TaskGraph by first topologically sorting them (Kahn's algorithm)
/// then sequentially running them
///
class SequentialTaskGraphController : public TaskGraphController
{
public:
    ///
    /// \brief Sorts the computational nodes
    ///
    void init() override;

    void execute() override;

private:
    // The current nodes to execute, ordered
    std::shared_ptr<std::list<std::shared_ptr<TaskNode>>> m_executionOrderedNodes;
};
}; // namespace imstk