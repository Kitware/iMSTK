/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

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

#include "imstkEntity.h"

namespace imstk
{
std::atomic<EntityID> Entity::s_count { 0 };

Entity::Entity(const std::string& name) : m_name(name)
{
    s_count++;
    m_ID = s_count;
}

EntityID
Entity::getID() const
{
    return m_ID;
}

void
Entity::addComponent(std::shared_ptr<Component> component)
{
    auto iter = std::find(m_components.begin(), m_components.end(), component);
    if (iter != m_components.end())
    {
        LOG(FATAL) << "Tried to add component to object twice";
        return;
    }
    m_components.push_back(component);
    this->postEvent(Event(modified()));
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
        m_components.erase(iter);
    }
    else
    {
        LOG(FATAL) << "Failed to remove component on entity, could not find " << m_ID;
    }
    this->postEvent(Event(modified()));
}
} // namespace imstk
