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

#include "imstkRigidObject2.h"
#include "imstkMacros.h"

using namespace imstk;

class NeedleObject : public RigidObject2
{
public:
    enum class CollisionState
    {
        REMOVED,
        TOUCHING,
        INSERTED
    };

public:
    NeedleObject(const std::string& name) : RigidObject2(name) { }
    virtual ~NeedleObject() = default;

    IMSTK_TYPE_NAME(NeedleObject)

public:
    void setCollisionState(const CollisionState state) { m_collisionState = state; }
    CollisionState getCollisionState() const { return m_collisionState; }

    ///
    /// \brief Set the force threshold for the needle
    ///
    void setForceThreshold(const double forceThreshold) { m_forceThreshold = forceThreshold; }
    double getForceThreshold() const { return m_forceThreshold; }

    ///
    /// \brief Returns the current axes of the needle (tip-tail)
    ///
    const Vec3d getAxes() const
    {
        return (-getCollidingGeometry()->getRotation().col(2)).normalized();
    }

protected:
    CollisionState m_collisionState = CollisionState::REMOVED;
    double m_forceThreshold = 10.0;
};