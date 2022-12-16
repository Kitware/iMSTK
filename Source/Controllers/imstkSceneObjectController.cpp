/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkSceneObjectController.h"
#include "imstkDeviceClient.h"
#include "imstkGeometry.h"
#include "imstkLogger.h"
#include "imstkSceneObject.h"

namespace imstk
{
SceneObjectController::SceneObjectController(const std::string& name) :
    TrackingDeviceControl(name)
{
}

void
SceneObjectController::update(const double& dt)
{
    if (!updateTrackingData(dt))
    {
        LOG(WARNING) << "warning: could not update tracking info.";
        return;
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