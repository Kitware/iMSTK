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

#include "smCore/smStaticSceneObject.h"

smStaticSceneObject::smStaticSceneObject(std::shared_ptr<smErrorLog> p_log) : smSceneObject()
{
    type = SIMMEDTK_SMSTATICSCENEOBJECT;

    name = "Static_SceneObject_" + std::to_string(this->getUniqueId()->getId());
}

smStaticSceneObject::~smStaticSceneObject()
{
}

void smStaticSceneObject::unSerialize ( void* /*p_memoryBlock*/ )
{
}

void smStaticSceneObject::serialize ( void* /*p_memoryBlock*/ )
{
}

void smStaticSceneObject::initialize()
{
}

void smStaticSceneObject::loadInitialStates()
{
}

bool smStaticSceneObject::configure(smString ConfigFile)
{
    return false;
}

std::shared_ptr<void> smStaticSceneObject::duplicateAtRuntime()
{
    std::shared_ptr<smStaticSceneObject> newSO =
        std::make_shared<smStaticSceneObject>();

    return (std::shared_ptr<void>)newSO;
}

std::shared_ptr<void> smStaticSceneObject::duplicateAtInitialization()
{
    std::shared_ptr<smStaticSceneObject> newSO =
        std::make_shared<smStaticSceneObject>();

    return (std::shared_ptr<void>)newSO;
}

void smStaticSceneObject::draw()
{
    staticModel->draw();
}

std::shared_ptr<smSceneObject> smStaticSceneObject::clone()
{
    return safeDownCast<smSceneObject>();
}

void smStaticSceneObject::setModel(std::shared_ptr<smModelRepresentation> model)
{
    this->staticModel = model;
}

std::shared_ptr<smModelRepresentation> smStaticSceneObject::getModel()
{
    return staticModel;
}

void smStaticSceneObject::printInfo()
{
    std::cout << "\t-------------------------------------\n";
    std::cout << "\t Name        : " << this->getName() << std::endl;
    std::cout << "\t Type        : " << this->getType() << std::endl;
    std::cout << "\t-------------------------------------\n";
}