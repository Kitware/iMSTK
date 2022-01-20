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