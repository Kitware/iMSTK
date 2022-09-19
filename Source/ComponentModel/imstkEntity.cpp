/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkEntity.h"
#include "imstkComponent.h"

namespace imstk
{
std::atomic<EntityID> Entity::s_count { 0 };

Entity::Entity(const std::string& name) : m_name(name)
{
    s_count++;
    m_ID = s_count;
}

void
Entity::addComponent(std::shared_ptr<Component> component)
{
    CHECK(component != nullptr) << "Tried to add nullptr component";
    auto iter = std::find(m_components.begin(), m_components.end(), component);
    if (iter != m_components.end())
    {
        LOG(FATAL) << "Tried to add component to object twice";
        return;
    }
    m_components.push_back(component);
    component->m_entity = this;
    this->postEvent(Event(modified()));
}

bool
Entity::containsComponent(std::shared_ptr<Component> component) const
{
    auto iter = std::find(m_components.begin(), m_components.end(), component);
    return iter != m_components.end();
}

std::shared_ptr<Component>
Entity::getComponent(const unsigned int index) const
{
    CHECK(index >= 0 && index < m_components.size()) <<
        "component with index does not exist, index out of range";
    return m_components[index];
}

void
Entity::removeComponent(std::shared_ptr<Component> component)
{
    auto iter = std::find(m_components.begin(), m_components.end(), component);
    if (iter != m_components.end())
    {
        (*iter)->m_entity = nullptr;
        m_components.erase(iter);
    }
    else
    {
        LOG(FATAL) << "Failed to remove component on entity, could not find " << m_ID;
    }
    this->postEvent(Event(modified()));
}
} // namespace imstk
