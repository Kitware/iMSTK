/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "NeedleInteraction.h"
#include "imstkCollidingObject.h"
#include "imstkCollisionDetectionAlgorithm.h"
#include "imstkLineMesh.h"
#include "imstkNeedle.h"
#include "imstkRigidObject2.h"
#include "imstkSignedDistanceField.h"
#include "imstkTaskGraph.h"
#include "NeedleRigidBodyCH.h"

using namespace imstk;

NeedleInteraction::NeedleInteraction(std::shared_ptr<CollidingObject> tissueObj,
                                     std::shared_ptr<RigidObject2>    needleObj,
                                     const std::string&               collisionName) :
    RigidObjectCollision(needleObj, tissueObj, collisionName)
{
    CHECK(needleObj->containsComponent<StraightNeedle>())
        << "NeedleInteraction only works with StraightNeedle component";
    CHECK(tissueObj->containsComponent<Puncturable>())
        << "NeedleInteraction only works with Puncturable component";
    CHECK(std::dynamic_pointer_cast<ImplicitGeometry>(tissueObj->getCollidingGeometry()) != nullptr)
        << "NeedleInteraction only works with SDF colliding geometry on colliding tissueObj";

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