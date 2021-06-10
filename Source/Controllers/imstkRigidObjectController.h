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
    void setControlledSceneObject(std::shared_ptr<SceneObject> obj) override;

    ///
    /// \brief Set/Get the linear damping coefficient. Default 10000.0
    ///
    double getLinearKd() const { return m_linearKd; }
    void setLinearKd(double kd) { m_linearKd = kd; }

    ///
    /// \brief Set/Get the angular damping coefficient. Default 10000.0
    ///
    double getAngularKd() const { return m_angularKd; }
    void setAngularKd(double kd) { m_angularKd = kd; }

    ///
    /// \brief Set/Get the linear spring coefficient. Default (8000000.0, 8000000.0, 8000000.0)
    ///
    const Vec3d& getLinearKs() const { return m_linearKs; }
    void setLinearKs(const Vec3d& ks) { m_linearKs = ks; }
    void setLinearKs(const double ks) { m_linearKs = Vec3d(ks, ks, ks); }

    ///
    /// \brief Set/Get the rotationl spring coefficient. Default (10000.0, 10000.0, 10000.0)
    ///
    const Vec3d& getAngularKs() const { return m_angularKs; }
    void setAngularKs(const Vec3d& ks) { m_angularKs = ks; }
    void setAngularKs(const double ks) { m_angularKs = Vec3d(ks, ks, ks); }

    ///
    /// \brief Set/Get the scaling of the force on the device, set to 0 for no force
    ///
    double getForceScaling() const { return m_forceScaling; }
    void setForceScaling(const double forceScaling) { m_forceScaling = forceScaling; }

    ///
    /// \brief Set/Get whether to use spring or not
    ///
    bool getUseSpring() const { return m_useSpring; }
    void setUseSpring(const bool useSpring) { m_useSpring = useSpring; }

    ///
    /// \brief Set/Get whether to use force smoothening
    /// Force smoothening averages the force used on the device over kernel size
    ///
    bool getUseForceSmoothening() const { return m_forceSmoothening; }
    void setUseForceSmoothening(const bool useForceSmoothening) { m_forceSmoothening = useForceSmoothening; }

    ///
    /// \brief Set/Get the kernel size
    /// \todo: Vary with dt as performance of program will effect the size/number of samples
    ///
    int getSmoothingKernelSize() const { return m_smoothingKernelSize; }
    void setSmoothingKernelSize(const int kernelSize) { m_smoothingKernelSize = kernelSize; }

    ///
    /// \brief Return the currently applied force
    ///
    const Vec3d getForce() const { return fS * m_forceScaling; }

    ///
    /// \brief Return the currently applied torque
    ///
    const Vec3d getTorque() const { return tS; }

public:
    ///
    /// \brief Update controlled scene object using latest tracking information
    ///
    void update(const double dt) override;

    ///
    /// \brief Apply forces to the haptic device
    ///
    void applyForces() override;

protected:
    std::shared_ptr<RigidObject2> m_rigidObject;

    double m_linearKd  = 10000.0;                                ///> Damping coefficient, linear
    double m_angularKd = 300.0;                                  ///> Damping coefficient, rotational
    Vec3d  m_linearKs  = Vec3d(8000000.0, 8000000.0, 8000000.0); ///> Spring coefficient, linear
    Vec3d  m_angularKs = Vec3d(10000.0, 10000.0, 10000.0);       ///> Spring coefficient, rotational

    Vec3d fS = Vec3d(0.0, 0.0, 0.0);
    Vec3d tS = Vec3d(0.0, 0.0, 0.0);

    double m_forceScaling = 0.0000075;
    bool   m_useSpring    = true; ///> Controller has ability to toggle to from springs

    bool m_forceSmoothening    = true;
    int  m_smoothingKernelSize = 25;
    std::deque<Vec3d> m_forces;
    Vec3d m_forceSum = Vec3d::Zero();
};
}