/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "NeedleInteraction.h"
#include "imstkCollidingObject.h"
#include "imstkCollisionDetectionAlgorithm.h"
#include "imstkLineMesh.h"
#include "imstkSignedDistanceField.h"
#include "imstkTaskGraph.h"
#include "NeedleObject.h"
#include "NeedleRigidBodyCH.h"

using namespace imstk;

NeedleInteraction::NeedleInteraction(std::shared_ptr<CollidingObject> tissueObj,
                                     std::shared_ptr<NeedleObject>    needleObj) : RigidObjectCollision(needleObj, tissueObj, "ImplicitGeometryToPointSetCD")
{
    if (std::dynamic_pointer_cast<LineMesh>(needleObj->getCollidingGeometry()) == nullptr)
    {
        LOG(WARNING) << "NeedleInteraction only works with LineMesh collision geometry on rigid NeedleObject";
    }
    if (std::dynamic_pointer_cast<ImplicitGeometry>(tissueObj->getCollidingGeometry()) == nullptr)
    {
        LOG(WARNING) << "NeedleInteraction only works with SDF colliding geometry on colliding tissueObj";
    }

    // First replace the handlers with our needle subclasses

    // This handler consumes collision data to resolve the tool from the tissue
    // except when the needle is inserted
    auto needleRbdCH = std::make_shared<NeedleRigidBodyCH>();
    needleRbdCH->setInputRigidObjectA(needleObj);
    needleRbdCH->setInputCollidingObjectB(tissueObj);
    needleRbdCH->setInputCollisionData(getCollisionDetection()->getCollisionData());
    needleRbdCH->setBaumgarteStabilization(0.001);
    setCollisionHandlingA(needleRbdCH);
}