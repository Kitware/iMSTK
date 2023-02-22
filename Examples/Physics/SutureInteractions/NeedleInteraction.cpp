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
#include "imstkPbdMethod.h"
#include "imstkPuncturable.h"
#include "NeedlePbdCH.h"

using namespace imstk;

NeedleInteraction::NeedleInteraction(std::shared_ptr<Entity> tissueObj,
                                     std::shared_ptr<Entity> needleObj,
                                     std::shared_ptr<Entity> threadObj)
    : PbdObjectCollision(tissueObj, needleObj), m_threadObj(threadObj)
{
}

bool
NeedleInteraction::initialize()
{
    PbdObjectCollision::initialize();

    auto tissueObj = m_objA;
    auto needleObj = m_objB;
    if (m_objA->containsComponent<Needle>())
    {
        std::swap(tissueObj, needleObj);
    }

    // Check inputs
    CHECK(needleObj->containsComponent<Needle>())
        << "NeedleInteraction only works with objects that have a Needle component";
    CHECK(tissueObj->containsComponent<Puncturable>())
        << "NeedleInteraction only works with objects that have a Puncturable component";

    CHECK(std::dynamic_pointer_cast<SurfaceMesh>(tissueObj->getComponent<Collider>()->getGeometry()) != nullptr) <<
        "NeedleInteraction only works with SufraceMesh collision geometry on the tissue object";
    CHECK(std::dynamic_pointer_cast<LineMesh>(needleObj->getComponent<Collider>()->getGeometry()) != nullptr) <<
        "NeedleInteraction only works with LineMesh collision geometry on NeedleObject";

    CHECK(m_threadObj->getComponent<PbdMethod>()->getPbdSystem() == tissueObj->getComponent<PbdMethod>()->getPbdSystem()) << "Tissue and thread must share a PbdSystem";

    // Add collision handler for the PBD reaction
    auto needlePbdCH = std::make_shared<NeedlePbdCH>();
    needlePbdCH->setTissue(tissueObj);
    needlePbdCH->setNeedle(needleObj);
    needlePbdCH->setThread(m_threadObj);
    needlePbdCH->setInputObjectA(tissueObj->getComponent<Collider>(), tissueObj->getComponent<PbdMethod>());
    needlePbdCH->setInputObjectB(needleObj->getComponent<Collider>(), needleObj->getComponent<PbdMethod>());
    needlePbdCH->setInputCollisionData(getCollisionDetection()->getCollisionData());
    needlePbdCH->initialize();
    setCollisionHandlingAB(needlePbdCH);

    return true;
}

void
NeedleInteraction::stitch()
{
    auto CH = std::dynamic_pointer_cast<NeedlePbdCH>(this->getCollisionHandlingAB());
    CH->stitch();
}
