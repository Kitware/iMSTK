/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkDeviceClient.h"

#include <unordered_map>

class vtkInteractorStyleVR;

namespace imstk
{
///
/// \class OpenVRDeviceClient
///
/// \brief This class provides quantities for the specified VR device
/// The devices creation is tied to the viewer. It is only acquirable
/// from a VR viewer and exists on the viewers thread.
///
class OpenVRDeviceClient : public DeviceClient
{
protected:
    ///
    /// This object is only creatable through its New method
    ///
    OpenVRDeviceClient(DeviceType deviceType) : DeviceClient("OpenVRDevice", ""),
        m_deviceType(deviceType), m_trackpadPosition(Vec2d::Zero()) { }

    ///
    /// This object is only creatable through this method
    ///
    static std::shared_ptr<OpenVRDeviceClient> New(DeviceType deviceType);

public:
    virtual ~OpenVRDeviceClient() override = default;

    // Only the viewer is allowed to provide these objects
    friend class ::vtkInteractorStyleVR;

public:
    DeviceType getDeviceType() const { return m_deviceType; }

    const Vec2d& getTrackpadPosition() { return m_trackpadPosition; }
    void setTrackpadPosition(const Vec2d& pos) { m_trackpadPosition = pos; }

    ///
    /// \brief Set the current position and orientation
    ///
    void setPose(const Vec3d& pos, const Quatd& orientation)
    {
        m_trackingEnabled = true;
        m_position    = pos;
        m_orientation = orientation;
    }

protected:
    ///
    /// \brief Emit various button events
    ///
    void emitButtonTouched(const int buttonId);
    void emitButtonUntouched(const int buttonId);
    void emitButtonPress(const int buttonId);
    void emitButtonRelease(const int buttonId);

private:
    DeviceType m_deviceType;
    Vec2d      m_trackpadPosition;
};
} // namespace imstk