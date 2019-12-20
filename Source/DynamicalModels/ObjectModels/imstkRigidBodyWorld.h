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

#ifndef imstkRigidBodyWorld_h
#define imstkRigidBodyWorld_h

#define PhysX_RELEASE(x)        if (x) { x->release(); x = NULL; }

#include "physx/PxConfig.h"
#include "physx/PxPhysicsAPI.h"

#include "imstkDynamicalModel.h"
#include "imstkRigidBodyState.h"
#include "imstkSurfaceMesh.h"

//#include "../snippets/snippetcommon/SnippetPrint.h"
//#include "../snippets/snippetcommon/SnippetPVD.h"
//#include "../snippets/snippetutils/SnippetUtils.h"

namespace imstk
{
using namespace physx;

///
/// \class RigidBodyWorld
///
/// \brief Interface to the PhysX context for rigid bodies
///
class RigidBodyWorld
{
public:
    ///
    /// \brief Destructor of RigidBodyWorld
    ///
    ~RigidBodyWorld();

    ///
    /// \brief Configure rigid body world
    ///
    void configure() {};

    ///
    /// \brief Get the singleton instance
    ///
    static RigidBodyWorld* getInstance();

    PxDefaultAllocator     m_Allocator;
    PxDefaultErrorCallback m_ErrorCallback;
    PxFoundation*          m_Foundation = NULL;
    PxPhysics* m_Physics = NULL;
    PxCooking* m_Cooking = NULL;
    PxDefaultCpuDispatcher* m_Dispatcher = NULL;
    PxScene* m_Scene = NULL;
    PxPvd*   m_Pvd   = NULL;

private:
    static RigidBodyWorld* m_rigidBodyWorldInstance;

    ///
    /// \brief Constructor of RigidBodyWorld
    ///
    RigidBodyWorld();
};
} // imstk

#endif // imstkRigidBodyWorld_h