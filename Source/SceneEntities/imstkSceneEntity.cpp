/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkSceneEntity.h"

namespace imstk
{
std::atomic<EntityID> SceneEntity::s_count { 0 };

SceneEntity::SceneEntity()
{
    s_count++;
    m_ID = s_count;
}

EntityID
SceneEntity::getID() const
{
    return m_ID;
}
} // namespace imstk
