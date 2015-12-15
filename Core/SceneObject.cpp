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

SceneObject::SceneObject()
{
    type = core::ClassType::Unknown;
    isActive = true;
    objectSim = nullptr;
    customRender = nullptr;
    //SDK::getInstance()->registerSceneObject(safeDownCast<SceneObject>());
    flags.isViewerInit = false;
    flags.isSimulatorInit = false;
    name = "SceneObject" + std::to_string(this->getUniqueId()->getId());
    hasContactForces = false;
}

SceneObject::~SceneObject()
{
}

void SceneObject::freeze()
{
    this->isActive = false;
}

void SceneObject::activate()
{
    this->isActive = true;
}

void SceneObject::attachObjectSimulator(std::shared_ptr<ObjectSimulator> p_objectSim)
{
    p_objectSim->addObject(safeDownCast<SceneObject>());
}

void SceneObject::attachCustomRenderer(std::shared_ptr<CustomRenderer> p_customeRenderer)
{
    customRender = p_customeRenderer;
}

void SceneObject::releaseObjectSimulator()
{
    objectSim->removeObject(safeDownCast<SceneObject>());
    objectSim = nullptr;
}

void SceneObject::releaseCustomeRenderer()
{
    customRender = nullptr;
}

std::shared_ptr<ObjectSimulator> SceneObject::getObjectSimulator()
{
    return objectSim;
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

std::shared_ptr<Core::BaseMesh> SceneObject::getModel(const int id, const int type)
{
    switch (type)
    {
    case core::ModelType::visual:
        return getVisualModel(id);

    case core::ModelType::collision:
        return getCollisionModel(id);

    case core::ModelType::physics:
        return getPhysicsModel(id);

    default:
        std::cout << "Error: Type of model supplied is not identified! \n";
        return nullptr;
    }
}

std::shared_ptr<Core::BaseMesh> SceneObject::getVisualModel(const int id)
{
    return this->visualModels[id];
}

std::shared_ptr<Core::BaseMesh> SceneObject::getCollisionModel(const int id)
{
    return this->collisionModels[id];
}

std::shared_ptr<Core::BaseMesh> SceneObject::getPhysicsModel(const int id)
{
    return this->physicsModels[id];
}

bool SceneObject::findMesh(std::shared_ptr<Core::BaseMesh>& m, const int type)
{
    switch (type)
    {
    case core::ModelType::visual:
        for (auto it = this->visualModels.begin(); it != this->visualModels.end(); ++it)
        {
            if (*it == m)
            {
                return true;
            }
        }

    case core::ModelType::collision:
        for (auto it = this->collisionModels.begin(); it != this->collisionModels.end(); ++it)
        {
            if (*it == m)
            {
                return true;
            }
        }

    case core::ModelType::physics:
        for (auto it = this->physicsModels.begin(); it != this->physicsModels.end(); ++it)
        {
            if (*it == m)
            {
                return true;
            }
        }

    default:
        std::cout << "Error: Type of model supplied is not identified! \n";
        return false;
    }
}

bool SceneObject::findMesh(std::shared_ptr<Core::BaseMesh>& m)
{
    for (auto it = this->visualModels.begin(); it != this->visualModels.end(); ++it)
    {
        if (*it == m)
        {
            return true;
        }
    }

    for (auto it = this->collisionModels.begin(); it != this->collisionModels.end(); ++it)
    {
        if (*it == m)
        {
            return true;
        }
    }

    for (auto it = this->physicsModels.begin(); it != this->physicsModels.end(); ++it)
    {
        if (*it == m)
        {
            return true;
        }
    }

    return false;
}

int SceneObject::getNumDof() const
{
    return numDof;
}

bool SceneObject::SceneObject::addMap(std::shared_ptr<MeshMap>& map)
{
    if (map->getMasterMesh() != nullptr && this->findMesh(map->getMasterMesh()) &&
        map->getSlaveMesh() != nullptr && this->findMesh(map->getSlaveMesh()))
    {
        this->maps.push_back(map);
        return true;
    }
    else
    {
        std::cout << "The map supplied is either not complete Or the meshes \
            that are being mapped are not part of scene object!\n";

        return false;
    }
}

void SceneObject::applyMap(int mapID)
{
    this->maps[mapID]->apply();
}

void SceneObject::applyAllMaps()
{
    for (auto it = this->maps.begin(); it != this->maps.end(); ++it)
    {
        (*it)->apply();
    }
}

bool SceneObject::addVisualModel(std::shared_ptr<Core::BaseMesh>& model)
{
    if (model != nullptr && this->findMesh(model, core::ModelType::visual))
    {
        this->visualModels.push_back(model);
        return 1;
    }
    else
    {
        return 0;
    }
}

bool SceneObject::addCollisionModel(std::shared_ptr<Core::BaseMesh>& model)
{
    if (model != nullptr && this->findMesh(model, core::ModelType::collision))
    {
        this->collisionModels.push_back(model);
        return 1;
    }
    else
    {
        return 0;
    }
}

bool SceneObject::addPhysicsModel(std::shared_ptr<Core::BaseMesh>& model)
{
    if (model != nullptr && this->findMesh(model, core::ModelType::physics))
    {
        this->physicsModels.push_back(model);
        return 1;
    }
    else
    {
        return 0;
    }
}

bool SceneObject::addModel(std::shared_ptr<Core::BaseMesh>& model, int type)
{
    bool added = 0;
    if (model != nullptr)
    {
        if (core::ModelType::visual & type)
        {
            addVisualModel(model);
            added = true;
        }

        if (core::ModelType::collision & type)
        {
            addCollisionModel(model);
            added = true;
        }

        if (core::ModelType::physics & type)
        {
            addPhysicsModel(model);
            added = true;
        }

        return added;
    }
    else
    {
        return 0;
    }
}

std::shared_ptr<Core::BaseMesh> SceneObject::getVisualModel(int id)
{
    return this->visualModels[id];
}

std::shared_ptr<Core::BaseMesh> SceneObject::getCollisionModel(int id)
{
    return this->collisionModels[id];
}

std::shared_ptr<Core::BaseMesh> SceneObject::getPhysicsModel(int id)
{
    return this->physicsModels[id];
}

bool SceneObject::findMesh(std::shared_ptr<Core::BaseMesh>& m, int type)
{
    switch (type)
    {
    case core::ModelType::visual:
        for (auto it = this->visualModels.begin(); it != this->visualModels.end(); ++it)
        {
            if (*it == m)
            {
                return true;
            }
        }

    case core::ModelType::collision:
        for (auto it = this->collisionModels.begin(); it != this->collisionModels.end(); ++it)
        {
            if (*it == m)
            {
                return true;
            }
        }

    case core::ModelType::physics:
        for (auto it = this->physicsModels.begin(); it != this->physicsModels.end(); ++it)
        {
            if (*it == m)
            {
                return true;
            }
        }

    default:
        std::cout << "Error: Type of model supplied is not identified! \n";
        return false;
    }
}

bool SceneObject::findMesh(std::shared_ptr<Core::BaseMesh>& m)
{
    for (auto it = this->visualModels.begin(); it != this->visualModels.end(); ++it)
    {
        if (*it == m)
        {
            return true;
        }
    }

    for (auto it = this->collisionModels.begin(); it != this->collisionModels.end(); ++it)
    {
        if (*it == m)
        {
            return true;
        }
    }

    for (auto it = this->physicsModels.begin(); it != this->physicsModels.end(); ++it)
    {
        if (*it == m)
        {
            return true;
        }
    }

    return false;
}

bool SceneObject::SceneObject::addMap(std::shared_ptr<MeshMap>& map)
{
    if (map->getMasterMesh() != nullptr && this->findMesh(map->getMasterMesh()) &&
        map->getSlaveMesh() != nullptr && this->findMesh(map->getSlaveMesh()))
    {
        this->maps.push_back(map);
        return true;
    }
    else
    {
        std::cout << "The map supplied is either not complete Or the meshes \
                        that are being mapped are not part of scene object!\n";

        return false;
    }
}

void SceneObject::applyMap(int mapID)
{
    this->maps[mapID]->apply();
}

void SceneObject::applyAllMaps()
{
    for (auto it = this->maps.begin(); it != this->maps.end(); ++it)
    {
        (*it)->apply();
    }
}

bool SceneObject::addVisualModel(std::shared_ptr<Core::BaseMesh>& model)
{
    if (model != nullptr && this->findMesh(model, core::ModelType::visual))
    {
        this->visualModels.push_back(model);
        return 1;
    }
    else
    {
        return 0;
    }
}

bool SceneObject::addCollisionModel(std::shared_ptr<Core::BaseMesh>& model)
{
    if (model != nullptr && this->findMesh(model, core::ModelType::collision))
    {
        this->collisionModels.push_back(model);
        return 1;
    }
    else
    {
        return 0;
    }
}

bool SceneObject::addPhysicsModel(std::shared_ptr<Core::BaseMesh>& model)
{
    if (model != nullptr && this->findMesh(model, core::ModelType::physics))
    {
        this->physicsModels.push_back(model);
        return 1;
    }
    else
    {
        return 0;
    }
}

bool SceneObject::addModel(std::shared_ptr<Core::BaseMesh>& model, int type)
{
    bool added = 0;
    if (model != nullptr)
    {
        if (core::ModelType::visual & type)
        {
            addVisualModel(model);
            added = true;
        }

        if (core::ModelType::collision & type)
        {
            addCollisionModel(model);
            added = true;
        }

        if (core::ModelType::physics & type)
        {
            addPhysicsModel(model);
            added = true;
        }

        return added;
    }
    else
    {
        return 0;
    }
}

void updateCollisionModels()
{
}

void updateVisualModels()
{
}
