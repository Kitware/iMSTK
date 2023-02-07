/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "NeedleInteraction.h"
#include "imstkCollider.h"
#include "imstkCollisionDetectionAlgorithm.h"
#include "imstkLineMesh.h"
#include "imstkNeedle.h"
#include "imstkPbdObject.h"
#include "imstkPuncturable.h"
#include "NeedlePbdCH.h"

using namespace imstk;

NeedleInteraction::NeedleInteraction(std::shared_ptr<PbdObject> tissueObj,
                                     std::shared_ptr<PbdObject> needleObj,
                                     std::shared_ptr<PbdObject> threadObj)
    : PbdObjectCollision(tissueObj, needleObj)
{
    // Check inputs
    CHECK(needleObj->containsComponent<Needle>())
        << "NeedleInteraction only works with objects that have a Needle component";
    CHECK(tissueObj->containsComponent<Puncturable>())
        << "NeedleInteraction only works with objects that have a Puncturable component";

    CHECK(std::dynamic_pointer_cast<SurfaceMesh>(tissueObj->getComponent<Collider>()->getGeometry()) != nullptr) <<
        "NeedleInteraction only works with SufraceMesh collision geometry on the tissue object";
    CHECK(std::dynamic_pointer_cast<LineMesh>(needleObj->getComponent<Collider>()->getGeometry()) != nullptr) <<
        "NeedleInteraction only works with LineMesh collision geometry on NeedleObject";

    CHECK(threadObj->getPbdModel() == tissueObj->getPbdModel()) << "Tissue and thread must share a PbdSystem";

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
