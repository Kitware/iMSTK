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
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contacts:
//---------------------------------------------------------------------------

#include "Core/CollisionManager.h"
#include "Core/Model.h"

void CollisionManager::setModels(std::shared_ptr<Model> first,
                                 std::shared_ptr<Model> second)
{
    this->modelRepresentations.first = first;
    this->modelRepresentations.second = second;
    this->modelContacts[first].clear();
    this->modelContacts[second].clear();
}

//---------------------------------------------------------------------------
void CollisionManager::addContact(const double& penetrationDepth,
                                  const core::Vec3d& vert,
                                  const int index,
                                  const core::Vec3d& contactNornmal)
{
    auto contact = std::make_shared<PenetrationDepthCollisionData>(penetrationDepth,
                   vert,
                   index,
                   contactNornmal);
    this->contacts.emplace_back(contact);
}

//---------------------------------------------------------------------------
void CollisionManager::addContact(std::shared_ptr<Model> model,
                                  const double& penetrationDepth,
                                  const core::Vec3d& vert,
                                  const int index,
                                  const core::Vec3d& contactNornmal)
{
    auto contact = std::make_shared<PenetrationDepthCollisionData>(penetrationDepth,
                   vert,
                   index,
                   contactNornmal);
    this->contacts.emplace_back(contact);
    this->modelContacts[model].emplace_back(contact);
}

//---------------------------------------------------------------------------
const std::pair< std::shared_ptr<Model>, std::shared_ptr<Model> >&
CollisionManager::getModels() const
{
    return this->modelRepresentations;
}

//---------------------------------------------------------------------------
void CollisionManager::clearContacts()
{
    this->contacts.clear();
    this->modelContacts[this->modelRepresentations.first].clear();
    this->modelContacts[this->modelRepresentations.second].clear();
}

//---------------------------------------------------------------------------
int CollisionManager::getNumberOfContacts()
{
    return this->contacts.size();
}

//---------------------------------------------------------------------------
std::shared_ptr<Model> CollisionManager::getFirst()
{
    return this->modelRepresentations.first;
}

//---------------------------------------------------------------------------
std::shared_ptr<Model> CollisionManager::getSecond()
{
    return this->modelRepresentations.second;
}

//---------------------------------------------------------------------------
bool CollisionManager::hasContacts()
{
    return !this->contacts.empty();
}

//---------------------------------------------------------------------------
std::vector<std::shared_ptr<PenetrationDepthCollisionData>>& CollisionManager::getContacts()
{
    return this->contacts;
}

//---------------------------------------------------------------------------
const std::vector<std::shared_ptr<PenetrationDepthCollisionData>>&
CollisionManager::getContacts() const
{
    return this->contacts;
}

//---------------------------------------------------------------------------
std::vector<std::shared_ptr<PenetrationDepthCollisionData>>&
CollisionManager::getContacts(const std::shared_ptr<Model> &model)
{
    return this->modelContacts.at(model);
}

//---------------------------------------------------------------------------
const std::vector<std::shared_ptr<PenetrationDepthCollisionData>>&
CollisionManager::getContacts(const std::shared_ptr<Model> &model) const
{
    return this->modelContacts.at(model);
}

//---------------------------------------------------------------------------
void CollisionManager::printCollisionPairs()
{
    std::cout << "# Contacts: " << this->contacts.size() << std::endl;
    for (size_t i = 0; i < this->contacts.size(); i++)
    {
        std::cout << "PenetrationDepthCollisionData no: " << i << std::endl;
        this->contacts[0]->printCollisionPair();
    }

}
