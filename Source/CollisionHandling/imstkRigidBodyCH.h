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
class CollisionData;
class RigidObject2;

///
/// \class RigidBodyCH
///
/// \brief Creates rigid body contact and frictional constraints given
/// collision data then adds them to the rigid body model, if rigid
/// objects use differing models, one-way contacts are added to both.
/// If only one rigid object is given, one-ways are used.
///
class RigidBodyCH : public CollisionHandling
{
public:
    RigidBodyCH() = default;
    virtual ~RigidBodyCH() override = default;

    virtual const std::string getTypeName() const override { return "RigidBodyCH"; }

public:
    void setInputRigidObjectA(std::shared_ptr<RigidObject2> rbdObjA);

    ///
    /// \brief Second input rigid object is optional
    ///
    void setInputRigidObjectB(std::shared_ptr<RigidObject2> rbdObjB);

    ///
    /// \brief Second input colliding object is optional
    ///
    void setInputCollidingObjectB(std::shared_ptr<CollidingObject> colObjB);

    std::shared_ptr<RigidObject2> getRigidObjA();
    std::shared_ptr<RigidObject2> getRigidObjB();

public:
    ///
    /// \brief Baumgarte stabilization term, scales the constraint scalars
    /// The higher it is, the faster constraint violations are dealt with
    /// (bigger steps), but could introduce erroneous velocity
    ///
    void setBeta(double beta) { m_beta = beta; }
    const double getBeta() const { return m_beta; }

    void setFriction(double frictionalCoefficient)
    {
        m_frictionalCoefficient = frictionalCoefficient;
        m_useFriction = (m_frictionalCoefficient != 0.0);
    }

    const double getFriction() const { return m_frictionalCoefficient; }

    void setUseFriction(bool useFriction) { m_useFriction = useFriction; }
    const double getUseFriction() const { return m_useFriction; }

protected:
    ///
    /// \brief Add rigid body constraints according to contacts
    ///
    virtual void handle(
        const std::vector<CollisionElement>& elementsA,
        const std::vector<CollisionElement>& elementsB) override;

    ///
    /// \brief Handle rigid vs rigid two-way
    /// edge-edge, vertex-triangle, etc
    ///
    void handleRbdRbdTwoWay(
        std::shared_ptr<RigidObject2>        rbdObjA,
        std::shared_ptr<RigidObject2>        rbdObjB,
        const std::vector<CollisionElement>& elementsA,
        const std::vector<CollisionElement>& elementsB);

    ///
    /// \brief Handle rigid vs static one-way
    /// edge-edge, vertex-triangle, etc
    ///
    void handleRbdStaticOneWay(
        std::shared_ptr<RigidObject2>        rbdObj,
        std::shared_ptr<CollidingObject>     colObj,
        const std::vector<CollisionElement>& elementsA,
        const std::vector<CollisionElement>& elementsB);

    ///
    /// \brief Add constraint for the rigid body given contact
    ///
    virtual void addConstraint(
        std::shared_ptr<RigidObject2> rbdObj,
        const Vec3d& contactPt, const Vec3d& contactNormal,
        const double contactDepth);

    ///
    /// \brief Add two-way constraint for the rigid bodies given contact
    ///
    virtual void addConstraint(
        std::shared_ptr<RigidObject2> rbdObjA,
        std::shared_ptr<RigidObject2> rbdObjB,
        const Vec3d& contactPt, const Vec3d& contactNormal,
        const double contactDepth);

protected:
    double m_beta = 0.0;
    double m_frictionalCoefficient = 2.0;
    bool   m_useFriction = false;
};
} // namespace imstk