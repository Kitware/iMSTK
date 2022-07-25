/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkPickingAlgorithm.h"

namespace imstk
{
class CollisionDetectionAlgorithm;

///
/// \class PointPicker
///
/// \brief Picks points on elements of geomToPick via those that that are
/// intersecting the provided ray.
/// \todo: Make extensible
///
class PointPicker : public PickingAlgorithm
{
public:
    PointPicker() = default;
    ~PointPicker() override = default;

    void requestUpdate() override;

    ///
    /// \brief Set the picking ray
    /// \param Start of the ray
    /// \param Direction of the ray
    /// \param Max distance of accepted pick points, anything farther excluded
    ///
    void setPickingRay(const Vec3d& rayStart, const Vec3d& rayDir,
                       const double maxDist = -1.0)
    {
        m_rayStart = rayStart;
        m_rayDir   = rayDir.normalized();
        m_maxDist  = maxDist;
    }

    const Vec3d& getPickRayStart() const { return m_rayStart; }
    const Vec3d& getPickRayDir() const { return m_rayDir; }

    ///
    /// \brief Get/Set whether only the first hit is used otherwise all
    /// intersections are returned
    ///@{
    void setUseFirstHit(const bool useFirstHit) { m_useFirstHit = useFirstHit; }
    bool getUseFirstHit() const { return m_useFirstHit; }
///@}

protected:
    Vec3d  m_rayStart    = Vec3d::Zero();
    Vec3d  m_rayDir      = Vec3d::Zero();
    double m_maxDist     = -1.0;
    bool   m_useFirstHit = true;
};
} // namespace imstk