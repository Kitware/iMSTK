/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkComponent.h"
#include "imstkLogger.h"

namespace imstk
{
void
Component::initialize()
{
    // Generally the callee should only initialize entities
    CHECK(m_entity.lock() != nullptr) << "Tried to initialize a component that doesn't exist"
        " on any entity.";

    // Initialize
    init();
}

void
LambdaBehaviour::update(const double& dt)
{
    if (m_updateFunc != nullptr)
    {
        m_updateFunc(dt);
    }
    else
    {
        LOG(FATAL) << "LambdaBehaviour has no function set";
    }
}
} // namespace imstk