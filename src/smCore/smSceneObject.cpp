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

#include "smSceneObject.h"
#include "smObjectSimulator.h"
#include "smSDK.h"

smSceneObject::smSceneObject()
{
    type = SIMMEDTK_SMSCENEOBJECT_TYPE_UNKNOWN;
    isActive = true;
    objectSim = nullptr;
    customRender = nullptr;
    //     smSDK::getInstance()->registerSceneObject(safeDownCast<smSceneObject>());
    flags.isViewerInit = false;
    flags.isSimulatorInit = false;
    name = "SceneObject" + std::to_string(this->getUniqueId()->getId());
}

smSceneObject::~smSceneObject()
{
}

void smSceneObject::attachObjectSimulator(std::shared_ptr<smObjectSimulator> p_objectSim)
{
    p_objectSim->addObject(safeDownCast<smSceneObject>());
}

void smSceneObject::releaseObjectSimulator()
{
    objectSim->removeObject(safeDownCast<smSceneObject>());
    objectSim = nullptr;
}

std::shared_ptr<smObjectSimulator> smSceneObject::getObjectSimulator()
{
    return objectSim;
}

void smSceneObject::attachCustomRenderer(std::shared_ptr<smCustomRenderer> p_customeRenderer)
{
    customRender = p_customeRenderer;
}

void smSceneObject::releaseCustomeRenderer()
{
    customRender = nullptr;
}

smInt smSceneObject::getObjectId()
{
    return this->getUniqueId()->getId();
}

smUnifiedId::Pointer smSceneObject::getObjectUnifiedID()
{
    return std::make_shared<smUnifiedId>();
}

smStdVector3d & smSceneObject::getLocalVertices()
{
    return localVertices;
}

smObjectInitFlags & smSceneObject::getFlags()
{
    return flags;
}

std::shared_ptr<smCustomRenderer> smSceneObject::getRenderer()
{
    return customRender;
}

void smSceneObject::freeze()
{
    this->isActive = false;
}

void smSceneObject::activate()
{
    this->isActive = true;
}