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

#include "imstkSceneObjectController.h"

#include "imstkCollidingObject.h"
#include "imstkGeometry.h"

#include <utility>

#include <g3log/g3log.hpp>

namespace imstk
{

void
SceneObjectController::initOffsets()
{
    m_translationOffset = m_sceneObject.getMasterGeometry()->getPosition();
    m_rotationOffset = m_sceneObject.getMasterGeometry()->getOrientation();
}

void
SceneObjectController::updateFromDevice()
{
    Vec3d p;
    Quatd r;

    if (!this->computeTrackingData(p, r))
    {
        LOG(WARNING) << "SceneObjectController::updateFromDevice warning: could not update tracking info.";
        return;
    }

    // Update colliding geometry
    m_sceneObject.getMasterGeometry()->setPosition(p);
    m_sceneObject.getMasterGeometry()->setOrientation(r);
}

void
SceneObjectController::applyForces()
{
    if(auto collidingObject = dynamic_cast<CollidingObject*>(&m_sceneObject))
    {
        m_deviceClient->setForce(collidingObject->getForce());
    }
}

} // imstk
