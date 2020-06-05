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

#include "imstkInteractionPair.h"

namespace imstk
{
class SceneObject;

///
/// \class ObjectInteractionPair
///
/// \brief This class defines an interaction between two SceneObjects
/// An interaction is a function occuring between two SceneObjects at some point
///
class ObjectInteractionPair : public InteractionPair
{
public:
    using SceneObjectPair = std::pair<std::shared_ptr<SceneObject>, std::shared_ptr<SceneObject>>;

// Cannot be constructed as synch points are externally undefinable
protected:
    ObjectInteractionPair() = default;
    ObjectInteractionPair(std::shared_ptr<SceneObject> objA, std::shared_ptr<SceneObject> objB);

    virtual ~ObjectInteractionPair() override = default;

public:
    /// \brief TODO
    const SceneObjectPair& getObjectsPair() const { return m_objects; }

public:
    ///
    /// \brief Modifies the compute graph of the provide objects to apply the interaction
    ///
    virtual void modifyComputeGraph() = 0;

protected:
    SceneObjectPair m_objects; ///> The two objects interacting
};
}
