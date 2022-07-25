/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

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