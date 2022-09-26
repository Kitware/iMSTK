/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkSceneObjectController.h"

namespace imstk
{
class RigidObject2;

///
/// \class RigidObjectController
///
/// \brief This class uses the provided device to control the provided rigid object via
/// virtual coupling. That is, it applies forces+torques to the rigid body that will
/// help move it to desired position/orientation.
/// It has linear and angular spring scales has well as dampening
/// You may also use force smoothening for the force applied back on the device
/// \todo: Force smoothening currently incurs loss
///
class RigidObjectController : public SceneObjectController
{
public:
    RigidObjectController(const std::string& name = "RigidObjectController") : SceneObjectController(name) { }
    ~RigidObjectController() override = default;

    void setControlledObject(std::shared_ptr<SceneObject> obj) override;

    ///
    /// \brief Set/Get the linear damping coefficient. Default 10000.0
    ///
    double getLinearKd() const { return m_linearKd; }
    void setLinearKd(const double kd) { m_linearKd = kd; }
    ///@}

    ///
    /// \brief Set/Get the angular damping coefficient. Default 10000.0
    ///@{
    double getAngularKd() const { return m_angularKd; }
    void setAngularKd(const double kd) { m_angularKd = kd; }
    ///@}

    ///
    /// \brief Set/Get the linear spring coefficient. Default (8000000.0, 8000000.0, 8000000.0)
    ///@{
    const Vec3d& getLinearKs() const { return m_linearKs; }
    void setLinearKs(const Vec3d& ks) { m_linearKs = ks; }
    void setLinearKs(const double ks) { m_linearKs = Vec3d(ks, ks, ks); }
    ///@}

    ///
    /// \brief Set/Get the rotationl spring coefficient. Default (10000.0, 10000.0, 10000.0)
    ///@{
    const Vec3d& getAngularKs() const { return m_angularKs; }
    void setAngularKs(const Vec3d& ks) { m_angularKs = ks; }
    void setAngularKs(const double ks) { m_angularKs = Vec3d(ks, ks, ks); }
    ///@}

    ///
    /// \brief Set/Get the scaling of the force on the device, set to 0 for no force
    ///@{
    double getForceScaling() const { return m_forceScaling; }
    void setForceScaling(const double forceScaling) { m_forceScaling = forceScaling; }
    ///@}

    ///
    /// \brief Set/Get whether to use spring or not
    ///@{
    bool getUseSpring() const { return m_useSpring; }
    void setUseSpring(const bool useSpring) { m_useSpring = useSpring; }
    ///@}

    ///
    /// \brief Set/Get whether to use force smoothening
    /// Force smoothening averages the force used on the device over kernel size
    ///@{
    bool getUseForceSmoothening() const { return m_forceSmoothening; }
    void setUseForceSmoothening(const bool useForceSmoothening) { m_forceSmoothening = useForceSmoothening; }
    ///@}

    ///
    /// \brief Set/Get whether to use critical damping (default on)
    /// Critical damping automatically computes linear & angular kd values. It may be turned
    /// off as it is sometimes useful to overdamp depending on other factors.
    ///@{
    bool getUseCritDamping() const { return m_useCriticalDamping; }
    void setUseCritDamping(const bool useCritDamping) { m_useCriticalDamping = useCritDamping; }
    ///@}

    ///
    /// \brief Set/Get the kernel size
    /// \todo: Vary with dt as performance of program will effect the size/number of samples
    ///@{
    int getSmoothingKernelSize() const { return m_smoothingKernelSize; }
    void setSmoothingKernelSize(const int kernelSize) { m_smoothingKernelSize = kernelSize; }
    ///@}

    ///
    /// \brief Return the device applied force (scaled)
    ///
    Vec3d getDeviceForce() const { return (m_fS + m_fD) * m_forceScaling; }

    ///
    /// \brief Returns spring force, without damper
    ///
    Vec3d getSpringForce() const { return m_fS; }

    ///
    /// \brief Returns damper force
    ///
    Vec3d getDamperForce() const { return m_fD; }

    ///
    /// \brief Return the device applied torque (scaled)
    ///
    Vec3d getDeviceTorque() const { return m_tS + m_tD; }

    ///
    /// \brief Returns spring torque, without damper
    ///
    Vec3d getSpringTorque() const { return m_tS; }

    ///
    /// \brief Returns damper torque
    ///
    Vec3d getDamperTorque() const { return m_tD; }

public:
    ///
    /// \brief Update controlled scene object using latest tracking information
    ///
    void update(const double& dt) override;

    ///
    /// \brief Apply forces to the haptic device
    ///
    void applyForces() override;

protected:
    std::shared_ptr<RigidObject2> m_rigidObject;

    double m_linearKd  = 10000.0;                                ///< Damping coefficient, linear
    double m_angularKd = 300.0;                                  ///< Damping coefficient, rotational
    Vec3d  m_linearKs  = Vec3d(8000000.0, 8000000.0, 8000000.0); ///< Spring coefficient, linear
    Vec3d  m_angularKs = Vec3d(10000.0, 10000.0, 10000.0);       ///< Spring coefficient, rotational

    // Linear spring force and damper force
    Vec3d m_fS = Vec3d::Zero();
    Vec3d m_fD = Vec3d::Zero();

    // Angular spring force and damper force
    Vec3d m_tS = Vec3d::Zero();
    Vec3d m_tD = Vec3d::Zero();

    double m_forceScaling       = 0.0000075;
    bool   m_useSpring          = true; ///< If off, pos & orientation directly set
    bool   m_useCriticalDamping = true; ///< If on, kd is automatically computed

    bool m_forceSmoothening    = true;
    int  m_smoothingKernelSize = 15;
    std::deque<Vec3d> m_forces;
    Vec3d m_forceSum = Vec3d::Zero();
};
} // namespace imstk