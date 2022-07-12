/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "NeedleInteraction.h"
#include "imstkLineMesh.h"
#include "imstkPbdObject.h"
#include "imstkCollisionDetectionAlgorithm.h"
#include "NeedlePbdCH.h"
#include "NeedleObject.h"

using namespace imstk;

NeedleInteraction::NeedleInteraction(std::shared_ptr<PbdObject>    tissueObj,
                                     std::shared_ptr<NeedleObject> needleObj,
                                     std::shared_ptr<PbdObject>    threadObj)
    : PbdObjectCollision(tissueObj, needleObj)
{
    // Check inputs
    CHECK(tissueObj != nullptr) << "NeedleInteraction requires a PbdObject tissue";
    CHECK(std::dynamic_pointer_cast<SurfaceMesh>(tissueObj->getCollidingGeometry()) != nullptr) <<
        "NeedleInteraction only works with SufraceMesh collision geometry on the tissue object";

    CHECK(threadObj != nullptr) << "NeedleInteraction requires a PbdObject thread";
    CHECK(std::dynamic_pointer_cast<LineMesh>(needleObj->getCollidingGeometry()) != nullptr) <<
        "NeedleInteraction only works with LineMesh collision geometry on NeedleObject";

    CHECK(threadObj->getPbdModel() == tissueObj->getPbdModel()) << "Tissue and thread must share a PbdModel";

    // Add collision handler for the PBD reaction
    auto needlePbdCH = std::make_shared<NeedlePbdCH>();
    needlePbdCH->setInputObjectA(tissueObj);
    needlePbdCH->setInputObjectB(needleObj);
    needlePbdCH->setInputCollisionData(getCollisionDetection()->getCollisionData());
    needlePbdCH->init(threadObj);
    setCollisionHandlingAB(needlePbdCH);
}

void
NeedleInteraction::stitch()
{
    auto CH = std::dynamic_pointer_cast<NeedlePbdCH>(this->getCollisionHandlingAB());
    CH->stitch();
}
