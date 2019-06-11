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

#ifndef imstkPointSetToSpherePickingCD_h
#define imstkPointSetToSpherePickingCD_h

#include <memory>

#include "imstkCollisionDetection.h"
#include "imstkDeviceTracker.h"

namespace imstk
{
class PointSet;
class Sphere;
class CollisionData;

///
/// \class PointSetToSpherePickingCD
///
/// \brief PointSet to sphere collision detection while picking
///
class PointSetToSpherePickingCD : public CollisionDetection
{
public:

    ///
    /// \brief Constructor
    ///
    PointSetToSpherePickingCD(std::shared_ptr<PointSet> pointSet,
                              std::shared_ptr<Sphere> sphere,
                              std::shared_ptr<CollisionData> colData) :
        CollisionDetection(CollisionDetection::Type::PointSetToSphere,
                           colData),
        m_pointSet(pointSet),
        m_sphere(sphere){}

    ///
    /// \brief Destructor
    ///
    ~PointSetToSpherePickingCD() = default;

    ///
    /// \brief Detect collision and compute collision data
    ///
    void computeCollisionData() override;

    ///
    /// \brief Set device tracker and the id of the button
    ///
    void setDeviceTrackerAndButton(const std::shared_ptr<imstk::DeviceTracker> devTracker,
                                   const unsigned int buttonId = 0){ m_deviceTracker = devTracker; }
private:

    std::shared_ptr<PointSet> m_pointSet;  ///> PointSet
    std::shared_ptr<Sphere> m_sphere;      ///> Sphere

    std::shared_ptr<imstk::DeviceTracker> m_deviceTracker;  ///> Device tracker to get the button status
    unsigned int m_buttonId = 0;                            ///> button id
};
}

#endif // ifndef imstkPointSetToSphereCD_h
