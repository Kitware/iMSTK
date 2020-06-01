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

#include "imstkObjectInteractionPair.h"
#include "imstkLogger.h"

namespace imstk
{
ObjectInteractionPair::ObjectInteractionPair(std::shared_ptr<SceneObject> objA, std::shared_ptr<SceneObject> objB)
{
    // Check that objects exist
    CHECK(objA != nullptr && objB != nullptr) << "InteractionPair error: invalid objects (nullptr).";

    this->m_objects.first  = objA;
    this->m_objects.second = objB;
}
}