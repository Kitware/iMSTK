/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkMacros.h"
#include "imstkPbdCollisionHandling.h"

#include "NeedleObject.h"

using namespace imstk;

///
/// \class NeedlePbdCH
///
/// \brief Surface collision disabled upon puncture
///
class NeedlePbdCH : public PbdCollisionHandling
{
public:
    NeedlePbdCH() = default;
    ~NeedlePbdCH() override = default;

    IMSTK_TYPE_NAME(NeedlePbdCH)

protected:
    void handle(
        const std::vector<CollisionElement>& elementsA,
        const std::vector<CollisionElement>& elementsB) override
    {
        // Don't handle collision data when punctured
        auto needleObj = std::dynamic_pointer_cast<NeedleObject>(getInputObjectB());
        if (needleObj->getCollisionState() == NeedleObject::CollisionState::TOUCHING)
        {
            PbdCollisionHandling::handle(elementsA, elementsB);
        }
    }
};