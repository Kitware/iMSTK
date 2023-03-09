/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkDynamicObject.h"
#include "imstkAbstractDynamicalSystem.h"
#include "imstkGeometryMap.h"
#include "imstkLogger.h"
#include "imstkTaskGraph.h"
#include "imstkVisualModel.h"

namespace imstk
{
size_t
DynamicObject::getNumOfDOF() const
{
    if (!m_dynamicalModel)
    {
        LOG(WARNING) << "Cannot get the degree of freedom since the dynamical model is not initialized! returning 0";
        return 0;
    }

    return m_dynamicalModel->getNumDegreeOfFreedom();
}

void
DynamicObject::updateGeometries()
{
    updatePhysicsGeometry();

    SceneObject::updateGeometries();
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
DynamicObject::updatePhysicsGeometry()
{
    m_dynamicalModel->updatePhysicsGeometry();
    if (m_physicsGeometry != nullptr)
    {
        m_physicsGeometry->postModified();
    }
}

bool
DynamicObject::initialize()
{
    if (SceneObject::initialize())
    {
        CHECK(m_physicsGeometry != nullptr) << "DynamicObject \"" << m_name
                                            << "\" expects a physics geometry at start, none was provided";

        if (m_physicsToCollidingGeomMap)
        {
            m_physicsToCollidingGeomMap->compute();
        }

        if (m_physicsToVisualGeomMap)
        {
            m_physicsToVisualGeomMap->compute();
        }

        return true;
    }
    else
    {
        return false;
    }
}

void
DynamicObject::initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink)
{
    // Copy, sum, and connect the model graph to nest within this graph
    m_taskGraph->addEdge(source, getUpdateNode());
    if (m_dynamicalModel != nullptr)
    {
        m_dynamicalModel->initGraphEdges();
        m_taskGraph->nestGraph(m_dynamicalModel->getTaskGraph(), getUpdateNode(), getUpdateGeometryNode());
    }
    else
    {
        m_taskGraph->addEdge(getUpdateNode(), getUpdateGeometryNode());
    }
    m_taskGraph->addEdge(getUpdateGeometryNode(), sink);
}

void
DynamicObject::reset()
{
    m_dynamicalModel->resetToInitialState();
    updateGeometries();
    postModifiedAll();
}

void
DynamicObject::postModifiedAll()
{
    if (m_physicsGeometry != nullptr)
    {
        m_physicsGeometry->postModified();
    }
    SceneObject::postModifiedAll();
}
} // namespace imstk