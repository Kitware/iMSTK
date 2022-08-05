/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkAnalyticalGeometry.h"
#include "imstkMacros.h"

namespace imstk
{
///
/// \class Capsule
///
/// \brief Capsule geometry, default configuration is centered
///        at origin with length running up and down the y axes.
///        The length refers to the length of the cylinder.
///
class Capsule : public AnalyticalGeometry
{
public:
    Capsule(const Vec3d& pos    = Vec3d(0.0, 0.0, 0.0), const double radius = 0.5,
            const double length = 1.0, const Quatd orientation = Quatd::Identity()) :
        AnalyticalGeometry()
    {
        setPosition(pos);
        setOrientation(orientation);
        setRadius(radius);
        setLength(length);
        updatePostTransformData();
    }

    ~Capsule() override = default;

    IMSTK_TYPE_NAME(Capsule)

    ///
    /// \brief Print the capsule info
    ///
    void print() const override;

    ///
    /// \brief Returns the volume of the capsule
    ///
    double getVolume() override { return PI * m_radius * m_radius * (m_length + 4.0 / 3.0 * m_radius); }

    ///
    /// \brief Get/Set the radius of the capsule
    ///@{
    double getRadius(DataType type = DataType::PostTransform);
    void setRadius(const double r);
    ///@}

    ///
    /// \brief Get/Set the length of the capsule
    ///@{
    double getLength(DataType type = DataType::PostTransform);
    void setLength(const double l);
    ///@}

    ///
    /// \brief Returns the signed distance to the capsule
    ///
    double getFunctionValue(const Vec3d& x) const override;

    ///
    /// \brief Get the min, max of the AABB around the capsule
    ///
    void computeBoundingBox(Vec3d& min, Vec3d& max, const double paddingPercent);

    ///
    /// \brief Update the Capsule parameters applying the latest transform
    ///
    void updatePostTransformData() const override;

protected:
    void applyTransform(const Mat4d& m) override;

    double m_radius = 1.0;                      ///< Radius of the hemispheres at the end of the capsule
    mutable double m_radiusPostTransform = 1.0; ///< Radius after transform
    double m_length = 1.0;                      ///< Length between the centers of two hemispheres
    mutable double m_lengthPostTransform = 1.0; ///< Length after transform
};
} // namespace imstk