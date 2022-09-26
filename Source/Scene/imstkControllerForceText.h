/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkComponent.h"
#include "imstkMath.h"

namespace imstk
{
class PbdObjectCollision;
class PbdObjectController;
class RigidObjectController;
class TextVisualModel;

///
/// \class ControllerForceText
///
/// \brief Displays virtual coupling force text in the top right
///
class ControllerForceText : public SceneBehaviour
{
public:
    ControllerForceText(const std::string& name = "ControllerForceText");

    ///
    /// \brief Update the display of the last frames update times
    ///
    void visualUpdate(const double& dt);

    ///
    /// \brief Get/Set the controller to display the device force of
    /// @{
    void setController(std::shared_ptr<PbdObjectController> controller) { m_pbdController = controller; }
    std::shared_ptr<PbdObjectController> getPbdController() const { return m_pbdController; }
    void setController(std::shared_ptr<RigidObjectController> controller) { m_rbdController = controller; }
    std::shared_ptr<RigidObjectController> getRbdController() const { return m_rbdController; }
    /// @}

    ///
    /// \brief Get/Set the collision to display the contact force of
    /// @{
    void setCollision(std::shared_ptr<PbdObjectCollision> collision) { m_collision = collision; }
    std::shared_ptr<PbdObjectCollision> getCollision() const { return m_collision; }
    /// @}

    std::shared_ptr<TextVisualModel> getText() const { return m_textVisualModel; }

protected:
    void computePbdContactForceAndTorque(Vec3d& force, Vec3d& torque) const;

    void init() override;

protected:
    std::shared_ptr<TextVisualModel>       m_textVisualModel = nullptr;
    std::shared_ptr<PbdObjectCollision>    m_collision       = nullptr;
    std::shared_ptr<PbdObjectController>   m_pbdController   = nullptr;
    std::shared_ptr<RigidObjectController> m_rbdController   = nullptr;
    double m_t = 0.0;
};
} // namespace imstk