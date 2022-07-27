/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkMath.h"
#include "imstkVecDataArray.h"

namespace imstk
{
class PointSet;

///
/// \class PbdState
///
/// \brief State of the body governed by PBD mathematical model
///
class PbdState
{
public:
    PbdState(const int numElements) :
        m_pos(std::make_shared<VecDataArray<double, 3>>(numElements)),
        m_vel(std::make_shared<VecDataArray<double, 3>>(numElements)),
        m_acc(std::make_shared<VecDataArray<double, 3>>(numElements))
    {
        std::fill_n(m_pos->getPointer(), numElements, Vec3d::Zero());
        std::fill_n(m_vel->getPointer(), numElements, Vec3d::Zero());
        std::fill_n(m_acc->getPointer(), numElements, Vec3d::Zero());
    }

    virtual ~PbdState() = default;

    ///
    /// \brief Returns the vector of current nodal accelerations
    ///
    std::shared_ptr<VecDataArray<double, 3>> getAccelerations() { return m_acc; }

    ///
    /// \brief Returns the vector of current nodal positions
    ///
    std::shared_ptr<VecDataArray<double, 3>> getPositions() { return m_pos; }

    ///
    /// \brief Returns the vector of current nodal velocities
    ///
    std::shared_ptr<VecDataArray<double, 3>> getVelocities() { return m_vel; }

    ///
    /// \brief Set the vector for accelerations
    ///
    void setAccelerations(std::shared_ptr<VecDataArray<double, 3>> accelerations) { m_acc = accelerations; }

    ///
    /// \brief Sets the vector that stores the positions
    ///
    void setPositions(std::shared_ptr<VecDataArray<double, 3>> positions) { m_pos = positions; }

    ///
    /// \brief Set the vector for velocities
    ///
    void setVelocities(std::shared_ptr<VecDataArray<double, 3>> velocities) { m_vel = velocities; }

    ///
    /// \brief Set the state to a given one, copies vector values by value instead of references
    ///
    void setState(std::shared_ptr<PbdState> rhs);

private:
    std::shared_ptr<VecDataArray<double, 3>> m_pos;   ///< Nodal positions
    std::shared_ptr<VecDataArray<double, 3>> m_vel;   ///< Nodal velocities
    std::shared_ptr<VecDataArray<double, 3>> m_acc;   ///< Nodal acelerations
};
} // namespace imstk