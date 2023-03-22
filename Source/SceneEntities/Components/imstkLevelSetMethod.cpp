/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkGeometry.h"
#include "imstkGeometryMap.h"
#include "imstkLevelSetMethod.h"
#include "imstkLevelSetSystem.h"

namespace imstk
{
LevelSetMethod::LevelSetMethod(const std::string& name) : SceneBehaviour(name)
{
    const std::string prefix = getTypeName() + "_" + m_name;
    m_taskGraph = std::make_shared<TaskGraph>(prefix + "_Source", prefix + "_Sink");

    m_updateNode = m_taskGraph->addFunction(
        prefix + "_Update",
        [this]() {}
        );

    m_updateGeometryNode = m_taskGraph->addFunction(
        prefix + "_UpdateGeometry",
        [this]() { updateGeometries(); }
        );
}

void
LevelSetMethod::init()
{
    if (m_system == nullptr)
    {
        LOG(FATAL) << "Dynamics pointer cast failure in LevelSetDeformableObject_old::initialize()";
        return;
    }

    CHECK(m_physicsGeometry != nullptr) << "LevelSetMethod \"" << m_name
                                        << "\" expects a physics geometry at start, none was provided";

    if (m_physicsToCollidingGeomMap)
    {
        m_physicsToCollidingGeomMap->compute();
    }

    if (m_physicsToVisualGeomMap)
    {
        m_physicsToVisualGeomMap->compute();
    }
}

void
LevelSetMethod::updateGeometries()
{
    updatePhysicsGeometry();

    if (m_physicsToCollidingGeomMap)
    {
        m_physicsToCollidingGeomMap->update();
        m_physicsToCollidingGeomMap->getChildGeometry()->postModified();
    }

    if (m_physicsToVisualGeomMap)
    {
        m_physicsToVisualGeomMap->update();
        m_physicsToVisualGeomMap->getChildGeometry()->postModified();
    }
}

void
LevelSetMethod::updatePhysicsGeometry()
{
    m_system->updatePhysicsGeometry();
    if (m_physicsGeometry != nullptr)
    {
        m_physicsGeometry->postModified();
    }
}

void
LevelSetMethod::reset()
{
    m_system->resetToInitialState();
    updateGeometries();
    postModifiedAll();
}

void
LevelSetMethod::postModifiedAll()
{
    if (m_physicsGeometry != nullptr)
    {
        m_physicsGeometry->postModified();
    }
}

void
LevelSetMethod::initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink)
{
    // Copy, sum, and connect the model graph to nest within this graph
    m_taskGraph->addEdge(source, m_updateNode);
    if (m_system != nullptr)
    {
        m_system->initGraphEdges();
        m_taskGraph->nestGraph(m_system->getTaskGraph(), m_updateNode, m_updateGeometryNode);
    }
    else
    {
        m_taskGraph->addEdge(m_updateNode, m_updateGeometryNode);
    }
    m_taskGraph->addEdge(m_updateGeometryNode, sink);
}
} // namespace imstk
