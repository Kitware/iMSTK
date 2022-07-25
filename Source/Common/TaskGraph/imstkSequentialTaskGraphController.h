/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

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