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

#ifndef imstkSphereToSphereCD_h
#define imstkSphereToSphereCD_h

#include "imstkCollisionDetection.h"

#include <memory>

namespace imstk {

class CollidingObject;
class CollisionData;

class SphereToSphereCD : public CollisionDetection
{
public:

    ///
    /// \brief Constructor
    ///
    SphereToSphereCD() : CollisionDetection(CollisionDetection::Type::SphereToSphere) {}

    ///
    /// \brief Destructor
    ///
    ~SphereToSphereCD() = default;

    ///
    /// \brief Detect collision and compute collision data
    ///
    void computeCollisionData(std::shared_ptr<CollidingObject> objA,
                              std::shared_ptr<CollidingObject> objB,
                              CollisionData& colDataA,
                              CollisionData& colDataB) override;

};
}

#endif // ifndef imstkSphereToSphereCD_h
