/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkPbdRigidObjectCollision.h"

#include "NeedlePbdCH.h"
#include "NeedleRigidBodyCH.h"

using namespace imstk;

///
/// \class NeedleInteraction
///
/// \brief Defines interaction between NeedleObject and PbdObject
///
class NeedleInteraction : public PbdRigidObjectCollision
{
public:
    NeedleInteraction(std::shared_ptr<PbdObject> tissueObj, std::shared_ptr<NeedleObject> needleObj) : PbdRigidObjectCollision(tissueObj, needleObj)
    {
        if (std::dynamic_pointer_cast<LineMesh>(needleObj->getCollidingGeometry()) == nullptr)
        {
            LOG(WARNING) << "NeedleInteraction only works with LineMesh collision geometry on NeedleObject";
        }

        imstkNew<NeedleRigidBodyCH> needleRbdCH;
        needleRbdCH->setInputRigidObjectA(needleObj);
        needleRbdCH->setInputCollidingObjectB(tissueObj);
        needleRbdCH->setInputCollisionData(getCollisionDetection()->getCollisionData());
        needleRbdCH->setBaumgarteStabilization(0.001);
        setCollisionHandlingB(needleRbdCH);

        imstkNew<NeedlePbdCH> needlePbdCH;
        needlePbdCH->setInputObjectA(tissueObj);
        needlePbdCH->setInputObjectB(needleObj);
        needlePbdCH->setInputCollisionData(getCollisionDetection()->getCollisionData());
        needlePbdCH->getCollisionSolver()->setCollisionIterations(1);
        setCollisionHandlingA(needlePbdCH);
    }

    ~NeedleInteraction() override = default;
};