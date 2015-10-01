// This file is part of the SimMedTK project.
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
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//---------------------------------------------------------------------------

#include "Core/SceneObject.h"
#include "Core/ObjectSimulator.h"
#include "Core/SDK.h"

SceneObject::SceneObject()
{
    type = core::ClassType::Unknown;
    isActive = true;
    objectSim = nullptr;
    customRender = nullptr;
    //     SDK::getInstance()->registerSceneObject(safeDownCast<SceneObject>());
    flags.isViewerInit = false;
    flags.isSimulatorInit = false;
    name = "SceneObject" + std::to_string(this->getUniqueId()->getId());
    hasContactForces = false;
}

SceneObject::~SceneObject()
{
}

void SceneObject::attachObjectSimulator(std::shared_ptr<ObjectSimulator> p_objectSim)
{
    p_objectSim->addObject(safeDownCast<SceneObject>());
}

void SceneObject::releaseObjectSimulator()
{
    objectSim->removeObject(safeDownCast<SceneObject>());
    objectSim = nullptr;
}

std::shared_ptr<ObjectSimulator> SceneObject::getObjectSimulator()
{
    return objectSim;
}

void SceneObject::attachCustomRenderer(std::shared_ptr<CustomRenderer> p_customeRenderer)
{
    customRender = p_customeRenderer;
}

void SceneObject::releaseCustomeRenderer()
{
    customRender = nullptr;
}

int SceneObject::getObjectId()
{
    return this->getUniqueId()->getId();
}

UnifiedId::Pointer SceneObject::getObjectUnifiedID()
{
    return std::make_shared<UnifiedId>();
}

std::vector<core::Vec3d> & SceneObject::getLocalVertices()
{
    return localVertices;
}

ObjectInitFlags & SceneObject::getFlags()
{
    return flags;
}

std::shared_ptr<CustomRenderer> SceneObject::getRenderer()
{
    return customRender;
}

void SceneObject::freeze()
{
    this->isActive = false;
}

void SceneObject::activate()
{
    this->isActive = true;
}
