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

    CHECK(std::dynamic_pointer_cast<SurfaceMesh>(tissueObj->getCollidingGeometry()) != nullptr) <<
        "NeedleInteraction only works with SufraceMesh collision geometry on the tissue object";

    CHECK(std::dynamic_pointer_cast<LineMesh>(needleObj->getCollidingGeometry()) != nullptr) <<
        "NeedleInteraction only works with LineMesh collision geometry on NeedleObject";

    CHECK(threadObj != nullptr) << "NeedleInteraction requires a PbdObject thread";

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
