/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkDeviceControl.h"
#include "imstkMacros.h"

namespace imstk
{
class KeyboardDeviceClient;
class KeyEvent;

///
/// \class KeyboardSceneController
///
/// \brief This is the base class for keyboard based controls
/// You can construct this and observe it or subclass and override
/// to implement controls.
///
class KeyboardControl : public DeviceControl
{
public:
    KeyboardControl(const std::string& name = "KeyboardControl") : DeviceControl(name) { }
    ~KeyboardControl() override = default;

public:
    void setDevice(std::shared_ptr<DeviceClient> device) override;

public:
    virtual void OnKeyPress(const char imstkNotUsed(key)) { }

    virtual void OnKeyRelease(const char imstkNotUsed(key)) { }

    ///
    /// \brief Recieves key press event
    ///
    virtual void keyPressEvent(KeyEvent* e);

    ///
    /// \brief Recieves key release event
    ///
    virtual void keyReleaseEvent(KeyEvent* e);
};
} // namespace imstk
