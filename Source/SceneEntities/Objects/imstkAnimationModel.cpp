/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkAnimationModel.h"

namespace imstk
{
AnimationModel::AnimationModel(std::shared_ptr<Geometry> geometry) : m_geometry(geometry)
{
}

std::shared_ptr<Geometry>
AnimationModel::getGeometry()
{
    return m_geometry;
}

void
AnimationModel::setGeometry(std::shared_ptr<Geometry> geometry)
{
    m_geometry = geometry;
}
} // namespace imstk