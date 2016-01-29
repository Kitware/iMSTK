// This file is part of the iMSTK project.
//
// Copyright (c) Kitware, Inc.
//
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "SceneModels/SceneObject.h"
#include "Simulators/ObjectSimulator.h"
#include "Core/BaseMesh.h"
#include "Core/Model.h"
#include "Core/Factory.h"

namespace imstk {

SceneObject::SceneObject()
{
    this->hasContactForces = false;
    this->isActive = true;
    this->flags.isViewerInit = false;
    this->flags.isSimulatorInit = false;
    this->type = ClassType::Unknown;
    this->objectSim = nullptr;
    this->name = "SceneObject" + std::to_string(this->getUniqueId()->getId());
    this->numOfDOF = 0;
    this->numOfNodes = 0;

    this->setRenderDelegate(
        Factory<RenderDelegate>::createConcreteClassForGroup(
        "SceneModelRenderDelegate",RenderDelegate::RendererType::VTK));
}

//---------------------------------------------------------------------------
bool SceneObject::configure(const std::string &)
{
    return false;
}

//---------------------------------------------------------------------------
void SceneObject::attachObjectSimulator(std::shared_ptr<ObjectSimulator> p_objectSim)
{
    if(!p_objectSim)
    {
        // Log this
        return;
    }
    this->objectSim = p_objectSim;
}

//---------------------------------------------------------------------------
void SceneObject::releaseObjectSimulator()
{
    if(!this->objectSim)
    {
        // Log this
        return;
    }
    this->objectSim->removeModel(safeDownCast<SceneObject>());
    this->objectSim = nullptr;
}

//---------------------------------------------------------------------------
void SceneObject::freeze()
{
    this->isActive = false;
}

//---------------------------------------------------------------------------
std::shared_ptr<ObjectSimulator> SceneObject::getObjectSimulator()
{
    return this->objectSim;
}

//---------------------------------------------------------------------------
int SceneObject::getObjectId()
{
    return this->getUniqueId()->getId();
}

//---------------------------------------------------------------------------
UnifiedId::Pointer SceneObject::getObjectUnifiedID()
{
    return std::make_shared<UnifiedId>();
}

//---------------------------------------------------------------------------
ObjectInitFlags & SceneObject::getFlags()
{
    return this->flags;
}

//---------------------------------------------------------------------------
void SceneObject::activate()
{
    this->isActive = true;
}

//---------------------------------------------------------------------------
bool SceneObject::computeContactForce()
{
    return this->hasContactForces;
}

//---------------------------------------------------------------------------
void SceneObject::setContactForcesOff()
{
    this->hasContactForces = false;
}

//---------------------------------------------------------------------------
void SceneObject::setContactForcesOn()
{
    this->hasContactForces = true;
}

//---------------------------------------------------------------------------
std::unordered_map< size_t, Vec3d> &SceneObject::getContactForces()
{
    return this->contactForces;
}

//---------------------------------------------------------------------------
void SceneObject::setContactForces(const std::unordered_map<size_t,Vec3d> &forces)
{
    this->contactForces.clear();
    this->contactForces = forces;
}

//---------------------------------------------------------------------------
const std::unordered_map< size_t,Vec3d> &SceneObject::getContactForces() const
{
    return this->contactForces;
}

//---------------------------------------------------------------------------
std::unordered_map< size_t,Vec3d> &SceneObject::getContactPoints()
{
    return this->contactPoints;
}

//---------------------------------------------------------------------------
const std::unordered_map< size_t,Vec3d> &SceneObject::getContactPoints() const
{
    return this->contactPoints;
}

//---------------------------------------------------------------------------
Eigen::Map<Vec3d> SceneObject::getVelocity(const int) const
{
    // TODO: Dangerous - Make this function pure virtual.
    return Eigen::Map<Vec3d>(nullptr);
}

//---------------------------------------------------------------------------
void SceneObject::setContactForcesToZero()
{
    this->contactForces.clear();
    this->contactPoints.clear();
}

//---------------------------------------------------------------------------
void SceneObject::setContactForce(const int dofID, const Vec3d &force)
{
    this->contactForces[dofID] = force;
}

//---------------------------------------------------------------------------
void SceneObject::
setContactForce(const int dofID, const Vec3d &point, const Vec3d &force)
{
    this->contactPoints[dofID] = point;
    this->contactForces[dofID] = force;
}

//---------------------------------------------------------------------------
void SceneObject::setModel(std::shared_ptr< Model > m)
{
    this->visualModel = m;
    this->collisionModel = m;
    this->physicsModel = m;
}

//---------------------------------------------------------------------------
void SceneObject::setVisualModel(std::shared_ptr< Model > m)
{
    this->visualModel = m;
}

//---------------------------------------------------------------------------
std::shared_ptr< Model > SceneObject::getVisualModel()
{
    return this->visualModel;
}

//---------------------------------------------------------------------------
void SceneObject::setCollisionModel(std::shared_ptr< Model > m)
{
    this->collisionModel = m;
}

//---------------------------------------------------------------------------
std::shared_ptr< Model > SceneObject::getCollisionModel()
{
    return this->collisionModel;
}

//---------------------------------------------------------------------------
void SceneObject::setPhysicsModel(std::shared_ptr< Model > m)
{
    this->physicsModel = m;
}

//---------------------------------------------------------------------------
std::shared_ptr< Model > SceneObject::getPhysicsModel()
{
    return this->physicsModel;
}

//---------------------------------------------------------------------------
void SceneObject::update(const double)
{
}

//---------------------------------------------------------------------------
void SceneObject::updateExternalForces(const std::unordered_map<size_t,Vec3d> &forces)
{
    this->contactForces = forces;
}

//---------------------------------------------------------------------------
std::size_t SceneObject::getNumOfDOF() const
{
    return this->numOfDOF;
}

//---------------------------------------------------------------------------
std::size_t SceneObject::getNumOfNodes() const
{
    return this->numOfNodes;
}

}
