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

class ArcNeedle : public Needle
{
public:
    ArcNeedle(const std::string& name = "ArcNeedle") : Needle(name)
    {
        const Mat4d rot = mat4dRotation(Rotd(-PI_2, Vec3d(0.0, 1.0, 0.0))) *
                          mat4dRotation(Rotd(-0.6, Vec3d(1.0, 0.0, 0.0)));

        // Manually setup an arc aligned with the geometry, some sort of needle+arc generator
        // could be a nice addition to imstk
        Mat3d arcBasis = Mat3d::Identity();
        arcBasis.col(0) = Vec3d(0.0, 0.0, -1.0);
        arcBasis.col(1) = Vec3d(1.0, 0.0, 0.0);
        arcBasis.col(2) = Vec3d(0.0, 1.0, 0.0);
        arcBasis = rot.block<3, 3>(0, 0) * arcBasis;
        const Vec3d  arcCenter = (rot * Vec4d(0.0, -0.005455, 0.008839, 1.0)).head<3>();
        const double arcRadius = 0.010705;
        setArc(arcCenter, arcBasis, arcRadius, 0.558, 2.583);
    }

    ~ArcNeedle() override = default;

    void setArc(const Vec3d& arcCenter, const Mat3d& arcBasis,
                double arcRadius, double beginRad, double endRad)
    {
        m_arcCenter = arcCenter;
        m_arcBasis  = arcBasis;
        m_beginRad  = beginRad;
        m_endRad    = endRad;
        m_arcRadius = arcRadius;
    }

    ///
    /// \brief Get the basis post transformation of the rigid body
    ///
    const Mat3d& getArcBasis() const { return m_arcBasis; }

    ///
    /// \brief Get the arc center post transformation of the rigid body
    ///
    const Vec3d& getArcCenter() { return m_arcCenter; }
    const double getBeginRad() const { return m_beginRad; }
    const double getEndRad() const { return m_endRad; }
    const double getArcRadius() const { return m_arcRadius; }

protected:
    std::shared_ptr<LineMesh> m_needleGeom;

    Mat3d  m_arcBasis  = Mat3d::Identity();
    Vec3d  m_arcCenter = Vec3d::Zero();
    double m_arcRadius = 1.0;
    double m_beginRad  = 0.0;
    double m_endRad    = PI * 2.0;
};
} // namespace imstk