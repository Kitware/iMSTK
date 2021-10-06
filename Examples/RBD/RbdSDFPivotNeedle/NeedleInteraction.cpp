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
    needleRbdCH->setBeta(0.001);
    needleRbdCH->getTaskNode()->m_isCritical = true;
    setCollisionHandlingA(needleRbdCH);
}