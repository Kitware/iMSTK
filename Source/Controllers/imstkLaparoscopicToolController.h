/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkTrackingDeviceControl.h"

namespace imstk
{
class Entity;
class Geometry;
class SceneObject;

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
    enum class JawState
    {
        Closed,
        Opened
    };

public:
    LaparoscopicToolController(const std::string& name = "LaparoscopicToolController") : TrackingDeviceControl(name) { }
    ~LaparoscopicToolController() override = default;

    void setParts(
        std::shared_ptr<Entity> shaft,
        std::shared_ptr<Entity> upperJaw,
        std::shared_ptr<Entity> lowerJaw,
        std::shared_ptr<Geometry>        pickGeom);

    void setDevice(std::shared_ptr<DeviceClient> device) override;

    // *INDENT-OFF*
    ///
    /// \brief Fired once when the jaw transitions to closed state
    /// 
    SIGNAL(LaparoscopicToolController, JawClosed);

    ///
    /// \brief Fired once when the jaw transitions to open state
    /// 
    SIGNAL(LaparoscopicToolController, JawOpened);
    // *INDENT-ON*

public:
    ///
    /// \brief Update controlled laparoscopic tool using latest tracking information
    ///
    void update(const double& dt) override;

    ///
    /// \brief Set the maximum jaw angle
    ///
    void setMaxJawAngle(const double maxAngle) { m_maxJawAngle = maxAngle; }

    ///
    /// \brief Set the increment
    ///
    void setJawAngleChange(const double dAngle) { m_change = dAngle; }

    ///
    /// \brief Set the jaw rotation axis
    ///
    void setJawRotationAxis(const Vec3d& axis) { m_jawRotationAxis = axis; }

    ///
    /// \brief Get the current jaw angle
    ///
    double getJawAngle() const { return m_jawAngle; }

    ///
    /// \brief Get the max jaw angle
    ///
    double getMaxJawAngle() const { return m_maxJawAngle; }

    ///
    /// \brief Get the state of the jaw
    /// whether it is open or closed
    ///
    JawState getJawState() const { return m_jawState; }

protected:
    std::shared_ptr<SceneObject> m_shaft;               ///< Tool shaft
    std::shared_ptr<SceneObject> m_upperJaw;            ///< Tool upper jaw
    std::shared_ptr<SceneObject> m_lowerJaw;            ///< Tool lower jaw
    std::shared_ptr<Geometry> m_pickGeom;

    double   m_jawAngle    = PI / 6.0;                      ///< Angle of the jaws
    double   m_change      = 6.0e-5;                        ///< Amount of change in jaw angle per frame
    double   m_maxJawAngle = PI / 6.0;                      ///< Maximum angle of the jaws
    JawState m_jawState    = JawState::Opened;

    Vec3d m_jawRotationAxis = Vec3d(1.0, 0.0, 0.0);         ///< Angle of the jaws

    Mat4d m_controllerWorldTransform = Mat4d::Identity();   // Final world transform of the controller
    Mat4d m_pickGeomTransform = Mat4d::Identity();

    Mat4d m_shaftVisualTransform    = Mat4d::Identity();    // Initial local transform of the visual shaft
    Mat4d m_upperJawVisualTransform = Mat4d::Identity();    // Initial local transform of the visual upper jaw
    Mat4d m_lowerJawVisualTransform = Mat4d::Identity();    // Initial local transform of the visual lower jaw

    Mat4d m_shaftCollidingTransform    = Mat4d::Identity(); // Initial local transform of the colliding shaft
    Mat4d m_upperJawCollidingTransform = Mat4d::Identity(); // Initial local transform of the colliding upper jaw
    Mat4d m_lowerJawCollidingTransform = Mat4d::Identity(); // Initial local transform of the colliding lower jaw

    Mat4d m_upperJawLocalTransform = Mat4d::Identity();     // upperJawWorldTransform = m_controllerWorldTransform * m_upperJawLocalTransform * m_upperJawVisual/CollidingTransform
    Mat4d m_lowerJawLocalTransform = Mat4d::Identity();     // lowerJawWorldTransform = m_controllerWorldTransform * m_lowerJawLocalTransform * m_lowerJawVisual/CollidingTransform
};
} // namespace imstk