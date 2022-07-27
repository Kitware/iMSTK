/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkSequentialTaskGraphController.h"
#include "imstkTaskGraph.h"

namespace imstk
{
void
SequentialTaskGraphController::init()
{
    m_executionOrderedNodes = TaskGraph::topologicalSort(m_graph);
}

void
SequentialTaskGraphController::execute()
{
    // Sequential
    for (std::shared_ptr<TaskNode> node : *m_executionOrderedNodes)
    {
        node->execute();
    }
}
} // namespace imstk