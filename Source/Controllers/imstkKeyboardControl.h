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
