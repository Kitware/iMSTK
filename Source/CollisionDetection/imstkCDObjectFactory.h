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

#include "imstkFactory.h"

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace imstk
{
class CollisionDetectionAlgorithm;

class CDObjectFactory : public ObjectFactory<std::shared_ptr<CollisionDetectionAlgorithm>>
{
public:
static std::shared_ptr<CollisionDetectionAlgorithm> 
makeCollisionDetection(const std::string collisionTypeName);
};

template<typename T>
using CDObjectRegistrar = SharedObjectRegistrar<CollisionDetectionAlgorithm, T>;

#define IMSTK_REGISTER_COLLISION_DETECTION(objType) CDObjectRegistrar<objType> _imstk_registercd ## objType(#objType)
} // namespace imstk
