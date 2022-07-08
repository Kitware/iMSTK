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

#include "imstkSceneObject.h"
#include "imstkMacros.h"

namespace imstk
{
class DeviceClient;

///
/// \class DeviceControl
///
/// \brief While the DeviceClient provides quantities from the device, the control
/// defines what to do with those quantities
///
class DeviceControl : public SceneObject
{
protected:
    DeviceControl(const std::string& name = "DeviceControl") : SceneObject(name) { }

public:
    ~DeviceControl() override = default;

    ///
    /// \brief Set/Get the device client used in the control
    ///@{
    std::shared_ptr<DeviceClient> getDevice() const { return m_deviceClient; }
    virtual void setDevice(std::shared_ptr<DeviceClient> device) { m_deviceClient = device; }
    ///@}

    using SceneObject::update;

    ///
    /// \brief Prints the controls
    ///
    virtual void printControls() { }

    ///
    /// \brief Updates control based on current device state
    ///
    virtual void update(const double imstkNotUsed(dt)) { }

    void initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink) override;

protected:
    std::shared_ptr<DeviceClient> m_deviceClient;
};
} // namespace imstk