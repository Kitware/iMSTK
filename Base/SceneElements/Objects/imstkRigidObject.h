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

#ifndef imstkRigidObject_h
#define imstkRigidObject_h

// imstk
#include "imstkDynamicObject.h"

namespace imstk
{
// TODO: Complete this
class RigidBodyState {};

///
/// \class RigidObject
///
/// \brief Base class for scene objects that is rigid and movable
///
class RigidObject : public DynamicObject<RigidBodyState>
{
public:

    ///
    /// \brief Constructor
    ///
    RigidObject(std::string name) : DynamicObject(name)
    {
        m_type = Type::Rigid;
    }

    ///
    /// \brief Destructor
    ///
    ~RigidObject() = default;

protected:
};
} // imstk

#endif // ifndef imstkRigidObject_h
