/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkDummyClient.h"
#include "imstkLogger.h"

namespace imstk
{
void
DummyClient::setButton(const unsigned int buttonId, const bool buttonStatus)
{
    auto x = m_buttons.find(buttonId);
    if (x != m_buttons.end())
    {
        x->second = buttonStatus;
    }
}
} // namespace imstk