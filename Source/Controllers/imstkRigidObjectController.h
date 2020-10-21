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

#include "imstkSceneObjectController.h"

namespace imstk
{
namespace expiremental
{
class RigidObject2;

///
/// \class RigidObjectController
///
/// \brief This class uses the provide device to control the provided rigid object via
/// virtual coupling. That is, it applies forces+torques to the rigid body that will
/// help move it to desired position/orientation
///
class RigidObjectController : public SceneObjectController
{
public:
    ///
    /// \brief Constructor
    ///
    RigidObjectController(std::shared_ptr<RigidObject2> rigidObject, std::shared_ptr<DeviceClient> trackingDevice);
    RigidObjectController() = delete;

    ///
    /// \brief Destructor
    ///
    virtual ~RigidObjectController() override = default;

public:
    ///
    /// \brief Set/Get the linear damping coefficient
    ///
    double getLinearKd() const { return m_linearKd; }
    void setLinearKd(double kd) { m_linearKd = kd; }

    ///
    /// \brief Set/Get the linear spring coefficient
    ///
    const Vec3d& getLinearKs() const { return m_linearKs; }
    void setLinearKs(const Vec3d& ks) { m_linearKs = ks; }

    ///
    /// \brief Set/Get the rotationl spring coefficient
    ///
    const Vec3d& getRotKs() const { return m_rotKs; }
    void setRotationalKs(const Vec3d& ks) { m_rotKs = ks; }

public:
    ///
    /// \brief Update controlled scene object using latest tracking information
    ///
    void updateControlledObjects() override;

    ///
    /// \brief Apply forces to the haptic device
    ///
    void applyForces() override;

protected:
    std::shared_ptr<RigidObject2> m_rigidObject;

    double m_linearKd = 800.0;                                  ///> Damping coefficient, linear
    Vec3d  m_linearKs = Vec3d(2000000.0, 2000000.0, 2000000.0); ///> Spring coefficient, linear
    Vec3d  m_rotKs    = Vec3d(1000.0, 1000.0, 1000.0);          //.> Spring coefficient, rotational
};
}
}
