/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkPbdObjectCollision.h"
#include "imstkPbdMethod.h"
#include "imstkStraightNeedle.h"
#include "NeedlePbdCH.h"
#include "NeedleRigidBodyCH.h"

using namespace imstk;

///
/// \class NeedleSurfaceInteraction
///
/// \brief Defines interaction between NeedleObject and PbdObject
///
class NeedleSurfaceInteraction : public PbdObjectCollision
{
public:
    NeedleSurfaceInteraction(std::shared_ptr<Entity> tissueObj,
                             std::shared_ptr<Entity> needleObj,
                             const std::string&      collisionName = "") : PbdObjectCollision(tissueObj, needleObj, collisionName)
    {
    }

    bool initialize() override
    {
        PbdObjectCollision::initialize();

        auto puncturable    = m_objA->getComponent<Puncturable>();
        auto straightNeedle = m_objB->getComponent<StraightNeedle>();

        CHECK(puncturable != nullptr)
            << "NeedleSurfaceInteraction only works with objects that have a Puncturable component";
        CHECK(straightNeedle != nullptr)
            << "NeedleSurfaceInteraction only works with objects that have a StraightNeedle component";

        auto needle = m_objA;
        auto tissue = m_objB;

        auto needleRigidCH = std::make_shared<NeedleRigidBodyCH>();
        needleRigidCH->setInputObjectA(needle->getComponentUnsafe<Collider>(), needle->getComponentUnsafe<PbdMethod>());
        needleRigidCH->setInputObjectB(tissue->getComponentUnsafe<Collider>(), tissue->getComponentUnsafe<PbdMethod>());
        needleRigidCH->setInputCollisionData(getCollisionDetection()->getCollisionData());
        needleRigidCH->setPuncturable(puncturable);
        needleRigidCH->setNeedle(straightNeedle);
        needleRigidCH->initialize();

        setCollisionHandlingB(needleRigidCH);

        auto needlePbdCH = std::make_shared<NeedlePbdCH>();
        needlePbdCH->setInputObjectA(tissue->getComponentUnsafe<Collider>(), tissue->getComponentUnsafe<PbdMethod>());
        needlePbdCH->setInputObjectB(needle->getComponentUnsafe<Collider>(), needle->getComponentUnsafe<PbdMethod>());
        needlePbdCH->setInputCollisionData(getCollisionDetection()->getCollisionData());
        needlePbdCH->setPuncturable(puncturable);
        needlePbdCH->setNeedle(straightNeedle);
        // These two can control compliance
        needlePbdCH->setDeformableStiffnessA(1.0);
        needlePbdCH->setDeformableStiffnessB(0.01);
        needlePbdCH->initialize();

        setCollisionHandlingA(needlePbdCH);
        return true;
    }

    ~NeedleSurfaceInteraction() override = default;

private:
    std::shared_ptr<Entity> m_needle, m_tissue;
};