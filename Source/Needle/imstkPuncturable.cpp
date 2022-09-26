/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkPuncturable.h"
#include "imstkEntity.h"
#include "imstkNeedle.h"

namespace imstk
{
void
Puncturable::setPuncture(const PunctureId& id, std::shared_ptr<Puncture> data)
{
    m_punctures[id] = data;
}

std::shared_ptr<Puncture>
Puncturable::getPuncture(const PunctureId& id)
{
    auto iter = m_punctures.find(id);
    if (iter == m_punctures.end())
    {
        m_punctures[id] = std::make_shared<Puncture>();
    }
    return m_punctures[id];
}

bool
Puncturable::getPunctured() const
{
    for (auto puncture : m_punctures)
    {
        if (puncture.second->state == Puncture::State::INSERTED)
        {
            return true;
        }
    }
    return false;
}

PunctureId
getPunctureId(std::shared_ptr<Needle>      needle,
              std::shared_ptr<Puncturable> puncturable,
              const int                    supportId)
{
    std::shared_ptr<Entity> needleEntity      = needle->getEntity().lock();
    std::shared_ptr<Entity> puncturableEntity = puncturable->getEntity().lock();
    CHECK(needleEntity != nullptr) << "Cannot generate puncture id without needle entity";
    CHECK(puncturableEntity != nullptr) << "Cannot generate puncture id without puncturable entity";
    return { needleEntity->getID(), puncturableEntity->getID(), supportId };
}
} // namespace imstk