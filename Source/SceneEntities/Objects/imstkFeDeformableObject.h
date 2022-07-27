/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkDynamicObject.h"
#include "imstkMacros.h"
#include "imstkMath.h"

namespace imstk
{
class FemDeformableBodyModel;

///
/// \class FeDeformableObject
///
/// \brief Scene objects that can deform
///
class FeDeformableObject : public DynamicObject
{
public:
    FeDeformableObject(const std::string& name = "FeDeformableObject") : DynamicObject(name) { }
    ~FeDeformableObject() override = default;

    IMSTK_TYPE_NAME(FeDeformableObject)

    ///
    /// \brief Initialize the deformable object
    ///
    bool initialize() override;

    std::shared_ptr<FemDeformableBodyModel> getFEMModel();

protected:
    std::shared_ptr<FemDeformableBodyModel> m_femModel = nullptr;
};
} // namespace imstk