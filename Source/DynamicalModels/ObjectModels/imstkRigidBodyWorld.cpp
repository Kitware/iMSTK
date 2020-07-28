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

#include "imstkRigidBodyWorld.h"
#include "imstkLogger.h"

#define MAX_CONTACTS_PER_BODY 16

namespace imstk
{
RigidBodyWorld* RigidBodyWorld::m_rigidBodyWorldInstance = 0;

RigidBodyWorld::RigidBodyWorld()
{
    m_Foundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_Allocator, m_ErrorCallback);
    m_Physics    = PxCreatePhysics(PX_PHYSICS_VERSION, *m_Foundation, PxTolerancesScale(), true, m_Pvd);
    m_Cooking    = PxCreateCooking(PX_PHYSICS_VERSION, *m_Foundation, PxCookingParams(PxTolerancesScale()));

    // TODO: move this into a separate structure that user can supply
    PxSceneDesc sceneDesc(m_Physics->getTolerancesScale());
    sceneDesc.gravity       = PxVec3(0.0f, -9.81f, 0.0f);
    m_Dispatcher            = PxDefaultCpuDispatcherCreate(2);
    sceneDesc.cpuDispatcher = m_Dispatcher;
    sceneDesc.solverType    = PxSolverType::eTGS;
    sceneDesc.filterShader  = PxDefaultSimulationFilterShader;
    m_Scene = m_Physics->createScene(sceneDesc);
}

RigidBodyWorld*
RigidBodyWorld::getInstance()
{
    if (!m_rigidBodyWorldInstance)
    {
        m_rigidBodyWorldInstance = new RigidBodyWorld();
    }
    return m_rigidBodyWorldInstance;
}

RigidBodyWorld::~RigidBodyWorld()
{
    if (!m_rigidBodyWorldInstance)
    {
        return;
    }

    // cleanup physx
    PhysX_RELEASE(m_Scene);
    PhysX_RELEASE(m_Dispatcher);
    PxCloseExtensions();
    PhysX_RELEASE(m_Physics);
    PhysX_RELEASE(m_Cooking);
    PhysX_RELEASE(m_Foundation);
}
} // imstk