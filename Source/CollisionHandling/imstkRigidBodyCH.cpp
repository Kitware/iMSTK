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

#include "imstkRigidBodyCH.h"
#include "imstkCollisionData.h"
#include "imstkRbdContactConstraint.h"
#include "imstkRbdFrictionConstraint.h"
#include "imstkRigidBodyModel2.h"
#include "imstkRigidObject2.h"

namespace imstk
{
namespace expiremental
{
RigidBodyCH::RigidBodyCH(const Side&                          side,
                         const std::shared_ptr<CollisionData> colData,
                         std::shared_ptr<RigidObject2>        rbdObjectA,
                         std::shared_ptr<RigidObject2>        rbdObjectB,
                         const double                         stiffness,
                         const double                         frictionalCoefficient) :
    CollisionHandling(Type::RBD, side, colData),
    m_rbdObjectA(rbdObjectA),
    m_rbdObjectB(rbdObjectB),
    m_stiffness(stiffness),
    m_frictionalCoefficient(frictionalCoefficient),
    m_useFriction(frictionalCoefficient != 0.0)
{
    if (side == Side::AB && rbdObjectA == nullptr && rbdObjectB == nullptr)
    {
        LOG(WARNING) << "RigidBodyCH, side AB was specified but only one object given";
        return;
    }
    if (side == Side::A && rbdObjectA == nullptr)
    {
        LOG(WARNING) << "RigidBodyCH, side A was specified but object A not given";
        return;
    }
    if (side == Side::B && rbdObjectB == nullptr)
    {
        LOG(WARNING) << "RigidBodyCH, side B was specified but object B not given";
        return;
    }
}

void
RigidBodyCH::processCollisionData()
{
    if (m_side == Side::A)
    {
        processA();
    }
    else if (m_side == Side::B)
    {
        processB();
    }
    else if (m_side == Side::AB)
    {
        std::shared_ptr<RigidBodyModel2> rbdModelA = m_rbdObjectA->getRigidBodyModel2();
        std::shared_ptr<RigidBodyModel2> rbdModelB = m_rbdObjectB->getRigidBodyModel2();

        // If the two bodies exist in separate models, add two one way constraints
        if (rbdModelA != rbdModelB)
        {
            if (rbdModelA != nullptr)
            {
                processA();
            }
            if (rbdModelB != nullptr)
            {
                processB();
            }
        }
        else
        {
            processAB();
        }
    }
}

void
RigidBodyCH::processA()
{
    std::shared_ptr<RigidBodyModel2> rbdModelA = m_rbdObjectA->getRigidBodyModel2();
    // Generate one-way constraints
    PositionDirectionCollisionData& pdColData = m_colData->PDColData;
    for (int i = 0; i < pdColData.getSize(); i++)
    {
        {
            std::shared_ptr<RbdContactConstraint> contactConstraint =
                std::make_shared<RbdContactConstraint>(
                    m_rbdObjectA->getRigidBody(), nullptr,
                    pdColData[i].dirAtoB.normalized(),
                    pdColData[i].posB,
                    //0.0001,
                    pdColData[i].penetrationDepth,
                    m_stiffness,
                    RbdConstraint::Side::A);
            contactConstraint->compute(rbdModelA->getTimeStep());
            rbdModelA->addConstraint(contactConstraint);
        }

        if (m_useFriction)
        {
            std::shared_ptr<RbdFrictionConstraint> frictionConstraint =
                std::make_shared<RbdFrictionConstraint>(
                    m_rbdObjectA->getRigidBody(), nullptr,
                    pdColData[i].posB,
                    pdColData[i].dirAtoB.normalized(),
                    pdColData[i].penetrationDepth,
                    m_frictionalCoefficient,
                    RbdConstraint::Side::A);
            frictionConstraint->compute(rbdModelA->getTimeStep());
            rbdModelA->addConstraint(frictionConstraint);
        }
    }
}

void
RigidBodyCH::processB()
{
    std::shared_ptr<RigidBodyModel2> rbdModelB = m_rbdObjectB->getRigidBodyModel2();
    // Generate one-way constraints
    PositionDirectionCollisionData& pdColData = m_colData->PDColData;
    for (int i = 0; i < pdColData.getSize(); i++)
    {
        std::shared_ptr<RbdContactConstraint> contactConstraint =
            std::make_shared<RbdContactConstraint>(
                nullptr, m_rbdObjectB->getRigidBody(),
                pdColData[i].dirAtoB.normalized(),
                pdColData[i].posB,
                pdColData[i].penetrationDepth,
                m_stiffness,
                RbdConstraint::Side::B);
        contactConstraint->compute(rbdModelB->getTimeStep());
        rbdModelB->addConstraint(contactConstraint);

        if (m_useFriction)
        {
            std::shared_ptr<RbdFrictionConstraint> frictionConstraint =
                std::make_shared<RbdFrictionConstraint>(
                    nullptr, m_rbdObjectB->getRigidBody(),
                    pdColData[i].posB,
                    pdColData[i].dirAtoB.normalized(),
                    pdColData[i].penetrationDepth,
                    m_frictionalCoefficient,
                    RbdConstraint::Side::B);
            frictionConstraint->compute(rbdModelB->getTimeStep());
            rbdModelB->addConstraint(frictionConstraint);
        }
    }
}

void
RigidBodyCH::processAB()
{
    // Generate one two-way constraint
    std::shared_ptr<RigidBodyModel2> rbdModelAB = m_rbdObjectA->getRigidBodyModel2();
    PositionDirectionCollisionData&  pdColData  = m_colData->PDColData;
    for (int i = 0; i < pdColData.getSize(); i++)
    {
        std::shared_ptr<RbdContactConstraint> contactConstraint =
            std::make_shared<RbdContactConstraint>(
                m_rbdObjectA->getRigidBody(), m_rbdObjectB->getRigidBody(),
                pdColData[i].dirAtoB.normalized(),
                pdColData[i].posB,
                pdColData[i].penetrationDepth,
                m_stiffness);
        contactConstraint->compute(rbdModelAB->getTimeStep());
        rbdModelAB->addConstraint(contactConstraint);

        if (m_useFriction)
        {
            std::shared_ptr<RbdFrictionConstraint> frictionConstraint =
                std::make_shared<RbdFrictionConstraint>(
                    m_rbdObjectA->getRigidBody(), m_rbdObjectB->getRigidBody(),
                    pdColData[i].posB,
                    pdColData[i].dirAtoB.normalized(),
                    pdColData[i].penetrationDepth,
                    m_frictionalCoefficient,
                    RbdConstraint::Side::AB);
            frictionConstraint->compute(rbdModelAB->getTimeStep());
            rbdModelAB->addConstraint(frictionConstraint);
        }
    }
}
}
}