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
        auto tissueObj = std::dynamic_pointer_cast<PbdObject>(getInputObjectA());
        auto needleObj = std::dynamic_pointer_cast<NeedleObject>(getInputObjectB());
        if ((elementsA.size() > 0 || elementsB.size() > 0)
            && needleObj->getCollisionState(tissueObj) == NeedleObject::CollisionState::REMOVED)
        {
            needleObj->setCollisionState(tissueObj, NeedleObject::CollisionState::TOUCHING);
        }

        // Don't handle collision data when punctured
        if (needleObj->getCollisionState(tissueObj) == NeedleObject::CollisionState::TOUCHING)
        {
            PbdCollisionHandling::handle(elementsA, elementsB);
        }
    }
};