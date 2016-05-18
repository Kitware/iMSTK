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

#ifndef imstkVirtualCouplingObject_h
#define imstkVirtualCouplingObject_h

#include "imstkCollidingObject.h"
#include "imstkTrackingController.h"

namespace imstk {

class Geometry;
class GeometryMap;

class VirtualCouplingObject : public CollidingObject, public TrackingController
{
public:

    VirtualCouplingObject(std::string name,
                          std::shared_ptr<DeviceClient> deviceClient = nullptr,
                          double scaling = 1.0) :
        CollidingObject(name),
        TrackingController(deviceClient, scaling)
    {
        m_type = Type::VirtualCoupling;
    }

    ~VirtualCouplingObject() = default;

    ///
    /// \brief Initialise offset based on object geometry
    ///
    void initOffsets();

    ///
    /// \brief Update geometries transformations
    ///
    void updateFromDevice();

};

}

#endif // ifndef imstkVirtualCouplingObject_h
