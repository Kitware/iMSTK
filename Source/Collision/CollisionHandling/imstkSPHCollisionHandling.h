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
#include "imstkCollisionHandling.h"
#include "imstkSPHObject.h"

namespace imstk
{
template<class Real>
class SPHCollisionHandling : public CollisionHandling
{
public:
    SPHCollisionHandling(const Side& side, const CollisionData& colData, const std::shared_ptr<CollidingObject>& obj) :
        CollisionHandling(Type::SPH, side, colData), m_SPHObject(std::dynamic_pointer_cast<SPHObject<Real>>(obj)) {}

    SPHCollisionHandling() = delete;
    virtual ~SPHCollisionHandling() = default;

    ///
    /// \brief Compute forces based on collision data
    ///
    virtual void computeContactForces() override;

    ///
    /// \brief Set the friction coefficient
    ///
    void setBoundaryFriction(Real friction);

    ///
    /// \brief Get the friction coefficient
    ///
    Real getBoundaryFriction() const { return m_BoundaryFriction; }

private:
    std::shared_ptr<SPHObject<Real>> m_SPHObject;
    Real m_BoundaryFriction = 0.1;
};
} // end namespace imstk