/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkLogger.h"
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
/// be your own struct or just a single primitive such as double timestep
///
/// Behaviour provides two ways of defining system logic.
/// 1. Updating via the normal update & visualUpdate function
/// 2. Updating via a TaskGraph definition.
/// Both may be used together but normal update's will always occur after
/// all TaskGraph updates. Additionally visualUpdate's occur before renders.
///
/// All TaskGraph's are joined into the Scene TaskGraph. If any node is shared
/// (occurs in both) between TaskGraphs, they will join/become one. For example,
/// graphA could define A->B->C. Whilst graphB defines D->B->E. B is shared.
/// This allows one to order calls easily and extensibly.
///
/// When dealing with large amounts of similar components one may also consider
/// making a separate system of components. For example, how the Scene is a system of
/// Components. The Renderer is a system of VisualModel components. This is more
/// cache friendly.
///
template<typename UpdateInfo>
class Behaviour : public Component
{
protected:
    Behaviour(const std::string& name = "Behaviour") : Component(name) { }
    Behaviour(const bool useTaskGraph, const std::string& name = "Behaviour") : Component(name)
    {
        if (useTaskGraph)
        {
            m_taskGraph = std::make_shared<TaskGraph>();

            // Set default names
            m_taskGraph->getSource()->m_name = "Behavior_Source";
            m_taskGraph->getSink()->m_name   = "Behavior_Sink";
        }
    }

public:
    ~Behaviour() override = default;

    virtual void update(const UpdateInfo& imstkNotUsed(updateData)) { }
    virtual void visualUpdate(const UpdateInfo& imstkNotUsed(updateData)) { }

    ///
    /// \brief Setup the edges/connections of the TaskGraph
    ///
    void initTaskGraphEdges()
    {
        CHECK(m_taskGraph != nullptr) << "Tried to setup task graph edges but no TaskGraph exists";
        m_taskGraph->clearEdges();
        initGraphEdges(m_taskGraph->getSource(), m_taskGraph->getSink());
    }

    std::shared_ptr<TaskGraph> getTaskGraph() const { return m_taskGraph; }

protected:
    ///
    /// \brief Setup the edges/connections of the TaskGraph\
    //     /// \param source, first node of the graph (does no function)
    /// \param sink, last node of the graph (does no function)
    ///
    virtual void initGraphEdges(std::shared_ptr<TaskNode> imstkNotUsed(source),
                                std::shared_ptr<TaskNode> imstkNotUsed(sink)) { }

    std::shared_ptr<TaskGraph> m_taskGraph = nullptr;
};

///
/// \class SceneBehaviour
/// \class SceneBehaviour
///
/// \brief A SceneBehaviour represents a single component system
/// that resides in the scene. It makes the assumption that all
/// components used are updated with a double for deltaTime/time passed.
///
using SceneBehaviour = Behaviour<double>;

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