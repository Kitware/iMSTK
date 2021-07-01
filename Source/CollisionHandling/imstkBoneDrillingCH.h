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
#include "imstkMath.h"

namespace imstk
{
class CollisionData;
class CollidingObject;
class RigidObject2;

///
/// \class BoneDrillingCH
///
/// \brief Implements bone drilling collision handling
/// \todo: Doesn't work right now. Tet removal needs to be fixed. I would suggest
/// buffering setting removed elements to a dummy vertex.
///
class BoneDrillingCH : public CollisionHandling
{
public:
    BoneDrillingCH() = default;
    virtual ~BoneDrillingCH() override = default;

    virtual const std::string getTypeName() const override { return "BoneDrillingCH"; }

public:
    ///
    /// \brief Set the input bone
    ///
    void setInputObjectBone(std::shared_ptr<CollidingObject> boneObject) { setInputObjectA(boneObject); }

    ///
    /// \brief Set the input drill
    ///
    void setInputObjectDrill(std::shared_ptr<RigidObject2> drillObject);

    std::shared_ptr<CollidingObject> getBoneObj() const { return getInputObjectA(); }
    std::shared_ptr<RigidObject2> getDrillObj() const;

public:
    ///
    /// \brief Get stiffness
    ///
    double getStiffness() const { return m_stiffness; }
    ///
    /// \brief Set stiffness
    ///
    void setStiffness(const double k) { m_stiffness = k; }

    ///
    /// \brief Get damping coefficient
    ///
    double getDamping() const { return m_damping; }
    ///
    /// \brief Set damping coefficient
    ///
    void setDamping(const double d) { m_damping = d; }

protected:
    ///
    /// \brief Does the bone drilling
    ///
    virtual void handle(
        const CDElementVector<CollisionElement>& elementsA,
        const CDElementVector<CollisionElement>& elementsB) override;

    ///
    /// \brief Decrease the density at the nodal points and remove if the density goes below 0
    ///
    void erodeBone(
        const CDElementVector<CollisionElement>& elementsA,
        const CDElementVector<CollisionElement>& elementsB);

private:
    double m_stiffness = 10e-01;                ///> Stiffness coefficient associated with virtual coupling object
    double m_damping   = 0.005;                 ///> Damping coefficient associated with virtual coupling object

    double m_angularSpeed = 10 * PI;            ///> Angular speed of the drill (rad per sec)
    double m_BoneHardness = 10;                 ///> Angular speed of the drill (rad per sec)

    std::vector<double> m_nodalDensity;         ///> Density of the bone
    double m_initialBoneDensity = 1.0;          ///> Density of the bone before the start of the drilling process

//    std::vector<size_t> m_erodedNodes; /// \todo Unused variable
    std::vector<bool> m_nodeRemovalStatus;               ///> Keeps track of the removal status of the node
    std::vector<std::vector<size_t>> m_nodalCardinalSet; ///> Keeps track of the removal status of the node

    bool  m_initialStep = true;                          ///> Number of times steps
    Vec3d m_prevPos;                                     ///> Previous position of the colliding object
};
}