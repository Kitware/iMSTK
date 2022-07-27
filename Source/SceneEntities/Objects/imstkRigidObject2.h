/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkDynamicObject.h"
#include "imstkMacros.h"

namespace imstk
{
class PointSet;
class RigidBodyModel2;
struct RigidBody;

///
/// \class RigidObject2
///
/// \brief Scene objects that are governed by rigid body dynamics under
/// the RigidBodyModel2
///
class RigidObject2 : public DynamicObject
{
public:
    RigidObject2(const std::string& name = "RigidObject2") : DynamicObject(name) { }
    ~RigidObject2() override = default;

    IMSTK_TYPE_NAME(RigidObject2)

    ///
    /// \brief Initialize the rigid scene object
    ///
    bool initialize() override;

    ///
    /// \brief Add local force at a position relative to object
    ///
    //void addForce(const Vec3d& force, const Vec3d& pos, bool wakeup = true);

    std::shared_ptr<RigidBodyModel2> getRigidBodyModel2();

    ///
    /// \brief Returns body in the model
    ///
    std::shared_ptr<RigidBody> getRigidBody() const { return m_rigidBody; }

    ///
    /// \brief Sets the model, and creates the body within the model
    ///
    void setDynamicalModel(std::shared_ptr<AbstractDynamicalModel> dynaModel) override;

    ///
    /// \brief Updates the physics geometry of the object
    ///
    void updatePhysicsGeometry() override;

protected:
    std::shared_ptr<RigidBodyModel2> m_rigidBodyModel2;
    std::shared_ptr<RigidBody>       m_rigidBody; ///< Gives the actual body within the model
};
} // namespace imstk