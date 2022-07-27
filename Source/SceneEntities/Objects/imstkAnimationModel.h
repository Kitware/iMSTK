/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkGeometry.h"

namespace imstk
{
///
/// \class AnimationModel
///
/// \brief Contains geometric and animation render information
///
class AnimationModel
{
public:
    AnimationModel(std::shared_ptr<Geometry> geometry);
    AnimationModel() = delete;

    ///
    /// \brief Get/set geometry
    ///@{
    std::shared_ptr<Geometry> getGeometry();
    virtual void setGeometry(std::shared_ptr<Geometry> geometry);
    ///@}

    ///
    /// \brief Update animation
    ///
    virtual void update() {};

    ///
    /// \brief Reset animation
    ///
    virtual void reset() {};

protected:
    std::shared_ptr<Geometry> m_geometry = nullptr;
};
} // namespace imstk