/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkMath.h"

namespace imstk
{
///
/// \class LinearProjectionConstraint
///
/// \brief Linear projection constraint
///
class LinearProjectionConstraint
{
public:
    LinearProjectionConstraint(const size_t& nodeId, const bool isFixed = false);
    LinearProjectionConstraint() = delete;

    ~LinearProjectionConstraint() = default;

    ///
    /// \brief Form the projection
    ///
    void setProjection(const size_t& nodeId, const Vec3d& p, const Vec3d& q = Vec3d::Zero());

    ///
    /// \brief Form the projection
    ///
    void setProjectionToLine(const size_t& nodeId, const Vec3d& p);

    ///
    /// \brief Set the projector to simulate Dirichlet conditions
    ///
    void setProjectorToDirichlet(const size_t& nodeId);
    void setProjectorToDirichlet(const unsigned int& nodeId, const Vec3d z);

    ///
    /// \brief Reset the linear projector
    ///
    void reset();

    ///
    /// \brief Set the value in the restricted subspace
    ///
    inline void setValue(const Vec3d& v) { m_value = v; }

    ///
    /// \brief Get the projector
    ///
    inline const Mat3d& getProjector() const { return m_projection; }

    ///
    /// \brief Get the value
    ///
    inline const Vec3d& getValue() const { return m_value; }

    ///
    /// \brief Get the node id
    ///
    inline const size_t& getNodeId() const { return m_nodeId; }

    ///
    /// \brief Returns true if the constraint is fixed
    ///
    inline bool isFixed() const { return m_isFixedConstraint; }

private:
    size_t m_nodeId;            ///< Node id
    bool   m_isFixedConstraint; ///< Flag to know if that node is fixed
    Mat3d  m_projection;        ///< Orthogonal projector
    Vec3d  m_value;             ///< Value in the subspace: range(I-m_projector)
};
} // imstk
