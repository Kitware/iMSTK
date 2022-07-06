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
SceneObject::SceneObject(const std::string& name) : Entity(name),
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
    auto visualModel = getComponent<VisualModel>();
    if (visualModel != nullptr)
    {
        return visualModel->getGeometry();
    }
    return nullptr;
}

void
SceneObject::setVisualGeometry(std::shared_ptr<Geometry> geometry)
{
    auto iter = std::find_if(m_components.begin(), m_components.end(),
        [](std::shared_ptr<Component> comp)
        {
            return std::dynamic_pointer_cast<VisualModel>(comp) != nullptr;
        });
    if (iter != m_components.end())
    {
        std::dynamic_pointer_cast<VisualModel>(*iter)->setGeometry(geometry);
    }
    else
    {
        auto visualModel = addComponent<VisualModel>();
        visualModel->setGeometry(geometry);
    }
}

std::shared_ptr<VisualModel>
SceneObject::getVisualModel(const int index) const
{
    int count = 0;
    for (auto comp : m_components)
    {
        if (auto visualModel = std::dynamic_pointer_cast<VisualModel>(comp))
        {
            if (count == index)
            {
                return visualModel;
            }
            count++;
        }
    }
    return nullptr;
}

void
SceneObject::addVisualModel(std::shared_ptr<VisualModel> visualModel)
{
    addComponent(visualModel);
}

void
SceneObject::removeVisualModel(std::shared_ptr<VisualModel> visualModel)
{
    removeComponent(visualModel);
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