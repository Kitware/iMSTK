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
        needleRbdCH->setBeta(0.001);
        needleRbdCH->getTaskNode()->m_isCritical = true;
        setCollisionHandlingB(needleRbdCH);

        imstkNew<NeedlePbdCH> needlePbdCH;
        needlePbdCH->setInputObjectA(tissueObj);
        needlePbdCH->setInputObjectB(needleObj);
        needlePbdCH->setInputCollisionData(getCollisionDetection()->getCollisionData());
        needlePbdCH->getCollisionSolver()->setCollisionIterations(1);
        needlePbdCH->getTaskNode()->m_isCritical = true;
        setCollisionHandlingA(needlePbdCH);
    }

    ~NeedleInteraction() override = default;
};