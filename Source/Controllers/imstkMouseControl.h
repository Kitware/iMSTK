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
class MouseControl : public DeviceControl
{
public:
    MouseControl() = default;
    MouseControl(std::shared_ptr<MouseDeviceClient> mouseDevice);
    virtual ~MouseControl() override = default;

public:
    virtual void setDevice(std::shared_ptr<DeviceClient> device) override;

public:
    virtual void OnButtonPress(const int imstkNotUsed(key)) { }

    virtual void OnButtonRelease(const int imstkNotUsed(key)) { }

    virtual void OnScroll(const double imstkNotUsed(dx)) { }

    virtual void OnMouseMove(const Vec2d& imstkNotUsed(pos)) { }

    ///
    /// \brief Posts key press/release events
    /// override to filter the event, or implement OnkeyDown/OnKeyUp
    ///
    virtual void mouseEvent(MouseEvent* e);

protected:
    std::shared_ptr<MouseDeviceClient> m_mouseDeviceClient;
};
}
