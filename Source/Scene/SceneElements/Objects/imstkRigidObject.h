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

// imstk
#include "imstkDynamicObject.h"
#include "imstkRigidBodyModel.h"

namespace imstk
{
///
/// \class RigidObject
///
/// \brief Scene objects that are governed by rigid body dynamics
///
class RigidObject : public DynamicObject<RigidBodyState>
{
public:

    ///
    /// \brief Constructor
    ///
    RigidObject(std::string name) : DynamicObject(name)
    {
        m_type = Type::Rigid;
    }

    ///
    /// \brief Destructor
    ///
    ~RigidObject() = default;

    ///
    /// \brief Initialize the rigid scene object
    ///
    bool initialize() override;

    ///
    /// \brief Add local force at a position relative to object
    ///
    void addForce(const Vec3d& force, const Vec3d& pos, bool wakeup = true)
    {
        getRigidBodyModel()->addForce(force, pos, wakeup);
    }

    ///
    /// \brief Get/Set rigid body model
    ///
    void setRigidBodyModel(std::shared_ptr<RigidBodyModel> rbModel) { m_dynamicalModel = rbModel; };
    std::shared_ptr<RigidBodyModel> getRigidBodyModel() const { return std::dynamic_pointer_cast<RigidBodyModel>(m_dynamicalModel); };

    void reset() override
    {
        m_dynamicalModel->resetToInitialState();
        this->updateGeometries();
    }

protected:
    //std::shared_ptr<RigidBodyModel> m_rigidBodyModel; ///> PBD mathematical model
};
} // imstk