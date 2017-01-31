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

#ifndef imstkSceneObjectController_h
#define imstkSceneObjectController_h

#include "imstkSceneObjectControllerBase.h"
#include "imstkDeviceTracker.h"
#include "imstkSceneObject.h"

#include <memory>

namespace imstk
{

///
/// \class SceneObjectController
///
/// \brief This class implements once tracking controller controlling one scnene object
///
class SceneObjectController : public SceneObjectControllerBase
{
public:
    ///
    /// \brief Constructor
    ///
    SceneObjectController(std::shared_ptr<SceneObject> sceneObject, std::shared_ptr<DeviceTracker> trackingController) :
        m_trackingController(trackingController), m_sceneObject(sceneObject) {}

    SceneObjectController() = delete;

    ///
    /// \brief Destructor
    ///
    ~SceneObjectController() = default;

    ///
    /// \brief Initialize offset based on object geometry
    ///
    void initOffsets() override;

    ///
    /// \brief Update controlled scene object using latest tracking information
    ///
    void updateControlledObjects() override;

    ///
    /// \brief Apply forces to the haptic device
    ///
    void applyForces() override;

    ///
    /// \brief Sets the tracker to out-of-date
    ///
    inline void setTrackerToOutOfDate() override { m_trackingController->setTrackerToOutOfDate(); }

    ///
    /// \brief Get/Set controlled scene object
    ///
    inline std::shared_ptr<SceneObject> getControlledSceneObject() const { return m_sceneObject; }
    inline void setControlledSceneObject(std::shared_ptr<SceneObject> so) { m_sceneObject = so; }

    ///
    /// \brief Get/Set tracking controller
    ///
    inline std::shared_ptr<DeviceTracker> getTrackingController() const { return m_trackingController; }
    inline void setTrackingController(std::shared_ptr<DeviceTracker> controller) { m_trackingController = controller; }

protected:
    std::shared_ptr<DeviceTracker> m_trackingController; ///< Device tracker
    std::shared_ptr<SceneObject> m_sceneObject;          ///< SceneObject controlled by the Tracker
};

} // imstk
#endif // ifndef imstkSceneObjectController_h
