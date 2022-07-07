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
class TrackingDeviceControl : public DeviceControl<DeviceClient>
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

protected:
    TrackingDeviceControl(const std::string& name = "TrackingDeviceControl");

public:
    ~TrackingDeviceControl() override = default;

    ///
    /// \brief Apply forces to the haptic device
    ///
    virtual void applyForces();

    ///
    /// \brief Set/Get the position of the tracker
    ///@{
    const Vec3d& getPosition() const;
    void setPosition(const Vec3d& pos);
    ///@}

    ///
    /// \brief Set/Get the orientation of the tracker
    ///@{
    const Quatd& getOrientation() const;;
    void setOrientation(const Quatd& orientation);
    ///@}

    ///
    /// \brief Set/Get whether to compute the velocity from previous and current samples
    /// Useful if a device does not provide the quantity
    ///@{
    void setComputeVelocity(const bool computeVelocity);
    bool getComputeVelocity() const;
    ///@}

    ///
    /// \brief Set/Get whether to compute the anular velocity from previous and current samples
    /// Useful if the device does not provide the quantity
    /// @{
    void setComputeAngularVelocity(const bool computeAngularVelocity);
    bool getComputeAngularVelocity() const;
    /// @}

    ///
    /// \brief Get/Set the angular velocity
    ///@{
    const Vec3d& getAngularVelocity() const;
    void setAngularVelocity(const Vec3d& angularVelocity);
    ///@}

    ///
    /// \brief Get/Set the linear velocity
    ///@{
    const Vec3d& getVelocity() const;
    void setVelocity(const Vec3d& velocity);
    ///@}

    ///
    /// \brief Get/Set the current scaling factor
    ///@{
    double getTranslationScaling() const;
    void setTranslationScaling(const double scaling);
    ///@}

    ///
    /// \brief Get/Set the translation offset
    ///@{
    const Vec3d& getTranslationOffset() const;
    void setTranslationOffset(const Vec3d& t);
    ///@}

    ///
    /// \brief Get/Set the rotation offset, this rotation is applied to the overall
    /// device coordinate system
    ///@{
    const Quatd& getRotationOffset();
    void setRotationOffset(const Quatd& r);
    ///@}

    ///
    /// \brief Get/Set the roation applied to the end effector, this can be used to
    /// register the device in virtual space with regard to the users point of view
    ///@}
    const Quatd& getEffectorRotationOffset();
    void setEffectorRotationOffset(const Quatd& r);
    ///@}

    ///
    /// \brief Get/Set the inversion flags, when set the corresponding axis coordinates
    /// or rotation angle will be negated
    ///@{
    unsigned char getInversionFlags();
    void setInversionFlags(const unsigned char f);
    ///@}

    ///
    /// \brief Update tracking data
    ///
    virtual bool updateTrackingData(const double dt);

protected:
    double m_scaling = 1.0;                                ///< Scaling factor for physical to virtual translations
    Vec3d  m_translationOffset      = Vec3d::Zero();       ///< Translation concatenated to the device translation
    Quatd  m_rotationOffset         = Quatd::Identity();   ///< Rotation concatenated to the device rotation
    Quatd  m_effectorRotationOffset = Quatd::Identity();   ///< Rotation prefixed to the device rotation
    unsigned char m_invertFlags     = 0x00;                ///< Invert flags to be masked with DeviceTracker::InvertFlag

    Vec3d m_currentPos = Vec3d::Zero();
    Quatd m_currentOrientation     = Quatd::Identity();
    Vec3d m_currentVelocity        = Vec3d::Zero();
    Vec3d m_currentAngularVelocity = Vec3d::Zero();

    Vec3d m_currentDisplacement = Vec3d::Zero();
    Quatd m_currentRotation     = Quatd::Identity();

    /// If true, will use current and previous positions to produce velocity, if off, will ask device for velocity
    bool m_computeVelocity = false;
    /// If true, will use current and previous rotations to produce angular velocity, if off, will ask device for angular velocity
    bool m_computeAngularVelocity = false;
};
} // namespace imstk