/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkLight.h"
#include "imstkMacros.h"

namespace imstk
{
///
/// \class PointLight
///
/// \brief A point light has a position, and it's range is determined by it's
/// intensity.
///
class PointLight : public Light
{
public:
    PointLight() = default;
    ~PointLight() override = default;

    IMSTK_TYPE_NAME(PointLight)

    const Vec3d getPosition() const { return m_position; }
    void setPosition(const Vec3d& p) { m_position = p; }
    void setPosition(const double x, const double y, const double z)
    {
        this->setPosition(Vec3d(x, y, z));
    }

protected:
    Vec3d m_position = Vec3d::Zero();
};
} // namespace imstk