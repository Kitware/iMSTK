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
/// \class FeDeformBodyState
///
/// \brief This class stores the state of the unknown field variable in vectorized form
///
class FeDeformBodyState
{
public:
    FeDeformBodyState() = default;
    FeDeformBodyState(const size_t size) { initialize(size); }
    virtual ~FeDeformBodyState() = default;

    ///
    /// \brief Initialize the problem state
    ///
    void initialize(const size_t numDof);

    ///
    /// \brief Set the state to a given one
    ///
    void setState(const Vectord& u, const Vectord& v, const Vectord& a);

    ///
    /// \brief Set the current state
    ///
    void setU(const Vectord& u);

    ///
    /// \brief Set the time derivative of state
    ///
    void setV(const Vectord& v);

    ///
    /// \brief Set double time derivative of the state
    ///
    void setA(const Vectord& a);

    ///
    /// \brief Set the state to a given one
    ///
    void setState(std::shared_ptr<FeDeformBodyState> rhs);

    ///
    /// \brief Get the state
    ///
    //const Vectord& getQ() const { return m_q; }
    Vectord& getQ() { return m_q; }

    ///
    /// \brief Get the derivative of state w.r.t time
    ///
    //const Vectord& getQDot() const { return m_qDot; }
    Vectord& getQDot() { return m_qDot; }

    ///
    /// \brief Get the double derivative of state w.r.t time
    ///
    //const Vectord& getQDotDot() const { return m_qDotDot; }
    Vectord& getQDotDot() { return m_qDotDot; }

    ///
    /// \brief Get the state
    ///
    //const Vectord& getState() const { return getQ(); }
    Vectord& getState() { return getQ(); }

protected:
    Vectord m_q;         // State
    Vectord m_qDot;      // Derivative of state w.r.t time
    Vectord m_qDotDot;   // Double derivative of state w.r.t time
};
} // namespace imstk