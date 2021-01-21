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

#include "imstkTrackingDeviceControl.h"

namespace imstk
{
class CollidingObject;

///
/// \class LaparoscopicToolController
///
/// \brief Two-jawed laparoscopic tool controlled by external device
/// The tool is composed of three scene objects: pivot, lower jaw and upper jaw
/// The jaws open-close based on the buttons at present.
/// This has to be replaced by potentiometer tracking in future.
///
class LaparoscopicToolController : public TrackingDeviceControl
{
public:
    ///
    /// \brief Constructor
    ///
    LaparoscopicToolController(
        std::shared_ptr<CollidingObject> shaft,
        std::shared_ptr<CollidingObject> upperJaw,
        std::shared_ptr<CollidingObject> lowerJaw,
        std::shared_ptr<DeviceClient>    trackingDevice);

    LaparoscopicToolController() = delete; //not allowed for now

    ~LaparoscopicToolController() override = default;

public:
    ///
    /// \brief Update controlled laparoscopic tool using latest tracking information
    ///
    void update(const double dt) override;

    ///
    /// \brief Apply forces to the haptic device
    ///
    void applyForces() override;

    ///
    /// \brief Set the maximum jaw angle
    ///
    inline void setMaxJawAngle(const double maxAngle) { m_maxJawAngle = maxAngle; }

    ///
    /// \brief Set the increment
    ///
    inline void setJawAngleChange(const double dAngle) { m_change = dAngle; }

    ///
    /// \brief Set the jaw rotation axis
    ///
    inline void setJawRotationAxis(const Vec3d& axis) { m_jawRotationAxis = axis; }

    ///
    /// \brief Get the current jaw angle
    ///
    inline double getJawAngle() const { return m_jawAngle; }

    ///
    /// \brief Get the max jaw angle
    ///
    inline double getMaxJawAngle() const { return m_maxJawAngle; }

protected:
    std::shared_ptr<CollidingObject> m_shaft;               ///< Tool shaft
    std::shared_ptr<CollidingObject> m_upperJaw;            ///< Tool upper jaw
    std::shared_ptr<CollidingObject> m_lowerJaw;            ///< Tool lower jaw

    double m_jawAngle    = PI / 6.0;                        ///< Angle of the jaws
    double m_change      = 6.0e-5;                          ///< Amount of change in jaw angle per frame
    double m_maxJawAngle = PI / 6.0;                        ///< Maximum angle of the jaws

    Vec3d m_jawRotationAxis;                                ///< Angle of the jaws

    Mat4d m_controllerWorldTransform = Mat4d::Identity();   // Final world transform of the controller

    Mat4d m_shaftVisualTransform    = Mat4d::Identity();    // Initial local transform of the visual shaft
    Mat4d m_upperJawVisualTransform = Mat4d::Identity();    // Initial local transform of the visual upper jaw
    Mat4d m_lowerJawVisualTransform = Mat4d::Identity();    // Initial local transform of the visual lower jaw

    Mat4d m_shaftCollidingTransform    = Mat4d::Identity(); // Initial local transform of the colliding shaft
    Mat4d m_upperJawCollidingTransform = Mat4d::Identity(); // Initial local transform of the colliding upper jaw
    Mat4d m_lowerJawCollidingTransform = Mat4d::Identity(); // Initial local transform of the colliding lower jaw

    Mat4d m_upperJawLocalTransform = Mat4d::Identity();     // upperJawWorldTransform = m_controllerWorldTransform * m_upperJawLocalTransform * m_upperJawVisual/CollidingTransform
    Mat4d m_lowerJawLocalTransform = Mat4d::Identity();     // lowerJawWorldTransform = m_controllerWorldTransform * m_lowerJawLocalTransform * m_lowerJawVisual/CollidingTransform
};
} // imstk
