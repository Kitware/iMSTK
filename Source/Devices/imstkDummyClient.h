/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkDeviceClient.h"

#include <unordered_map>

namespace imstk
{
///
/// \class DummyClient
///
/// \brief Allows setting the pose of the device from external caller without a
///  real device connected
///
class DummyClient : public DeviceClient
{
public:
    DummyClient(const std::string& name = "") : DeviceClient(name, "localhost") {}
    ~DummyClient() override = default;

protected:
    ///
    /// \brief Initialize the client
    ///
    void init(const unsigned int numButtons = 0);

    void run()     = delete;
    void cleanUp() = delete;

public:
    ///
    /// \brief Set position
    ///
    void setPosition(const Vec3d& pos);

    ///
    /// \brief Set velocity
    ///
    void setVelocity(const Vec3d& vel);

    ///
    /// \brief Set orientation
    ///
    void setOrientation(const Quatd& orient);

    ///
    /// \brief Set orientation from 4x4 transform
    ///
    void setOrientation(double* transform);

    ///
    /// \brief Set the button status if it exists
    ///
    void setButton(const unsigned int buttonId, const bool buttonStatus);

protected:
    std::unordered_map<int, bool> m_buttons;
};
} // namespace imstk
