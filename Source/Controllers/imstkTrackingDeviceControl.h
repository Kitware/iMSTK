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

namespace imstk
{
///
/// \class TrackingDeviceControl
///
/// \brief Base class for all DeviceControls that track
/// something in space (position and orientation)
///
class TrackingDeviceControl : public DeviceControl
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

public:
    TrackingDeviceControl();
    TrackingDeviceControl(std::shared_ptr<DeviceClient> device);
    virtual ~TrackingDeviceControl() override = default;

public:
    ///
    /// \brief Update controlled scene objects using latest tracking information
    ///
    virtual void updateControlledObjects() = 0;

    ///
    /// \brief Apply forces to the haptic device
    ///
    virtual void applyForces() = 0;

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
    const Vec3d& getPosition() const { return m_currentPos; };

    ///
    /// \brief Set the position of the tracker
    ///
    void setPosition(const Vec3d& pos) { this->m_currentPos = pos; }

    ///
    /// \brief Get the latest rotation
    ///
    const Quatd& getRotation() const { return m_currentRot; };

    ///
    /// \brief Set the orientation of the tracker
    ///
    void setRotation(const Quatd& orientation) { this->m_currentRot = orientation; }

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
    /// \brief Compute the world position and orientation
    ///
    bool updateTrackingData();

protected:
    double m_scaling = 1.0;                                ///< Scaling factor for physical to virtual translations
    Vec3d  m_translationOffset;                            ///< Translation concatenated to the device translation
    Quatd  m_rotationOffset;                               ///< Rotation concatenated to the device rotation
    unsigned char m_invertFlags = 0x00;                    ///< Invert flags to be masked with DeviceTracker::InvertFlag

    Vec3d m_currentPos;
    Quatd m_currentRot;
    bool  m_trackingDataUptoDate = false;
};
} // imstk
