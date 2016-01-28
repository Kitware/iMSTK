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

#include "SceneModels/StaticSceneObject.h"
#include "Core/Factory.h"
#include "Core/RenderDelegate.h"
#include "Geometry/MeshModel.h"

namespace imstk {

StaticSceneObject::StaticSceneObject(std::shared_ptr<ErrorLog> /*p_log*/)
{
    type = ClassType::StaticSceneObject;

    name = "Static_SceneObject_" + std::to_string(this->getUniqueId()->getId());
}

//---------------------------------------------------------------------------
void StaticSceneObject::initialize()
{
    this->objectSim = nullptr;
    this->flags.isSimulatorInit = false;
}

//---------------------------------------------------------------------------
void StaticSceneObject::printInfo() const
{
    std::cout << "\t-------------------------------------\n";
    std::cout << "\t Name        : " << this->getName() << std::endl;
    std::cout << "\t-------------------------------------\n";
}

//---------------------------------------------------------------------------
void StaticSceneObject::loadInitialStates()
{
    if(fileName.empty())
    {
        // TODO: log this
        return;
    }

    auto model = std::make_shared<MeshModel>();

    model->load(fileName);

    this->setModel(model);
}
void StaticSceneObject::loadMesh(const std::string &file)
{
    this->fileName = file;
    this->loadInitialStates();
}

}
