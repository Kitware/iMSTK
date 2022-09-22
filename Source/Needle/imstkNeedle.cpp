/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkNeedle.h"

namespace imstk
{
void
Needle::setPuncture(const PunctureId& id, std::shared_ptr<Puncture> data)
{
    m_punctures[id] = data;
}

std::shared_ptr<Puncture>
Needle::getPuncture(const PunctureId& id)
{
    auto iter = m_punctures.find(id);
    if (iter == m_punctures.end())
    {
        m_punctures[id] = std::make_shared<Puncture>();
    }
    return m_punctures[id];
}

void
Needle::setState(const PunctureId& id, const Puncture::State state)
{
    auto iter = m_punctures.find(id);
    if (iter == m_punctures.end())
    {
        m_punctures[id] = std::make_shared<Puncture>();
    }
    m_punctures[id]->state = state;
}

Puncture::State
Needle::getState(const PunctureId& id)
{
    return getPuncture(id)->state;
}

bool
Needle::getInserted() const
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
} // namespace imstk