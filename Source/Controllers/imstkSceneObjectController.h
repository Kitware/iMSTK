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

#include "imstkTrackingDeviceControl.h"

#include <functional>

namespace imstk
{
class SceneObject;

///
/// \class SceneObjectController
///
/// \brief This class implements once tracking controller controlling one scnene object
///
class SceneObjectController : public TrackingDeviceControl
{
public:
    SceneObjectController(std::shared_ptr<SceneObject> sceneObject, std::shared_ptr<DeviceClient> trackingDevice);
    SceneObjectController() = delete;
    virtual ~SceneObjectController() override = default;

public:
    ///
    /// \brief Update controlled scene object using latest tracking information
    ///
    void updateControlledObjects() override;

    ///
    /// \brief Apply forces to the haptic device
    ///
    void applyForces() override;

    ///
    /// \brief Get/Set controlled scene object
    ///
    inline std::shared_ptr<SceneObject> getControlledSceneObject() const { return m_sceneObject; }
    inline void setControlledSceneObject(std::shared_ptr<SceneObject> so) { m_sceneObject = so; }

protected:
    std::shared_ptr<SceneObject> m_sceneObject; ///< SceneObject controlled by the Tracker
};
} // imstk
