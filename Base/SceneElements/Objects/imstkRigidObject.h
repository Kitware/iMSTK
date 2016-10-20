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
#include "imstkMath.h"

namespace imstk
{

///
/// \class RigidObject
///
/// \brief Base class for scene objects that is rigid and movable
///
class RigidObject : public DynamicObject
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

    ///
    /// \brief Translate the physics geometric model
    ///
    void translate(const Vec3d& t);
    void translate(const double& x, const double& y, const double& z);

    ///
    /// \brief Rotate the physics geometric model
    ///
    void rotate(const Quatd& r);
    void rotate(const Mat3d& r);
    void rotate(const Vec3d & axis, const double& angle);

protected:

};

} // imstk

#endif // ifndef imstkRigidObject_h
