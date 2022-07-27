/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkRbdConstraint.h"

using namespace imstk;

///
/// \class RbdPointToArcConstraint
///
/// \brief Constrains an rigid body arc geometry to a point by computing the
/// linear force and angular torque to get the arc to the point
///
class RbdPointToArcConstraint : public RbdConstraint
{
public:
    ///
    /// \param the Rigid body
    /// \param the center of the circle arc is defined with
    /// \param the radians/range of the arc, with relation to the arcBasis
    /// \param the radians/range of the arc, with relation to the arcBasis
    /// \param the radius of the circle the arc is defined with
    /// \param the basis of the arc. Where any point on the plane has a radian with relation to x,y columns.
    /// and the z column gives the normal of the plane the circle+arc lie on
    /// \param the fixed point
    /// \param baumgarte stabilization, varies step length
    ///
    RbdPointToArcConstraint(
        std::shared_ptr<RigidBody> obj,
        const Vec3d arcCenter, const double beginRadian, const double endRadian,
        const double arcCircleRadius, const Mat3d arcBasis,
        const Vec3d fixedPoint,
        const double beta = 0.05) : RbdConstraint(obj, nullptr, Side::A),
        m_arcCenter(arcCenter), m_beginRadian(beginRadian), m_endRadian(endRadian),
        m_arcCircleRadius(arcCircleRadius), m_arcBasis(arcBasis),
        m_fixedPoint(fixedPoint),
        m_beta(beta)
    {
        // Check orthonormal basis
    }

    ~RbdPointToArcConstraint() override = default;

public:
    void compute(double dt) override
    {
        // Jacobian of contact (defines linear and angular constraint axes)
        J = Eigen::Matrix<double, 3, 4>::Zero();
        if ((m_side == Side::AB || m_side == Side::A) && !m_obj1->m_isStatic)
        {
            // Compute the direction and closest point to the arc from the fixedPoint
            const Vec3d circleDiff = m_fixedPoint - m_arcCenter;
            const Vec3d dir = circleDiff.normalized();

            // m_arcBasis Should be orthonormal, this should project onto the axes
            const Mat3d  invArcBasis = m_arcBasis.transpose();
            const Vec3d  p   = invArcBasis * circleDiff;
            const double rad = atan2(-p[2], -p[0]) + PI;
            // Clamp to range (if closest point on circle is outside on range we want the end)
            const double clampedRad = std::min(std::max(rad, m_beginRadian), m_endRadian);
            // Finally compute the closest point to the arc using the new radian
            const Vec3d closestPt = (cos(clampedRad) * m_arcBasis.col(0) +
                                     sin(clampedRad) * m_arcBasis.col(2)) * m_arcCircleRadius +
                                    m_arcCenter;

            Vec3d       diff = m_fixedPoint - closestPt;
            const Vec3d r    = closestPt - m_obj1->getPosition();
            const Vec3d c    = r.cross(diff);

            vu      = diff.norm() * m_beta / dt;
            diff    = diff.normalized();
            J(0, 0) = diff[0]; J(0, 1) = c[0];
            J(1, 0) = diff[1]; J(1, 1) = c[1];
            J(2, 0) = diff[2]; J(2, 1) = c[2];
        }
    }

private:
    Vec3d  m_arcCenter       = Vec3d::Zero();
    Mat3d  m_arcBasis        = Mat3d::Zero(); // Should be orthonormal
    double m_arcCircleRadius = 0.0;
    double m_beginRadian     = 0.0;
    double m_endRadian       = 0.0;

    Vec3d m_fixedPoint = Vec3d::Zero();

    double m_beta = 0.05;
};