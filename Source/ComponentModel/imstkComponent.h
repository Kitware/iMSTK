/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkMacros.h"

#include <memory>
#include <string>

namespace imstk
{
class Entity;

///
/// \class Component
///
/// \brief Represents a part of a component, involved in a system.
/// The component system is doubly linked meaning the Entity contains
/// a shared_ptr to Component while the Component keeps a raw pointer to Entity.
/// We do not garuntee the raw pointer is not null.
/// Components are not required/enforced to exist on an Entity.
/// This effects whether you can add a component through another components
/// constructor. Components are able to not exist on an entity.
/// Instead we provide a separate initialize step that should be called when
/// the components have entities ready
///
class Component
{
friend class Entity;

protected:
    Component(const std::string& name = "Component") : m_name(name) { }

public:
    virtual ~Component() = default;

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

protected:
    std::string m_name;
    /// Raw pointer to parent entity
    /// Used as shared_from_this cannot be used in a constructor
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
} // namespace imstk