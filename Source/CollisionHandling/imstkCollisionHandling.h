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

#include <memory>

namespace imstk
{
struct CollisionData;

class CollidingObject;
class InteractionPair;
class TaskNode;

///
/// \class CollisionHandling
///
/// \brief Base class for all collision handling classes
///
class CollisionHandling
{
public:
    ///
    /// \brief Type of the collision handling
    ///
    enum class Type
    {
        None,
        Penalty,
        VirtualCoupling,
        NodalPicking,
        BoneDrilling,
        SPH,
        PBD,
        RBD,
        LevelSet
    };

    ///
    /// \brief Direction of the collision handling
    ///
    enum class Side
    {
        A,
        B,
        AB
    };

public:
    CollisionHandling(const Type& type, const Side& side, const std::shared_ptr<CollisionData> colData);
    CollisionHandling() = delete;

    virtual ~CollisionHandling() = default;

public:
    ///
    /// \brief Compute forces based on collision data (pure virtual)
    ///
    virtual void processCollisionData() = 0;

    ///
    /// \brief Returns collision handling type
    ///
    const Type& getType() const { return m_type; }
    const Side& getSide() const { return m_side; }

    std::shared_ptr<TaskNode> getTaskNode() const { return m_taskNode; }

protected:
    Type m_type;                                              ///< Collision handling algorithm type
    Side m_side;                                              ///< Direction of the collisionData
    const std::shared_ptr<CollisionData> m_colData = nullptr; ///< Collision data
    std::shared_ptr<TaskNode> m_taskNode = nullptr;
};
}
