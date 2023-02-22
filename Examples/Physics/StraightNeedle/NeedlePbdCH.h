/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkNeedle.h"
#include "imstkPbdCollisionHandling.h"
#include "imstkPuncturable.h"

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

    void setNeedle(std::shared_ptr<Needle> needle) { m_needle = needle; }
    void setPuncturable(std::shared_ptr<Puncturable> puncturable) { m_puncturable = puncturable; }

protected:
    void handle(
        const std::vector<CollisionElement>& elementsA,
        const std::vector<CollisionElement>& elementsB) override
    {
        CHECK(m_needle != nullptr) << "Needle component not set.";
        CHECK(m_puncturable != nullptr) << "Puncturable component not set.";

        const PunctureId punctureId = getPunctureId(m_needle, m_puncturable);
        if ((elementsA.size() > 0 || elementsB.size() > 0)
            && m_needle->getState(punctureId) == Puncture::State::REMOVED)
        {
            m_needle->setState(punctureId, Puncture::State::TOUCHING);
            m_puncturable->setPuncture(punctureId, m_needle->getPuncture(punctureId));
        }

        // Don't handle collision data when punctured
        if (m_needle->getState(punctureId) == Puncture::State::TOUCHING)
        {
            PbdCollisionHandling::handle(elementsA, elementsB);
        }
    }

private:
    std::shared_ptr<Needle>      m_needle;
    std::shared_ptr<Puncturable> m_puncturable;
};