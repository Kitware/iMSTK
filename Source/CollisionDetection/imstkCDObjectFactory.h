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

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace imstk
{
class CollisionDetectionAlgorithm;

///
/// \class CDObjectFactory
///
/// \brief This is the factory class for CollisionDetectionAlgorithm. It may be
/// used to construct CollisionDetectionAlgorithm objects by name.
/// Note: Does not auto register CollisionDetectionAlgorithm's. If one creates
/// their own CollisionDetectionAlgorithm they must register themselves.
///
class CDObjectFactory
{
public:
    using CDMakeFunc = std::function<std::shared_ptr<CollisionDetectionAlgorithm>()>;

    ///
    /// \brief Register the CollisionDetectionAlgorithm creation function given name
    ///
    static void registerCD(std::string name, CDMakeFunc func)
    {
        cdObjCreationMap[name] = func;
    }

    ///
    /// \brief Creates a CollisionDetectionAlgorithm object by name if registered to factory
    ///
    static std::shared_ptr<CollisionDetectionAlgorithm> makeCollisionDetection(const std::string collisionTypeName);

private:
    static std::unordered_map<std::string, CDMakeFunc> cdObjCreationMap;
};

///
/// \class CDObjectRegistrar
///
/// \brief Construction of this object will register to the CDObjectFactory. One could
/// construct this at the bottom of their CollisionDetectionAlgorithm when building
/// dynamic libraries or executables for static initialization.
///
template<typename T>
class CDObjectRegistrar
{
public:
    CDObjectRegistrar(std::string name)
    {
        CDObjectFactory::registerCD(name, []() { return std::make_shared<T>(); });
    }
};
#define REGISTER_COLLISION_DETECTION(cdType) CDObjectRegistrar<cdType> __register ## cdType(#cdType)
}