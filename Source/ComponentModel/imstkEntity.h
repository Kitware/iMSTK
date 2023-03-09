/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkEventObject.h"
#include "imstkLogger.h"

#include <atomic>

namespace imstk
{
using EntityID = std::uint32_t;

class Component;

///
/// \class Entity
///
/// \brief Top-level class for entities. Entities contain a collection of
/// components which define how to be used in a system.
///
class Entity : public EventObject, public std::enable_shared_from_this<Entity>
{
public:
    Entity(const std::string& name = "Entity");
    ~Entity() override = default;

    // *INDENT-OFF*
    SIGNAL(Entity, modified);
    // *INDENT-ON*

    ///
    /// \brief Get ID (ALWAYS query the ID in your code, DO NOT hardcode it)
    /// \returns ID of entity
    ///
    EntityID getID() const { return m_ID; }

    virtual const std::string getTypeName() const { return "Entity"; }

    ///
    /// \brief Get/Set the name of the entity
    ///@{
    const std::string& getName() const { return m_name; }
    void setName(const std::string& name) { m_name = name; }
    ///@}

    ///
    /// \brief Create and return a component on this entity
    ///
    template<typename T>
    std::shared_ptr<T> addComponent()
    {
        auto component = std::make_shared<T>();
        addComponent(component);
        return component;
    }

    template<typename T>
    std::shared_ptr<T> addComponent(std::string name)
    {
        auto component = std::make_shared<T>(name);
        addComponent(component);
        return component;
    }

    ///
    /// \brief Add existing component to entity
    ///
    void addComponent(std::shared_ptr<Component> component);

    ///
    /// \brief Get a component by index
    ///
    std::shared_ptr<Component> getComponent(const unsigned int index) const;

    ///
    /// \brief Get the first component of type T.
    /// Returns nullptr if component is not found.
    ///
    template<typename T>
    std::shared_ptr<T> getComponentUnsafe() const
    {
        for (const auto& component : m_components)
        {
            if (auto compT = std::dynamic_pointer_cast<T>(component))
            {
                return compT;
            }
        }
        return nullptr;
    }

    ///
    /// \brief Get the first component of type T. Logs a fatal error if no component is found.
    ///
    template<typename T>
    std::shared_ptr<T> getComponent() const
    {
        if (auto compT = getComponentUnsafe<T>())
        {
            return compT;
        }
        else
        {
            LOG(FATAL) << "Component not found in Entity.";
            return nullptr;
        }
    }

    ///
    /// \brief Get's the Nth component of type T
    ///
    template<typename T>
    std::shared_ptr<T> getComponentN(const int index) const
    {
        int count = 0;
        for (auto comp : m_components)
        {
            if (auto castComp = std::dynamic_pointer_cast<T>(comp))
            {
                if (count == index)
                {
                    return castComp;
                }
                count++;
            }
        }
        return nullptr;
    }

    ///
    /// \brief Check if contains component of type T
    ///
    template<class T>
    bool containsComponent() const { return getComponentUnsafe<T>() != nullptr; }
    ///
    /// \brief Check if contains given component
    ///
    bool containsComponent(std::shared_ptr<Component> component) const;

    ///
    /// \brief Get all components of type T
    ///
    template<typename T>
    std::vector<std::shared_ptr<T>> getComponents() const
    {
        std::vector<std::shared_ptr<T>> components;
        for (const auto& component : m_components)
        {
            if (auto compT = std::dynamic_pointer_cast<T>(component))
            {
                components.push_back(compT);
            }
        }
        return components;
    }

    const std::vector<std::shared_ptr<Component>>& getComponents() { return m_components; }

    ///
    /// \brief Remove component if it exists
    ///
    void removeComponent(std::shared_ptr<Component> component);

protected:
    std::vector<std::shared_ptr<Component>> m_components;

    static std::atomic<EntityID> m_count; ///< current count of entities

    EntityID    m_ID;                     ///< unique ID of entity
    std::string m_name;                   ///< Not unique name
};
} // namespace imstk
