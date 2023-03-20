/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkLevelSetDeformableObject.h"
#include "imstkLogger.h"
#include "imstkPbdSystem.h"
#include "imstkLevelSetSystem.h"

namespace imstk
{
std::shared_ptr<LevelSetSystem>
LevelSetDeformableObject::getLevelSetModel()
{
    m_levelSetModel = std::dynamic_pointer_cast<LevelSetSystem>(m_dynamicalModel);
    return m_levelSetModel;
}

bool
LevelSetDeformableObject::initialize()
{
    m_levelSetModel = std::dynamic_pointer_cast<LevelSetSystem>(m_dynamicalModel);
    if (m_levelSetModel == nullptr)
    {
        LOG(FATAL) << "Dynamics pointer cast failure in LevelSetDeformableObject::initialize()";
        return false;
    }

    DynamicObject::initialize();
    m_levelSetModel->initialize();

    return true;
}
} // namespace imstk