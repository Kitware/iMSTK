/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

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
} // namespace imstk