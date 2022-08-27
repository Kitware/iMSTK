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
/// \class Cylinder
///
/// \brief Cylinder geometry, default configuration is at origin with length running up the y axes
///
class Cylinder : public AnalyticalGeometry
{
public:
    Cylinder(const Vec3d& pos = Vec3d(0.0, 0.0, 0.0), const double radius = 1.0, const double length = 1.0,
             const Quatd& orientation = Quatd::Identity()) :
        AnalyticalGeometry()
    {
        setPosition(pos);
        setOrientation(orientation);
        setRadius(radius);
        setLength(length);
        updatePostTransformData();
    }

    ~Cylinder() override = default;

    IMSTK_TYPE_NAME(Cylinder)

    ///
    /// \brief Print the cylinder info
    ///
    void print() const override;

    ///
    /// \brief Returns the volume of the cylinder
    ///
    double getVolume() override { return PI * m_radius * m_radius * m_length; }

    ///
    /// \brief Get/Set the radius of the cylinder
    ///@{
    double getRadius(DataType type = DataType::PostTransform);
    void setRadius(const double r);
    ///@}

    ///
    /// \brief Get/Set the length of the cylinder
    ///@{
    double getLength(DataType type = DataType::PostTransform);
    void setLength(const double r);
    ///@}

    ///
    /// \brief Get the min, max of the AABB around the cylinder
    ///
    void computeBoundingBox(Vec3d& min, Vec3d& max, const double paddingPercent);

    ///
    /// \brief Update the Cylinder parameters applying the latest transform
    ///
    void updatePostTransformData() const override;

    ///
    /// \brief Polymorphic clone, hides the declaration in superclass
    /// return own type
    ///
    std::unique_ptr<Cylinder> clone()
    {
        return std::unique_ptr<Cylinder>(cloneImplementation());
    }

protected:
    // Hide these unimplemented functions
    using AnalyticalGeometry::getFunctionValue;

    void applyTransform(const Mat4d& m) override;

    double m_radius = 1.0;                      ///< Radius of the cylinder
    double m_length = 1.0;                      ///< Length of the cylinder
    mutable double m_radiusPostTransform = 1.0; ///< Radius of the cylinder oncee transform applied
    mutable double m_lengthPostTransform = 1.0; ///< Length of the cylinder onc transform applied

private:
    Cylinder* cloneImplementation() const
    {
        return new Cylinder(*this);
    }
};
} // namespace imstk