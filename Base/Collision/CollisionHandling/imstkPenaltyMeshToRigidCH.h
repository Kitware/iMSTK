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

#ifndef imstkPenaltyMeshToRigidCH_h
#define imstkPenaltyMeshToRigidCH_h

#include "imstkCollisionHandling.h"

#include <memory>

namespace imstk {

class CollidingObject;
class CollisionData;

class PenaltyMeshToRigidCH : public CollisionHandling
{
public:

    ///
    /// \brief Constructor
    ///
    PenaltyMeshToRigidCH(const Side& side, const CollisionData& colData, std::shared_ptr<CollidingObject> obj) :
        CollisionHandling(CollisionHandling::Type::Penalty, side, colData), m_object(obj)
    {
        m_stiffness = 1000;
        m_damping = 0.5;
    }

    ///
    /// \brief Destructor
    ///
    ~PenaltyMeshToRigidCH() = default;

    ///
    /// \brief Compute forces based on collision data
    ///
    void computeContactForces() override;

private:
    std::shared_ptr<CollidingObject> m_object; ///> The mesh object under collision

    double m_stiffness; ///> Stiffness of contact
    double m_damping; ///> Damping of the contact
};
}

#endif // ifndef imstkPenaltyMeshToRigidCH_h
