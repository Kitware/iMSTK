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
class PbdModel;

///
/// \class PbdObject
///
/// \brief Base class for scene objects that move and/or deform under position
/// based dynamics formulation, implements the PbdModel and PbdSolver
///
class PbdObject : public DynamicObject
{
public:
    PbdObject(const std::string& name = "PbdObject") : DynamicObject(name) { }
    ~PbdObject() override = default;

    IMSTK_TYPE_NAME(PbdObject)

    ///
    /// \biref Get the Pbd model of the object
    ///
    std::shared_ptr<PbdModel> getPbdModel();

    ///
    /// \brief Initialize the Pbd scene object
    ///
    bool initialize() override;

protected:
    std::shared_ptr<PbdModel> m_pbdModel = nullptr; ///< Pbd mathematical model
};
} // namespace imstk