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

#include <memory>

// imstk
#include "imstkMath.h"
#include "imstkDeviceClient.h"

namespace imstk
{
///
/// \class DeviceTracker
///
/// \brief This class reports external device's position and orientation with a given offset
///
class DeviceTracker
{
public:

    enum InvertFlag
    {
        transX = 0x01,
        transY = 0x02,
        transZ = 0x04,
        rotX = 0x08,
        rotY = 0x10,
        rotZ = 0x20
    };

    ///
    /// \brief Constructor
    ///
    DeviceTracker(std::shared_ptr<DeviceClient> deviceClient) :
        m_deviceClient(deviceClient) {}

    ///
    /// \brief Destructor
    ///
    ~DeviceTracker() = default;

    ///
    /// \brief Compute the world position and orientation
    ///
    bool updateTrackingData();

    ///
    /// \brief Get/Set the device client
    ///
    std::shared_ptr<DeviceClient> getDeviceClient() const;
    void setDeviceClient(std::shared_ptr<DeviceClient> deviceClient);

    ///
    /// \brief Get/Set the current scaling factor
    ///
    double getTranslationScaling() const;
    void setTranslationScaling(double scaling);

    ///
    /// \brief Get/Set the translation offset
    ///
    const Vec3d& getTranslationOffset() const;
    void setTranslationOffset(const Vec3d& t);

    ///
    /// \brief Get/Set the rotation offset
    ///
    const Quatd& getRotationOffset();
    void setRotationOffset(const Quatd& r);

    ///
    /// \brief Get/Set the inversion flags
    ///
    unsigned char getInversionFlags();
    void setInversionFlags(unsigned char f);

    ///
    /// \brief Sets the tracking data to be out of date or up to date
    ///
    void setTrackerToOutOfDate() { m_trackingDataUptoDate = false; }
    void setTrackerToUpToDate() { m_trackingDataUptoDate = true; }

    ///
    /// \brief Returns true if the tracking data is already updated in current frame. Else, false.
    ///
    bool isTrackerUpToDate() const { return m_trackingDataUptoDate; }

    ///
    /// \brief Get the latest position
    ///
    const Vec3d& getPosition(){ return m_currentPos; };

    ///
    /// \brief Get the latest rotation
    ///
    const Quatd& getRotation() { return m_currentRot; };

protected:

    std::shared_ptr<DeviceClient> m_deviceClient; ///< Reports device tracking information
    double m_scaling = 1.0;                       ///< Scaling factor for physical to virtual translations
    Vec3d m_translationOffset = WORLD_ORIGIN;     ///< Translation concatenated to the device translation
    Quatd m_rotationOffset = Quatd::Identity();   ///< Rotation concatenated to the device rotation
    unsigned char m_invertFlags = 0x00;           ///< Invert flags to be masked with DeviceTracker::InvertFlag

    Vec3d m_currentPos;
    Quatd m_currentRot;
    bool m_trackingDataUptoDate = false; ///< Keeps track of the device update in a given frame
};
} // imstk

