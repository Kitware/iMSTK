/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkNeedle.h"

namespace imstk
{
class LineMesh;

///
/// \class StraightNeedle
///
/// \brief Definition of straight, single segment needle
///
class StraightNeedle : public Needle
{
public:
    IMSTK_TYPE_NAME(StraightNeedle)

    StraightNeedle(const std::string& name = "StraightNeedle") : Needle(name) { }
    ~StraightNeedle() override = default;

    void setNeedleGeometry(std::shared_ptr<LineMesh> straightNeedleGeom) { m_needleGeom = straightNeedleGeom; }
    std::shared_ptr<LineMesh> getNeedleGeometry() const { return m_needleGeom; }

    ///
    /// \brief Returns axes/direciton of the needle
    ///
    Vec3d getNeedleDirection() const override;

    ///
    /// \brief Returns tip of the needle
    ///
    Vec3d getNeedleTip() const override { return getNeedleStart(); }

    const Vec3d& getNeedleStart() const;
    const Vec3d& getNeedleEnd() const;

protected:
    std::shared_ptr<LineMesh> m_needleGeom;
};
} // namespace imstk