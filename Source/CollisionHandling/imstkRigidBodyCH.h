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

#include <vector>

namespace imstk
{
struct CollisionData;
namespace expiremental
{
class RigidObject2;


///
/// \class RigidBodyContactHandler
///
/// \brief Creates rigid body contact and frictional constraints given
/// collision data then adds them to the rigid body model, if rigid
/// objects use differing models, one-way contacts are added to both.
/// If only one rigid object is given, one-ways are used.
///
class RigidBodyCH : public CollisionHandling
{
public:
    RigidBodyCH(const Side& side,
        const std::shared_ptr<CollisionData> colData,
        std::shared_ptr<RigidObject2>        rbdObjectA,
        std::shared_ptr<RigidObject2>        rbdObjectB = nullptr,
        const double                         stiffness = 0.0,
        const double                         frictionalCoefficient = 0.8);

    RigidBodyCH() = delete;

    virtual ~RigidBodyCH() override = default;

public:
    ///
    /// \brief Compute forces based on collision data
    ///
    void processCollisionData() override;

    void setStiffness(double stiffness) { m_stiffness = stiffness; }
    void setFriction(double frictionalCoefficient) { m_frictionalCoefficient = frictionalCoefficient; }
    void setUseFriction(bool useFriction) { m_useFriction = useFriction; }

protected:
    void processA();
    void processB();
    void processAB();

private:
    std::shared_ptr<RigidObject2> m_rbdObjectA = nullptr;
    std::shared_ptr<RigidObject2> m_rbdObjectB = nullptr;
    double m_stiffness = 0.0;
    double m_frictionalCoefficient = 0.8;
    bool   m_useFriction = true;
};
}
}
