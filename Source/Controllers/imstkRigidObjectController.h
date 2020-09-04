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

#include "imstkSceneObjectController.h"

namespace imstk
{
namespace expiremental
{
class RigidObject2;

///
/// \class RigidObjectController
///
/// \brief This class uses the provide device to control the provided rigid object via
/// virtual coupling
///
class RigidObjectController : public SceneObjectController
{
public:
    ///
    /// \brief Constructor
    ///
    RigidObjectController(std::shared_ptr<RigidObject2> rigidObject, std::shared_ptr<DeviceClient> trackingDevice);
    RigidObjectController() = delete;

    ///
    /// \brief Destructor
    ///
    virtual ~RigidObjectController() override = default;

public:
    ///
    /// \brief Update controlled scene object using latest tracking information
    ///
    void updateControlledObjects() override;

    ///
    /// \brief Apply forces to the haptic device
    ///
    void applyForces() override;

protected:
    std::shared_ptr<RigidObject2> m_rigidObject;
};
}
}
