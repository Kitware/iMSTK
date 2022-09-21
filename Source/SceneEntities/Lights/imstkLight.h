/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkColor.h"
#include "imstkEntity.h"
#include "imstkMath.h"

#include <string>

namespace imstk
{
///
/// \class Light
///
/// \brief Abstract base class for lights
///
class Light : public Entity
{
protected:
    Light() = default;

public:
    ~Light() override = default;

    ///
    /// \brief Get/Set the light focal point
    ///@{
    void setFocalPoint(const Vec3d& p) { m_focalPoint = p; }
    void setFocalPoint(const double x, const double y, const double z) { setFocalPoint(Vec3d(x, y, z)); }
    const Vec3d& getFocalPoint() const { return m_focalPoint; }
    ///@}

    ///
    /// \brief Get the status (On/off) of the light
    ///
    bool isOn() const { return m_switchState; }

    ///
    /// \brief Switch the light On
    ///
    void switchOn() { m_switchState = true; }

    ///
    /// \brief Switch the light Off
    ///
    void switchOff() { m_switchState = false; }

    ///
    /// \brief Get the light color
    ///
    const Color& getColor() const { return m_color; }

    ///
    /// \brief Set the light color
    ///
    void setColor(const Color& c) { m_color = c; }

    ///
    /// \brief Get the light intensity
    ///
    double getIntensity() const { return m_intensity; }

    ///
    /// \brief Set the light intensity. This value is unbounded.
    ///
    void setIntensity(const double intensity) { m_intensity = intensity; }

    ///
    /// \brief Sets the attenuation values. Quadratic, linear, and constant c (ax^2+bx+c)
    /// (a,b,c) = {0,0,1} would be constant lighting, multiplied with intensity
    ///
    void setAttenuationValues(const double a, const double b, const double c)
    {
        m_attenuation[0] = c;
        m_attenuation[1] = b;
        m_attenuation[2] = a;
    }

    const Vec3d& getAttenuationValues() const { return m_attenuation; }

protected:
    // properties with defaults
    double m_intensity   = 1.0;
    Color  m_color       = Color::White;
    bool   m_switchState = true;
    Vec3d  m_focalPoint  = Vec3d::Zero();
    Vec3d  m_attenuation = Vec3d(1.0, 0.0, 0.0); ///< c, b, a (ax^2+bx+c)
};
} // namespace imstk