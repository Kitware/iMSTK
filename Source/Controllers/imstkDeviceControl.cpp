/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkDeviceControl.h"
#include "imstkTaskGraph.h"
#include "imstkTaskNode.h"

namespace imstk
{
void
DeviceControl::initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink)
{
    // Remove the update node from the TaskGraph, updates invoked manually
    m_taskGraph->addEdge(source, sink);
}
} // namespace imstk