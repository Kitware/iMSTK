/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkMacros.h"
#include "imstkNeedle.h"
#include "imstkPbdCollisionHandling.h"
#include "imstkPbdObject.h"

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
        auto puncturable = getInputObjectA()->getComponent<Puncturable>();
        auto needle      = getInputObjectB()->getComponent<Needle>();

        const PunctureId punctureId = getPunctureId(needle, puncturable);

        if ((elementsA.size() > 0 || elementsB.size() > 0)
            && needle->getState(punctureId) == Puncture::State::REMOVED)
        {
            needle->setState(punctureId, Puncture::State::TOUCHING);
            puncturable->setPuncture(punctureId, needle->getPuncture(punctureId));
        }

        // Don't handle collision data when punctured
        if (needle->getState(punctureId) == Puncture::State::TOUCHING)
        {
            PbdCollisionHandling::handle(elementsA, elementsB);
        }
    }
};