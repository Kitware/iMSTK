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
class SphModel;

///
/// \class SphObject
///
/// \brief Base class for scene objects that move and/or deform under
/// smooth particle hydrodynamics
///
class SphObject : public DynamicObject
{
public:
    SphObject(const std::string& name = "SphObject") : DynamicObject(name) { }
    ~SphObject() override = default;

    IMSTK_TYPE_NAME(SphObject)

    ///
    /// \brief Get the model governing the Sph fluid dynamics of this object
    ///
    std::shared_ptr<SphModel> getSphModel();

    ///
    /// \brief Initialize the SphObject
    ///
    bool initialize() override;

protected:
    std::shared_ptr<SphModel> m_sphModel = nullptr;
};
} // namespace imstk