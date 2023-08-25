/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkComponent.h"
#include "imstkPuncture.h"

namespace imstk
{
///
/// \class Needle
///
/// \brief Base for all needles in imstk it supports global puncture state,
/// per object puncture state, and per id/cell puncture state
///
class Needle : public Component
{
public:
    Needle(const std::string& name = "Needle") : Component(name) { }
    ~Needle() override = default;

    ///
    /// \brief Get/Set puncture data
    /// @{
    void setPuncture(const PunctureId& id, std::shared_ptr<Puncture> data);
    std::shared_ptr<Puncture> getPuncture(const PunctureId& id);
    /// @}

    ///
    /// \brief Get/set puncture state. This can be done through data too
    /// but this supports the allocation of new puncture data should you
    /// query a non-existent puncture.
    /// @{
    void setState(const PunctureId& id, const Puncture::State state);
    Puncture::State getState(const PunctureId& id);
    /// @}

    PunctureMap& getPunctures() { return m_punctures; }

    ///
    /// \brief Get if inserted at all
    ///
    bool getInserted() const;

    ///
    /// \brief Returns direction at needle tip.
    ///
    virtual Vec3d getNeedleDirection() const { return Vec3d(1.0, 0.0, 0.0); }
    virtual Vec3d getNeedleTip() const { return Vec3d(0.0, 0.0, 0.0); }

protected:
    PunctureMap m_punctures;
};
} // namespace imstk