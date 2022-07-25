/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkPbdObject.h"
#include "imstkLogger.h"
#include "imstkPbdModel.h"

namespace imstk
{
std::shared_ptr<PbdModel>
PbdObject::getPbdModel()
{
    m_pbdModel = std::dynamic_pointer_cast<PbdModel>(m_dynamicalModel);
    return m_pbdModel;
}

bool
PbdObject::initialize()
{
    m_pbdModel = std::dynamic_pointer_cast<PbdModel>(m_dynamicalModel);
    if (m_pbdModel == nullptr)
    {
        LOG(FATAL) << "PbdObject " << m_name << " was not given a PbdModel. Please PbdObject::setDynamicalModel";
        return false;
    }

    DynamicObject::initialize();

    return true;
}
} // namespace imstk