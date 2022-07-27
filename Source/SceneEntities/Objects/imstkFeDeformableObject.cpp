/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkFeDeformableObject.h"
#include "imstkFemDeformableBodyModel.h"
#include "imstkLogger.h"

namespace imstk
{
bool
FeDeformableObject::initialize()
{
    m_femModel = std::dynamic_pointer_cast<FemDeformableBodyModel>(m_dynamicalModel);
    if (m_femModel == nullptr)
    {
        LOG(FATAL) << "Dynamics pointer cast failure in DeformableObject::initialize()";
        return false;
    }

    DynamicObject::initialize();
    m_femModel->initialize();

    return true;
}

std::shared_ptr<FemDeformableBodyModel>
FeDeformableObject::getFEMModel()
{
    m_femModel = std::dynamic_pointer_cast<FemDeformableBodyModel>(m_dynamicalModel);
    return m_femModel;
}
} // namespace imstk