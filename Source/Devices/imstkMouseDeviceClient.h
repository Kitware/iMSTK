/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

=========================================================================*/

#pragma once

#include "imstkDeviceClient.h"

#include <unordered_map>

namespace imstk
{
using MouseActionType = int;
#define MOUSE_RELEASE 0
#define MOUSE_PRESS 1

using MouseButtonType = int;
#define LEFT_BUTTON 0
#define RIGHT_BUTTON 1
#define MIDDLE_BUTTON 2

///
/// \class MouseEvent
///
/// \brief Provides the information of a mouse event, this includes button
/// presses/releases and scrolling
///
class MouseEvent : public Event
{
public:
    MouseEvent(const std::string type, const MouseButtonType buttonId) :
        Event(type), m_scrollDx(0.0), m_buttonId(buttonId)
    {
    }

    MouseEvent(std::string type, const double scrollDx) :
        Event(type), m_scrollDx(scrollDx), m_buttonId(0)
    {
    }

    MouseEvent(std::string type) :
        Event(type), m_scrollDx(0.0), m_buttonId(0)
    {
    }

    ~MouseEvent() override = default;

public:
    double m_scrollDx;          ///> Mouse scroll
    MouseButtonType m_buttonId; ///> Button id
};

///
/// \class KeyboardDeviceClient
///
/// \brief This class provides the keyboard state it also posts button events
///
class MouseDeviceClient : public DeviceClient
{
protected:
    ///
    /// This object is only creatable through its New method
    ///
    MouseDeviceClient() : DeviceClient("MouseDevice", ""),
        m_prevPos(Vec2d(0.0, 0.0)), m_pos(Vec2d(0.0, 0.0)) { }

    ///
    /// This object is only creatable through this method
    ///
    static std::shared_ptr<MouseDeviceClient> New();

public:
    ~MouseDeviceClient() override = default;

    // Only the viewer is allowed to provide these objects
    friend class VTKInteractorStyle;

public:
    // *INDENT-OFF*
    ///
    /// \brief Posted when a button on the mouse is pressed (not continuously)
    ///
    SIGNAL(MouseDeviceClient, mouseButtonPress);

    ///
    /// \brief Posted when a button on the mouse is released (not continuously)
    ///
    SIGNAL(MouseDeviceClient, mouseButtonRelease);

    ///
    /// \brief Posted when mouse scrolls
    ///
    SIGNAL(MouseDeviceClient, mouseScroll);

    ///
    /// \brief Posted when mouse moves
    ///
    SIGNAL(MouseDeviceClient, mouseMove);
    // *INDENT-ON*

public:
    ///
    /// \brief Get the current position of the mouse
    ///
    const Vec2d& getPos() const { return m_pos; }

    ///
    /// \brief Get the previous position of the mouse
    ///
    const Vec2d& getPrevPos() const { return m_prevPos; }

    ///
    /// \brief Query if the button is down
    ///
    bool isButtonDown(const MouseButtonType buttonId) const
    {
        if (m_buttons.find(buttonId) != m_buttons.end())
        {
            return m_buttons.at(buttonId) == MOUSE_PRESS;
        }
        else
        {
            return MOUSE_RELEASE;
        }
    }

protected:
    ///
    /// \brief Update the mouse position
    ///
    void updateMousePos(const Vec2d& pos)
    {
        m_prevPos = m_pos;
        m_pos     = pos;
        this->postEvent(MouseEvent(MouseDeviceClient::mouseMove()));
    }

    ///
    /// \brief Post a click down event
    ///
    void emitButtonPress(const MouseButtonType buttonId)
    {
        m_buttons[buttonId] = MOUSE_PRESS;
        this->postEvent(MouseEvent(MouseDeviceClient::mouseButtonPress(), buttonId));
    }

    ///
    /// \brief Post a click release event
    ///
    void emitButtonRelease(const MouseButtonType buttonId)
    {
        m_buttons[buttonId] = MOUSE_RELEASE;
        this->postEvent(MouseEvent(MouseDeviceClient::mouseButtonRelease(), buttonId));
    }

    ///
    /// \brief Post a mouse scroll event
    ///
    void emitScroll(const double dx) { this->postEvent(MouseEvent(MouseDeviceClient::mouseScroll(), dx)); }

protected:
    Vec2d m_prevPos;
    Vec2d m_pos;
};
}