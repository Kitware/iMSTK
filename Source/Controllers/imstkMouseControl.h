/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkDeviceControl.h"
#include "imstkMath.h"
#include "imstkMacros.h"

namespace imstk
{
class MouseDeviceClient;
class MouseEvent;

///
/// \class MouseControl
///
/// \brief This is the base class for mouse based controls
/// It can be constructed and observed or subclassed and overridden
/// to implement controls.
///
class MouseControl : public DeviceControl, public std::enable_shared_from_this<MouseControl>
{
public:
    MouseControl(const std::string& name = "MouseControl") : DeviceControl(name) { }
    ~MouseControl() override = default;

    void setDevice(std::shared_ptr<DeviceClient> device) override;

    virtual void OnButtonPress(const int imstkNotUsed(key)) { }

    virtual void OnButtonRelease(const int imstkNotUsed(key)) { }

    virtual void OnScroll(const double imstkNotUsed(dx)) { }

    virtual void OnMouseMove(const Vec2d& imstkNotUsed(pos)) { }

private:
    ///
    /// \brief Recieves button presses
    ///
    virtual void mouseButtonPressEvent(MouseEvent* e);

    ///
    /// \brief Recieves button releases
    ///
    virtual void mouseButtonReleaseEvent(MouseEvent* e);

    ///
    /// \brief Recieves scrolls
    ///
    virtual void mouseScrollEvent(MouseEvent* e);

    ///
    /// \brief Recieves moves
    ///
    virtual void mouseMoveEvent(MouseEvent* e);

protected:
    std::shared_ptr<MouseDeviceClient> m_mouseDevice = nullptr;
};
} // namespace imstk
