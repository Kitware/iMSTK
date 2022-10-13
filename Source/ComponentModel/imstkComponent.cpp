/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkComponent.h"
#include "imstkLogger.h"
#include "imstkTaskGraph.h"

namespace imstk
{
void
Component::initialize()
{
    // Generally the callee should only initialize entities
    CHECK(m_entity.lock() != nullptr) << "Tried to initialize a component that doesn't exist"
        " on any entity.";

    // Initialize
    init();
}

TaskBehaviour::TaskBehaviour(const std::string& name) : Component(name),
    m_taskGraph(std::make_shared<TaskGraph>())
{
}

void
TaskBehaviour::initGraphEdges()
{
    m_taskGraph->clearEdges();
    initGraphEdges(m_taskGraph->getSource(), m_taskGraph->getSink());
}

void
LambdaBehaviour::update(const double& dt)
{
    if (m_updateFunc != nullptr)
    {
        m_updateFunc(dt);
    }
}

void
LambdaBehaviour::visualUpdate(const double& dt)
{
    if (m_visualUpdateFunc != nullptr)
    {
        m_visualUpdateFunc(dt);
    }
}
} // namespace imstk