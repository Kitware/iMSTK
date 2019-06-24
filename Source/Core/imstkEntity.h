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

#include <atomic>

namespace imstk
{
using EntityID = unsigned long;

///
/// \class Entity
/// \brief Top-level class for iMSTK objects
///
class Entity
{
public:
    ///
    /// \brief Get ID (ALWAYS query the ID in your code, DO NOT hardcode it)
    /// \returns ID of entity
    ///
    EntityID getID() const;

protected:
    ///
    /// \brief Constructor
    ///
    Entity();

    // Not the best design pattern
    static std::atomic<EntityID> s_count; ///< current count of entities

    EntityID m_ID; ///< unique ID of entity
};
}

