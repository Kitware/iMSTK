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

#ifndef imstkRigidBodyState_h
#define imstkRigidBodyState_h

#include <Eigen/Dense>
#include <memory>
#include <vector>

#include "imstkMath.h"

namespace imstk
{
///
/// \class RigidBodyState
///
/// \brief Kinematic state of a Rigid Body
///
class RigidBodyState
{
public:
    ///
    /// \brief Default constructor/destructor
    ///
    RigidBodyState()  = default;
    ~RigidBodyState() = default;

    ///
    /// \brief Initialize the rigid body state
    ///
    void initialize(const Vec3d position, const RigidTransform3d rotationMat);
    void initialize() {}

    ///
    /// \brief Get/Set object position
    ///
    Vec3d& getPosition() { return m_position; };
    void setPosition(const Vec3d& p) { m_position = p; };

    ///
    /// \brief Set the state to a given one
    ///
    void setState(std::shared_ptr<RigidBodyState> state)
    {
        m_orientation = state->getRotation();
        m_position    = state->getPosition();
    }

    ///
    /// \brief Get/Set object rotation matrix
    ///
    RigidTransform3d& getRotation() { return m_orientation; };
    void setRotation(const RigidTransform3d& r) { m_orientation = r; };

private:
    RigidTransform3d m_orientation = RigidTransform3d::Identity(); ///> Rotation
    Vec3d m_position = Vec3d::Zero();                              ///> position

    //Can add linear velocity and angular velocity too.
};
} // imstk

#endif // imstkPbdState_h