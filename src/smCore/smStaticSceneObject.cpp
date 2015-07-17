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

#include "smStaticSceneObject.h"
#include "smFactory.h"

smStaticSceneObject::smStaticSceneObject(std::shared_ptr<smErrorLog> p_log) : smSceneObject()
{
    type = SIMMEDTK_SMSTATICSCENEOBJECT;

    name = "Static_SceneObject_" + std::to_string(this->getUniqueId()->getId());

    this->setRenderDelegate(
      smFactory<smRenderDelegate>::createSubclass(
        "smRenderDelegate","smStaticSceneObjectRenderDelegate"));
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

bool smStaticSceneObject::configure(const smString ConfigFile)
{
    return false;
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

void smStaticSceneObject::printInfo() const
{
    std::cout << "\t-------------------------------------\n";
    std::cout << "\t Name        : " << this->getName() << std::endl;
    std::cout << "\t Type        : " << this->getType() << std::endl;
    std::cout << "\t-------------------------------------\n";
}
