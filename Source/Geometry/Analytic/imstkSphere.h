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
/// \class Sphere
///
/// \brief Represents a sphere via its position & radius
///
class Sphere : public AnalyticalGeometry
{
public:
    Sphere(const Vec3d& pos = Vec3d(0.0, 0.0, 0.0), const double radius = 1.0) :
        AnalyticalGeometry()
    {
        setPosition(pos);
        setRadius(radius);
        updatePostTransformData();
    }

    ~Sphere() override = default;

    IMSTK_TYPE_NAME(Sphere)

    ///
    /// \brief Print the sphere info
    ///
    void print() const override;

    ///
    /// \brief Returns the volume of the sphere
    ///
    double getVolume() override { return 4.0 / 3.0 * PI * m_radius * m_radius * m_radius; }

    ///
    /// \brief Get/Set the radius of the sphere
    ///@{
    double getRadius(DataType type = DataType::PostTransform);
    void setRadius(const double r);
    ///@}

    ///
    /// \brief Get the min, max of the AABB around the sphere
    ///
    void computeBoundingBox(Vec3d& lowerCorner, Vec3d& upperCorner, const double paddingPercent = 0.0) override;

    ///
    /// \brief Returns signed distance to surface given position
    ///
    double getFunctionValue(const Vec3d& pos) const override { return (pos - m_positionPostTransform).norm() - m_radiusPostTransform; }

    ///
    /// \brief Update the Sphere parameters applying the latest transform
    ///
    void updatePostTransformData() const override;

    ///
    /// \brief Polymorphic clone, hides the declaration in superclass
    /// return own type
    ///
    std::unique_ptr<Sphere> clone()
    {
        return std::unique_ptr<Sphere>(cloneImplementation());
    }

protected:
    void applyTransform(const Mat4d& m) override;

    double m_radius = 1.0;                      ///< Radius of the sphere
    mutable double m_radiusPostTransform = 1.0; ///< Radius of the sphere once transform applied

private:
    Sphere* cloneImplementation() const
    {
        return new Sphere(*this);
    }
};
} // namespace imstk