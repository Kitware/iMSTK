/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkLinearProjectionConstraint.h"

namespace  imstk
{
LinearProjectionConstraint::
LinearProjectionConstraint(const size_t& nodeId,
                           const bool    isFixed /*= false*/) :
    m_isFixedConstraint(false),
    m_projection(Mat3d::Identity()),
    m_value(Vec3d(0., 0., 0.))
{
    m_nodeId = nodeId;
    if (isFixed)
    {
        m_projection = Mat3d::Zero();
        m_isFixedConstraint = true;
    }
}

void
LinearProjectionConstraint::setProjection(const size_t& nodeId, const Vec3d& p, const Vec3d& q /*= Vec3d::Zero()*/)
{
    m_nodeId     = nodeId;
    m_projection = Mat3d::Identity() - p * p.transpose() - q * q.transpose();
}

void
LinearProjectionConstraint::setProjectionToLine(const size_t& nodeId, const Vec3d& p)
{
    m_nodeId = nodeId;
    auto v = p / p.norm();
    m_projection = v * v.transpose();
}

void
LinearProjectionConstraint::setProjectorToDirichlet(const unsigned int& nodeId, const Vec3d z)
{
    m_nodeId     = nodeId;
    m_projection = Mat3d::Zero();
    m_isFixedConstraint = true;
    m_value = z;
}

void
LinearProjectionConstraint::reset()
{
    m_projection = Mat3d::Identity();
    m_value      = Vec3d(0.0, 0.0, 0.0);
}
} // namespace imstk