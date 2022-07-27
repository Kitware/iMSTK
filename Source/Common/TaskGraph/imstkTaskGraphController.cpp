/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkTaskGraphController.h"
#include "imstkTaskGraph.h"
#include "imstkLogger.h"

namespace imstk
{
bool
TaskGraphController::initialize()
{
    // Ensure the source is reachable from the sink and the graph is not cyclic
    // Still doesn't garuntee all nodes reach sink
    if (!m_graph->isReachable(m_graph->getSource(), m_graph->getSink()))
    {
        LOG(WARNING) << "TaskGraph Sink not reachable from source. Graph initialization failed.";
        return false;
    }

    if (TaskGraph::isCyclic(m_graph))
    {
        LOG(WARNING) << "TaskGraph is cyclic. Graph initialization failed.";
        return false;
    }

    init();
    return true;
}
} // namespace imstk