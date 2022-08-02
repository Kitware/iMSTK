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
/// \class OrientedBox
///
/// \brief OrientedBox geometry, specified with extents (half lengths)
///
class OrientedBox : public AnalyticalGeometry
{
public:
    OrientedBox(const Vec3d& pos         = Vec3d::Zero(),
                const Vec3d extents      = Vec3d(0.5, 0.5, 0.5),
                const Quatd& orientation = Quatd::Identity()) : AnalyticalGeometry()
    {
        setPosition(pos);
        setOrientation(orientation);
        setExtents(extents);
        updatePostTransformData();
    }

    ~OrientedBox() override = default;

    IMSTK_TYPE_NAME(OrientedBox)

    ///
    /// \brief Print the cube info
    ///
    void print() const override;

    ///
    /// \brief Returns the volume of the cube
    ///
    double getVolume() override { return m_extents[0] * m_extents[1] * m_extents[2] * 8.0; }

    ///
    /// \brief Returns the extents of the cube
    ///
    Vec3d getExtents(DataType type = DataType::PostTransform);

    ///
    /// \brief Sets the extents of the cube (half dimensions)
    ///
    void setExtents(const Vec3d extents);

    ///
    /// \brief Sets the width of the cube (half dimensions)
    ///
    void setExtents(const double x, const double y, const double z) { setExtents(Vec3d(x, y, z)); }

    ///
    /// \brief Returns signed distance to surface at pos
    ///
    double getFunctionValue(const Vec3d& pos) const override;

    ///
    /// \brief Get the min, max of the AABB around the cube
    ///
    void computeBoundingBox(Vec3d& min, Vec3d& max, const double paddingPercent = 0.0) override;

    ///
    /// \brief Update the Cube parameters applying the latest transform
    ///
    void updatePostTransformData() const override;

protected:
    void applyTransform(const Mat4d& m) override;

    Vec3d m_extents = Vec3d(0.5, 0.5, 0.5);                      // Half dimensions of the oriented box
    mutable Vec3d m_extentsPostTransform = Vec3d(0.5, 0.5, 0.5); ///< Half dimensions of the oriented box once transform applied
};
} // namespace imstk
