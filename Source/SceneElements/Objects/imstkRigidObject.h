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

#ifdef iMSTK_USE_ODE
// imstk
#include "imstkDynamicObject.h"
#include "ode/ode.h"

namespace imstk
{
// TODO: Complete this
class RigidBodyState
{
public:
    ///
    /// \brief Set the state to a given one
    ///
    void setState(std::shared_ptr<RigidBodyState> rhs){};
};

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

    //ode related functions

    //
    // callback called at every loop
    //
    static void odeNearCallback(void *data, dGeomID o1, dGeomID o2);

    // initialize ode
    static void initOde();

    // close ode
    static void closeOde();

    //
    static void simulationStep();

    //
    static void getGeometryConfig(imstk::Vec3d &p, imstk::Mat3d &orientation);

    // This is just a simple function to test ODE
    static void setup();

    ///
    /// \brief Destructor
    ///
    ~RigidObject() = default;

protected:
};
} // imstk
#endif //imstk_USE_ODE
#endif // ifndef imstkRigidObject_h
