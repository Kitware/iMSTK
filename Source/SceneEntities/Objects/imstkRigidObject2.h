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

#include "imstkDynamicObject.h"

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
    RigidObject2(const std::string& name) : DynamicObject(name) { }

    virtual ~RigidObject2() = default;

    virtual const std::string getTypeName() const override { return "RigidObject2"; }

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
    std::shared_ptr<RigidBody>       m_rigidBody; ///> Gives the actual body within the model
};
} // imstk