/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkDeviceClient.h"

#include <unordered_map>

namespace imstk
{
using KeyStateType = int;
#define KEY_RELEASE 0
#define KEY_PRESS 1

///
/// \class KeyEvent
///
/// \brief Provides the information of a key event (press, release, & which key)
///
class KeyEvent : public Event
{
public:
    KeyEvent(const std::string type, const char key, const KeyStateType keyPressType) : Event(type), m_keyPressType(keyPressType), m_key(key) { }
    ~KeyEvent() override = default;

public:
    KeyStateType m_keyPressType;
    const char   m_key = -1;
};

///
/// \class KeyboardDeviceClient
///
/// \brief This class provides the keyboard state it also posts events
///
class KeyboardDeviceClient : public DeviceClient
{
protected:
    ///
    /// This object is only creatable through its New method
    ///
    KeyboardDeviceClient() : DeviceClient("KeyboardDevice", "") { }

    ///
    /// This object is only creatable through this method
    ///
    static std::shared_ptr<KeyboardDeviceClient> New();

public:
    ~KeyboardDeviceClient() override = default;

    // Only these classes may provide this object
    friend class VTKInteractorStyle;
    friend class ConsoleModule;

public:
    // *INDENT-OFF*
    ///
    /// \brief Posted when key is pressed (not continuously)
    ///
    SIGNAL(KeyboardDeviceClient, keyPress);

    ///
    /// \brief Posted when key is released (not continuously)
    ///
    SIGNAL(KeyboardDeviceClient, keyRelease);
    // *INDENT-ON*

public:
    const std::unordered_map<int, KeyStateType>& getKeyState() const { return m_buttons; }

protected:
    ///
    /// \brief Post a key press
    ///
    void emitKeyDown(char key);

    ///
    /// \brief Post a key release
    ///
    void emitKeyUp(char key);

    ///
    /// \brief Returns true if key is currently down
    ///
    bool isKeyDown(const char key) const;
};
} // namespace imstk