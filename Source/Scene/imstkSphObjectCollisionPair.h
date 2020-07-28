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

#include "imstkCollisionPair.h"
#include "imstkCollisionDetection.h"

namespace imstk
{
class SPHObject;

///
/// \class SphObjectCollisionPair
///
/// \brief This class defines where collision should happen bsetween an SphObject and colliding object
///
class SphObjectCollisionPair : public CollisionPair
{
public:
    SphObjectCollisionPair(std::shared_ptr<SPHObject> obj1, std::shared_ptr<CollidingObject> obj2,
                           CollisionDetection::Type cdType = CollisionDetection::Type::PointSetToImplicit);

    virtual ~SphObjectCollisionPair() override = default;
};
}