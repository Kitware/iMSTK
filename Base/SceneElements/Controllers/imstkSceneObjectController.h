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

#include "imstkTrackingController.h"
#include "imstkSceneObject.h"

#include <memory>

namespace imstk
{

///
/// \class SceneObjectController
///
/// \brief
///
class SceneObjectController : public TrackingController
{
public:
    ///
    /// \brief Constructor
    ///
    SceneObjectController(SceneObject& sceneObject,
                          std::shared_ptr<DeviceClient> deviceClient) :
        TrackingController(deviceClient),
        m_sceneObject(sceneObject)
    {}

    ///
    /// \brief Destructor
    ///
    ~SceneObjectController() = default;

    ///
    /// \brief Initialize offset based on object geometry
    ///
    void initOffsets();

    ///
    /// \brief Update geometries transformations
    ///
    void updateFromDevice();

    ///
    /// \brief Apply forces to the haptic device
    ///
    void applyForces();

protected:

    SceneObject& m_sceneObject; ///< SceneObject controlled by the external device

};

} // imstk
#endif // ifndef imstkSceneObjectController_h
