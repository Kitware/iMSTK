/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

=========================================================================*/

#pragma once

#include "imstkEventObject.h"
#include "imstkLogger.h"

#include <atomic>

namespace imstk
{
using EntityID = unsigned long;

class Component;

///
/// \class Entity
///
/// \brief Top-level class for entities. Entities contain a collection of
/// components which define how to be used in a system.
///
class Entity : public EventObject
{
public:
    ~Entity() override = default;

    // *INDENT-OFF*
    SIGNAL(Entity, modified);
    // *INDENT-ON*

    ///
    /// \brief Get ID (ALWAYS query the ID in your code, DO NOT hardcode it)
    /// \returns ID of entity
    ///
    EntityID getID() const;

    virtual const std::string getTypeName() const = 0;

    ///
    /// \brief Get/Set the name of the entity
    ///@{
    const std::string& getName() const { return m_name; }
    void setName(const std::string& name) { m_name = name; }
    ///@}

    ///
    /// \brief Create and return a component on this object
    ///
    template<typename T>
    std::shared_ptr<T> addComponent()
    {
        auto component = std::make_shared<T>();
        m_components.push_back(component);
        this->postEvent(Event(modified()));
        return component;
    }

    void addComponent(std::shared_ptr<Component> component);

    ///
    /// \brief Get a component by index
    ///
    std::shared_ptr<Component> getComponent(const unsigned int index) const;

    ///
    /// \brief Get the first component of type T
    ///
    template<typename T>
    std::shared_ptr<T> getComponent() const
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
    /// \brief Check if contains component of type T
    ///
    template<class T>
    bool containsComponent() const { return getComponent<T>() != nullptr; }

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

    ///
    /// \brief Remove component if it exists
    ///
    void removeComponent(std::shared_ptr<Component> component);

protected:
    std::vector<std::shared_ptr<Component>> m_components;

    Entity(const std::string& name = "Entity");

    // Not the best design pattern
    static std::atomic<EntityID> s_count; ///< current count of entities

    EntityID    m_ID;                     ///< unique ID of entity
    std::string m_name;                   ///< Not unique name
};
} // namespace imstk
