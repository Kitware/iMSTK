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

#ifndef imstkVirtualCouplingCH_h
#define imstkVirtualCouplingCH_h

// imstk
#include "imstkCollisionHandling.h"

namespace imstk
{
class CollidingObject;
class CollisionData;

///
/// \class VirtualCouplingCH
///
/// \brief Implements virtual coupling collision handling
///
class VirtualCouplingCH : public CollisionHandling
{
public:

    ///
    /// \brief Constructor
    ///
    VirtualCouplingCH(const Side& side,
                      const CollisionData& colData,
                      std::shared_ptr<CollidingObject> obj) :
        CollisionHandling(Type::VirtualCoupling, side, colData),
        m_object(obj) {}

    VirtualCouplingCH() = delete;

    ///
    /// \brief Destructor
    ///
    ~VirtualCouplingCH() = default;

    ///
    /// \brief Compute forces based on collision data
    ///
    void computeContactForces() override;

    ///
    /// \brief Get/Set Stiffness coefficient
    ///
    inline double getStiffness() const { return m_stiffness; }
    inline void setStiffness(const double stiffness) { m_stiffness = stiffness; }

    ///
    /// \brief Get/Set Damping coefficient
    ///
    inline double getDamping() const { return m_damping; }
    inline void setDamping(const double damping) { m_damping = damping; }

    ///
    /// \brief Get the offset of the
    ///
    Vec3d getOffset();

private:

    std::shared_ptr<CollidingObject> m_object;  ///> colliding object
    bool m_initialStep = true;                  ///> Number of times steps
    Vec3d m_prevPos;                            ///> Previous position of the colliding object
    Vec3d m_offset;

    double m_stiffness = 5e-01;                 ///> Stiffness coefficient use to compute spring force
    double m_damping = 0.005;                   ///> Damping coefficient used to compute damping force
};
}

#endif // ifndef imstkVirtualCouplingCH_h