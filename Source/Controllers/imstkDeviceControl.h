/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkComponent.h"
#include "imstkEventObject.h"
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
class DeviceControl : public SceneBehaviour, public EventObject
{
protected:
    DeviceControl(const std::string& name = "DeviceControl") : SceneBehaviour(name) { }

public:
    ~DeviceControl() override = default;

    // *INDENT-OFF*
    SIGNAL(DeviceControl, modified);
    // *INDENT-ON*

    ///
    /// \brief Set/Get the device client used in the control
    ///@{
    std::shared_ptr<DeviceClient> getDevice() const { return m_deviceClient; }
    virtual void setDevice(std::shared_ptr<DeviceClient> device) { m_deviceClient = device; }
    ///@}

    ///
    /// \brief Prints the controls
    ///
    virtual void printControls() { }

    void update(const double& imstkNotUsed(dt)) override { }

protected:
    std::shared_ptr<DeviceClient> m_deviceClient;
};
} // namespace imstk