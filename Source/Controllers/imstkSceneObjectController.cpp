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
SceneObjectController::SceneObjectController(std::shared_ptr<SceneObject>  sceneObject,
                                             std::shared_ptr<DeviceClient> trackingDevice) :
    TrackingDeviceControl(trackingDevice),
    m_sceneObject(sceneObject)
{
}

void
SceneObjectController::update(const double dt)
{
    if (!isTrackerUpToDate())
    {
        if (!updateTrackingData(dt))
        {
            LOG(WARNING) << "warning: could not update tracking info.";
            return;
        }
    }

    if (m_sceneObject == nullptr)
    {
        return;
    }

    if (!m_deviceClient->getTrackingEnabled())
    {
        return;
    }

    this->postEvent(Event(SceneObjectController::modified()));

    // Update geometry
    // \todo revisit this; what if we need to move a group of objects
    m_sceneObject->getVisualGeometry()->setTranslation(getPosition());
    m_sceneObject->getVisualGeometry()->setRotation(getOrientation());
    m_sceneObject->getVisualGeometry()->postModified();
}
} // namespace imstk