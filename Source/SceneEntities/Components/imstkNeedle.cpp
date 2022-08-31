/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkNeedle.h"
#include "imstkEntity.h"
#include "imstkLineMesh.h"

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

Vec3d
StraightNeedle::getNeedleDirection() const
{
    return (-m_needleGeom->getRotation().col(1)).normalized();
}

const Vec3d&
StraightNeedle::getNeedleStart() const
{
    return (*m_needleGeom->getVertexPositions())[0];
}

const Vec3d&
StraightNeedle::getNeedleEnd() const
{
    return (*m_needleGeom->getVertexPositions())[1];
}

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
    return { needle->getEntity()->getID(), puncturable->getEntity()->getID(), supportId };
}
} // namespace imstk