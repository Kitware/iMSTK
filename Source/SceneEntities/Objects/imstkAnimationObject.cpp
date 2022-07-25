/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkAnimationObject.h"

namespace imstk
{
std::shared_ptr<AnimationModel>
AnimationObject::getAnimationModel() const
{
    return m_animationModel;
}

void
AnimationObject::setAnimationModel(std::shared_ptr<AnimationModel> model)
{
    m_animationModel = model;
}
} // namespace imstk