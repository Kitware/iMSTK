/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkPbdObjectCollision.h"
#include "imstkPbdObject.h"
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
    NeedleSurfaceInteraction(std::shared_ptr<PbdObject> tissueObj,
                             std::shared_ptr<PbdObject> needleObj,
                             const std::string&         collisionName = "") : PbdObjectCollision(tissueObj, needleObj, collisionName)
    {
        CHECK(needleObj->containsComponent<StraightNeedle>())
            << "NeedleSurfaceInteraction only works with objects that have a StraightNeedle component";
        CHECK(tissueObj->containsComponent<Puncturable>())
            << "NeedleSurfaceInteraction only works with objects that have a Puncturable component";

        auto needleRigidCH = std::make_shared<NeedleRigidBodyCH>();
        needleRigidCH->setInputObjectA(needleObj);
        needleRigidCH->setInputObjectB(tissueObj);
        needleRigidCH->setInputCollisionData(getCollisionDetection()->getCollisionData());
        setCollisionHandlingB(needleRigidCH);

        auto needlePbdCH = std::make_shared<NeedlePbdCH>();
        needlePbdCH->setInputObjectA(tissueObj);
        needlePbdCH->setInputObjectB(needleObj);
        needlePbdCH->setInputCollisionData(getCollisionDetection()->getCollisionData());
        // These two can control compliance
        needlePbdCH->setDeformableStiffnessA(1.0);
        needlePbdCH->setDeformableStiffnessB(0.01);
        setCollisionHandlingA(needlePbdCH);
    }

    ~NeedleSurfaceInteraction() override = default;
};