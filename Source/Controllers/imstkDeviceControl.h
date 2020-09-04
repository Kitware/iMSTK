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

#include "imstkEventObject.h"

#include <memory>

namespace imstk
{
class DeviceClient;

///
/// \class DeviceControl
///
/// \brief While the DeviceClient provides quantities from the device, the control
/// defines what to do with those quantities
///
class DeviceControl : public EventObject
{
protected:
    DeviceControl() = default;
    DeviceControl(std::shared_ptr<DeviceClient> device) : m_deviceClient(device) { }

public:
    virtual ~DeviceControl() = default;

public:
    std::shared_ptr<DeviceClient> getDevice() const { return m_deviceClient; }

    virtual void setDevice(std::shared_ptr<DeviceClient> device) { m_deviceClient = device; }

    ///
    /// \brief Prints the controls
    /// 
    virtual void printControls() { }

    ///
    /// \brief Updates the state of the control
    ///
    virtual void update() { }

public:
    std::shared_ptr<DeviceClient> m_deviceClient;
};
} // imstk
