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

#include "Core/StaticSceneObject.h"
#include "Core/Factory.h"
#include "RenderDelegate.h"

StaticSceneObject::StaticSceneObject(std::shared_ptr<ErrorLog> /*p_log*/) : SceneObject()
{
    type = core::ClassType::StaticSceneObject;

    name = "Static_SceneObject_" + std::to_string(this->getUniqueId()->getId());

    this->setRenderDelegate(
        Factory<RenderDelegate>::createConcreteClassForGroup(
        "StaticSceneObjectRenderDelegate",RenderDelegate::RendererType::VTK));
}

StaticSceneObject::~StaticSceneObject()
{
}

void StaticSceneObject::unSerialize ( void* /*p_memoryBlock*/ )
{
}

void StaticSceneObject::serialize ( void* /*p_memoryBlock*/ )
{
}

void StaticSceneObject::initialize()
{
}

void StaticSceneObject::loadInitialStates()
{
}

bool StaticSceneObject::configure(const std::string /*ConfigFile*/)
{
    return false;
}

std::shared_ptr<SceneObject> StaticSceneObject::clone()
{
    return safeDownCast<SceneObject>();
}

void StaticSceneObject::printInfo() const
{
    std::cout << "\t-------------------------------------\n";
    std::cout << "\t Name        : " << this->getName() << std::endl;
    std::cout << "\t-------------------------------------\n";
}
