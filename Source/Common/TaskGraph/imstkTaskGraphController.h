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

namespace imstk
{
class TaskGraph;

///
/// \class TaskGraphController
///
/// \brief Base class for TaskGraph controllers which are responsible for executing the TaskGraph
///
class TaskGraphController
{
public:
    TaskGraphController() = default;
    virtual ~TaskGraphController() = default;

public:
    virtual void setTaskGraph(std::shared_ptr<TaskGraph> graph) { this->m_graph = graph; }

    std::shared_ptr<TaskGraph> getTaskGraph() const { return m_graph; }

    ///
    /// \brief Initialization of the TaskGraphController, good for anything the controller may need
    /// to do after it recieves input, before execution, but not do everytime execution is called.
    /// Returns if successful
    ///
    bool initialize();

    ///
    /// \brief Executes the TaskGraph
    ///
    virtual void execute() = 0;

protected:
    ///
    /// \brief Subclass initialization call
    ///
    virtual void init() { }

protected:
    std::shared_ptr<TaskGraph> m_graph = nullptr;
};
}