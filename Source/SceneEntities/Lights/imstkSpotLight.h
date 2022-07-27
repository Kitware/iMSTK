/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkMacros.h"
#include "imstkPointLight.h"

namespace imstk
{
///
/// \class Spot light class
///
/// \brief A spot light is a point light in a cone shape.
///
class SpotLight : public PointLight
{
public:
    SpotLight() : m_spotAngle(10.0) { }
    ~SpotLight() override = default;

    IMSTK_TYPE_NAME(SpotLight)

    ///
    /// \brief Get the spotlight angle in degrees
    ///
    double getSpotAngle() const { return m_spotAngle; }

    ///
    /// \brief Set the spotlight angle in degrees
    ///
    void setSpotAngle(const double angle) { m_spotAngle = angle; }

protected:
    double m_spotAngle = 45.0;
};
} // namespace imstk