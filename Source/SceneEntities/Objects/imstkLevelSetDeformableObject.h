/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkDynamicObject.h"
#include "imstkMacros.h"

namespace imstk
{
class LevelSetSystem;

///
/// \class LevelSetDeformableObject
///
/// \brief Base class for scene objects that move and/or deform under position
/// based dynamics formulation, implements the PbdSystem and PbdSolver
///
class LevelSetDeformableObject : public DynamicObject
{
public:
    LevelSetDeformableObject(const std::string& name = "LevelSetDeformableObject") : DynamicObject(name) { }
    ~LevelSetDeformableObject() override = default;

    IMSTK_TYPE_NAME(LevelSetDeformableObject)

    ///
    /// \brief Get the LevelSet model of the object
    ///
    std::shared_ptr<LevelSetSystem> getLevelSetModel();

    ///
    /// \brief Initialize the LevelSet scene object
    ///
    bool initialize() override;

protected:
    std::shared_ptr<LevelSetSystem> m_levelSetModel = nullptr; ///< LevelSet mathematical model
};
} // namespace imstk