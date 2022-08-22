/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkModule.h"
#include "imstkMacros.h"

namespace imstk
{
class KeyboardDeviceClient;

///
/// \class ConsoleModule
///
/// \brief The console thread can run separately or in
/// sync to provide keyboard events from the console
/// they should be handled on another thread
///
class ConsoleModule : public Module
{
public:
    ConsoleModule(std::string name = "ConsoleModule");
    ~ConsoleModule() override      = default;

    IMSTK_TYPE_NAME(ConsoleModule)

    std::shared_ptr<KeyboardDeviceClient> getKeyboardDevice() const { return m_keyboardDeviceClient; }

protected:
    bool initModule() override { return true; }

    ///
    /// \brief Awaits input
    ///
    void updateModule() override;

    std::shared_ptr<KeyboardDeviceClient> m_keyboardDeviceClient;
};
} // namespace imstk