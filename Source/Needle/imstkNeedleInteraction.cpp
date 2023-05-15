/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkNeedleInteraction.h"
#include "imstkCollisionDetectionAlgorithm.h"
#include "imstkLineMesh.h"
#include "imstkNeedle.h"
#include "imstkPbdObject.h"
#include "imstkPuncturable.h"
#include "imstkNeedlePbdCH.h"



namespace imstk 
{
    NeedleInteraction::NeedleInteraction(std::shared_ptr<PbdObject> tissueObj,
        std::shared_ptr<PbdObject> needleObj,
        std::shared_ptr<PbdObject> threadObj)
        : PbdObjectCollision(tissueObj, needleObj)
    {
        // Check inputs
        CHECK(threadObj != nullptr) << "NeedleInteraction: Thread object cannot be null";
        CHECK(tissueObj != nullptr) << "NeedleInteraction: Tissue object cannot be null";
        CHECK(needleObj != nullptr) << "NeedleInteraction: Needle object cannot be null";

        if (!needleObj->containsComponent<Needle>())
        {
            needleObj->addComponent<Needle>();
            LOG(INFO) << "NeedleInteraction: Needle component added to needle object";
        }

        if (!tissueObj->containsComponent<Puncturable>())
        {
            tissueObj->addComponent<Puncturable>();
            LOG(INFO) << "NeedleInteraction: Puncturable component added to tissue object";
        }

        CHECK(std::dynamic_pointer_cast<SurfaceMesh>(tissueObj->getCollidingGeometry()) != nullptr) <<
            "NeedleInteraction only works with SufraceMesh collision geometry on the tissue object";
        CHECK(std::dynamic_pointer_cast<LineMesh>(needleObj->getCollidingGeometry()) != nullptr) <<
            "NeedleInteraction only works with LineMesh collision geometry on NeedleObject";

        CHECK(threadObj->getPbdModel() == tissueObj->getPbdModel() &&
            threadObj->getPbdModel() == needleObj->getPbdModel())
            << "Tissue, thread, and needle must share a PbdModel";

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
                                     }
