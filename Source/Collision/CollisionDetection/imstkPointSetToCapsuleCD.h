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

#ifndef imstkPointSetToCapsuleCD_h
#define imstkPointSetToCapsuleCD_h

#include <memory>

#include "imstkCollisionDetection.h"

namespace imstk
{
class PointSet;
class Capsule;
class CollisionData;

///
/// \class PointSetToCapsuleCD
///
/// \brief PointSet to Capsule collision detection
///
class PointSetToCapsuleCD : public CollisionDetection
{
public:

    ///
    /// \brief Constructor
    ///
    PointSetToCapsuleCD(std::shared_ptr<PointSet> pointSet,
                        std::shared_ptr<Capsule> capsule,
                        std::shared_ptr<CollisionData> colData);

    ///
    /// \brief Destructor
    ///
    virtual ~PointSetToCapsuleCD() override = default;

    ///
    /// \brief Detect collision and compute collision data
    ///
    void computeCollisionData() override;

private:

    std::shared_ptr<PointSet> m_pointSet;       ///> PointSet
    std::shared_ptr<Capsule> m_capsule;         ///> Capsule
};
}

#endif // ifndef imstkPointSetToCapsuleCD_h
