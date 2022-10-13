/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkMacros.h"
#include "imstkTaskGraph.h"

namespace imstk
{
class Entity;

///
/// \class Component
///
/// \brief Represents a part of an entity, involved in a system.
/// The component system is doubly linked meaning the Entity contains
/// a shared_ptr to Component while the Component keeps a weak_ptr to Entity.
/// Components are able to not exist on an entity as the entity parent is not
/// garunteed to exist.
/// The initialize call cannot be issue'd without a valid entity.
///
class Component
{
friend class Entity;

protected:
    Component(const std::string& name = "Component") : m_name(name) { }

public:
    virtual ~Component() = default;

    const std::string& getName() const { return m_name; }
    void setName(const std::string& name) { m_name = name; }

    ///
    /// \brief Get parent entity
    ///
    std::weak_ptr<Entity> getEntity() const { return m_entity; }

    ///
    /// \brief Initialize the component, called at a later time after all
    /// component construction is complete.
    ///
    void initialize();

protected:
    ///
    /// \brief Initialize the component, called at a later time after all
    /// component construction is complete.
    ///
    virtual void init() { }

    std::string m_name;
    /// Parent entity this component exists on
    std::weak_ptr<Entity> m_entity;
};

///
/// \class Behaviour
///
/// \brief A Behaviour represents a single component system
/// A template is used here for UpdateInfo to keep the ComponentModel
/// library more general and separable. UpdateInfo could be anything
/// you need from outside to update the component, this would generally
/// be your own struct or just a single primitive such as double deltatime
///
template<typename UpdateInfo>
class Behaviour : public Component
{
protected:
    Behaviour(const std::string& name = "Behaviour") : Component(name) { }

public:
    ~Behaviour() override = default;

    virtual void update(const UpdateInfo& imstkNotUsed(updateData)) { }
    virtual void visualUpdate(const UpdateInfo& imstkNotUsed(updateData)) { }
};

///
/// \class TaskBehaviour
///
/// \brief Defines a behaviour that also can also schedule TaskGraph functions.
/// It's pipeline can also be "joined"/combined with other TaskGraphs. Nodes
/// shared between two graphs are combined.
///
template<typename UpdateInfo>
class TaskBehaviour : public Behaviour<UpdateInfo>
{
protected:
    TaskBehaviour(const std::string& name = "TaskBehaviour") : Behaviour<UpdateInfo>(name),
        m_taskGraph(std::make_shared<TaskGraph>())
    {
    }

public:
    ~TaskBehaviour() override = default;

    ///
    /// \brief Setup the edges/connections of the TaskGraph
    ///
    void initGraph()
    {
        m_taskGraph->clearEdges();
        initGraphEdges(m_taskGraph->getSource(), m_taskGraph->getSink());
    }

    std::shared_ptr<TaskGraph> getTaskGraph() const { return m_taskGraph; }

protected:
    ///
    /// \brief Setup the edges/connections of the TaskGraph
    ///
    virtual void initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink) = 0;

    std::shared_ptr<TaskGraph> m_taskGraph = nullptr;
};

///
/// \class SceneBehaviour
///
/// \brief A SceneBehaviour represents a single component system
/// that resides in the scene. It makes the assumption that all
/// components used are updated with a double for deltaTime/time passed.
///
using SceneBehaviour     = Behaviour<double>;
using SceneTaskBehaviour = TaskBehaviour<double>;

///
/// \brief A SceneBehaviour that can update via a lambda function
///
class LambdaBehaviour : public SceneBehaviour
{
public:
    LambdaBehaviour(const std::string& name = "LambdaBehaviour") : Behaviour(name) { }
    ~LambdaBehaviour() override = default;

    void update(const double& dt) override;
    void visualUpdate(const double& dt) override;

    void setUpdate(std::function<void(const double& dt)> updateFunc) { m_updateFunc = updateFunc; }
    void setVisualUpdate(std::function<void(const double& dt)> updateFunc) { m_visualUpdateFunc = updateFunc; }

protected:
    std::function<void(const double& dt)> m_updateFunc;
    std::function<void(const double& dt)> m_visualUpdateFunc;
};
} // namespace imstk