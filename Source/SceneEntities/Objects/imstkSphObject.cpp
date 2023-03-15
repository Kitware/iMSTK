/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkSphObject.h"
#include "imstkSphSystem.h"

namespace imstk
{
bool
SphObject::initialize()
{
    m_sphModel = std::dynamic_pointer_cast<SphSystem>(m_dynamicalModel);
    if (m_sphModel == nullptr)
    {
        LOG(FATAL) << "Dynamics pointer cast failure in SPHObject::initialize()";
        return false;
    }

    // why are we initializing twice?
    DynamicObject::initialize();
    m_sphModel->initialize();

    return true;
}

std::shared_ptr<SphSystem>
SphObject::getSphModel()
{
    m_sphModel = std::dynamic_pointer_cast<SphSystem>(m_dynamicalModel);
    return m_sphModel;
}
} // namespace imstk