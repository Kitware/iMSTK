/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkConsoleModule.h"
#include "imstkKeyboardDeviceClient.h"

namespace imstk
{
ConsoleModule::ConsoleModule(std::string name) :
    m_keyboardDeviceClient(KeyboardDeviceClient::New())
{
}

void
ConsoleModule::updateModule()
{
    const char c = static_cast<char>(getchar());

    // Simulate key press/release
    m_keyboardDeviceClient->emitKeyDown(c);
    m_keyboardDeviceClient->emitKeyUp(c);
}
} // namespace imstk