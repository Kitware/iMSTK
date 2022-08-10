/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkSceneObject.h"
#include "imstkGeometry.h"
#include "imstkLogger.h"
#include "imstkTaskGraph.h"
#include "imstkVisualModel.h"

namespace imstk
{
SceneObject::SceneObject(const std::string& name) : SceneEntity(), m_name(name),
    m_taskGraph(std::make_shared<TaskGraph>(
        "SceneObject_" + m_name + "_Source",
        "SceneObject_" + m_name + "_Sink"))
{
    m_updateNode = m_taskGraph->addFunction("SceneObject_" + m_name + "_Update", [this]() { update(); });
    m_updateGeometryNode = m_taskGraph->addFunction("SceneObject_" + m_name + "_UpdateGeometry", [this]() { updateGeometries(); });
}

std::shared_ptr<Geometry>
SceneObject::getVisualGeometry() const
{
    if (!m_visualModels.empty())
    {
        return m_visualModels[0]->getGeometry();
    }
    return nullptr;
}

void
SceneObject::setVisualGeometry(std::shared_ptr<Geometry> geometry)
{
    if (m_visualModels.empty())
    {
        auto visualModel = std::make_shared<VisualModel>();
        visualModel->setGeometry(geometry);
        m_visualModels.push_back(visualModel);
    }
    else
    {
        m_visualModels[0]->setGeometry(geometry);
    }
}

void
SceneObject::initGraphEdges()
{
    m_taskGraph->clearEdges();
    initGraphEdges(m_taskGraph->getSource(), m_taskGraph->getSink());
}

void
SceneObject::initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink)
{
    m_taskGraph->addEdge(source, m_updateNode);
    m_taskGraph->addEdge(m_updateNode, m_updateGeometryNode);
    m_taskGraph->addEdge(m_updateGeometryNode, sink);
}

void
SceneObject::postModifiedAll()
{
    // Assume geometry may be changed upon reset
    for (auto visualModel : m_visualModels)
    {
        visualModel->getGeometry()->postModified();
    }
}
} // namespace imstk