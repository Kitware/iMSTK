/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkAbstractDynamicalSystem.h"
#include "imstkGeometry.h"
#include "imstkGeometryMap.h"
#include "imstkSphMethod.h"
#include "imstkSphSystem.h"

namespace imstk
{
SphMethod::SphMethod(const std::string& name) : SceneBehaviour(name)
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
SphMethod::init()
{
    if (!m_sphSystem)
    {
        LOG(FATAL) << "SPH system is required to be set before initialization.";
        return;
    }

    CHECK(m_physicsGeometry != nullptr) << "SphMethod \"" << m_name
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
SphMethod::updateGeometries()
{
    updatePhysicsGeometry();

    // SceneObject::updateGeometries();
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
SphMethod::updatePhysicsGeometry()
{
    m_sphSystem->updatePhysicsGeometry();
    if (m_physicsGeometry != nullptr)
    {
        m_physicsGeometry->postModified();
    }
}

void
SphMethod::reset()
{
    m_sphSystem->resetToInitialState();
    updateGeometries();
    postModifiedAll();
}

void
SphMethod::postModifiedAll()
{
    if (m_physicsGeometry != nullptr)
    {
        m_physicsGeometry->postModified();
    }

    /*
    // Assume geometry may be changed upon reset
    for (auto comp : m_components)
    {
        if (auto visualModel = std::dynamic_pointer_cast<VisualModel>(comp))
        {
            if (visualModel->getGeometry() != nullptr)
            {
                visualModel->getGeometry()->postModified();
            }
        }
    }
    */
}

void
SphMethod::initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink)
{
    // Copy, sum, and connect the model graph to nest within this graph
    m_taskGraph->addEdge(source, m_updateNode);
    if (m_sphSystem != nullptr)
    {
        m_sphSystem->initGraphEdges();
        m_taskGraph->nestGraph(m_sphSystem->getTaskGraph(), m_updateNode, m_updateGeometryNode);
    }
    else
    {
        m_taskGraph->addEdge(m_updateNode, m_updateGeometryNode);
    }
    m_taskGraph->addEdge(m_updateGeometryNode, sink);
}
} // namespace imstk