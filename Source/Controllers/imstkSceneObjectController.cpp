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
#include "imstkDeviceClient.h"
#include "imstkGeometry.h"
#include "imstkLogger.h"

namespace imstk
{
SceneObjectController::SceneObjectController(std::shared_ptr<SceneObject> sceneObject, std::shared_ptr<DeviceClient> trackingDevice) :
    TrackingDeviceControl(trackingDevice), m_sceneObject(sceneObject)
{
}

void
SceneObjectController::updateControlledObjects()
{
    if (!isTrackerUpToDate())
    {
        if (!updateTrackingData())
        {
            LOG(WARNING) << "SceneObjectController::updateControlledObjects warning: could not update tracking info.";
            return;
        }
    }

    if (m_updateCallback)
    {
        m_updateCallback(this);
    }

    // Update colliding geometry
    m_sceneObject->getMasterGeometry()->setTranslation(getPosition());
    m_sceneObject->getMasterGeometry()->setRotation(getRotation());
}

void
SceneObjectController::applyForces()
{
    if (auto collidingObject = dynamic_cast<CollidingObject*>(m_sceneObject.get()))
    {
        m_deviceClient->setForce(collidingObject->getForce());
    }
}
}