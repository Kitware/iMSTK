/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkMacros.h"
#include "imstkPbdObjectCollision.h"
#include "imstkNeedlePbdCH.h"

namespace imstk
{
class PbdObject;

///
/// \class NeedleInteraction
///
/// \brief Defines interaction between NeedleObject and PbdObject
///
class NeedleInteraction : public PbdObjectCollision
{
public:
    NeedleInteraction(std::shared_ptr<PbdObject> tissueObj,
                      std::shared_ptr<PbdObject> needleObj,
                      std::shared_ptr<PbdObject> threadObj);
    ~NeedleInteraction() override = default;

    IMSTK_TYPE_NAME(NeedleInteraction)

    void stitch();

    const NeedlePbdCH::PunctureData& getPunctureData();
};
} // namespace imstk