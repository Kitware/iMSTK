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
/// \class DirectionalLight
///
/// \brief A directional light has no position or range. The focal point is the
/// direction.
///
class DirectionalLight : public Light
{
public:
    DirectionalLight()
    {
        this->setFocalPoint(-1.0, -1.0, -1.0);
    }

    ~DirectionalLight() override = default;

    IMSTK_TYPE_NAME(DirectionalLight)

    ///
    /// \brief Set the direction of the light
    ///@{
    void setDirection(const Vec3d& dir) { setFocalPoint(dir); }
    void setDirection(const double x, const double y, const double z) { setFocalPoint(Vec3d(x, y, z)); }
    ///@}
};
} // namespace imstk