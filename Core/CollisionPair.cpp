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

#include "Core/CollisionPair.h"

CollisionPair::CollisionPair() {}
CollisionPair::~CollisionPair() {}

void CollisionPair::setModels(const std::shared_ptr< ModelRepresentation >& first,
                                const std::shared_ptr< ModelRepresentation >& second)
{
    this->modelRepresentations.first = first;
    this->modelRepresentations.second = second;
}

void CollisionPair::addContact(const double& penetrationDepth, const core::Vec3d& vert, const int index, const core::Vec3d& contactNornmal)
{
    auto contact = std::make_shared<Contact>(penetrationDepth, vert, index, contactNornmal);
    //std::shared_ptr<Contact> contact(new Contact(penetrationDepth, vert, index, contactNornmal));
    this->contacts.emplace_back(contact);
}

const std::pair< std::shared_ptr< ModelRepresentation >, std::shared_ptr< ModelRepresentation > >&
CollisionPair::getModels() const
{
    return modelRepresentations;
}

void CollisionPair::clearContacts()
{
    contacts.clear();
}

int CollisionPair::getNumberOfContacts()
{
    return contacts.size();
}

std::shared_ptr< ModelRepresentation > CollisionPair::getFirst()
{
    return this->modelRepresentations.first;
}

std::shared_ptr< ModelRepresentation > CollisionPair::getSecond()
{
    return this->modelRepresentations.second;
}

bool CollisionPair::hasContacts()
{
    return !this->contacts.empty();
}

std::vector< std::shared_ptr< Contact > >& CollisionPair::getContacts()
{
    return contacts;
}

const std::vector< std::shared_ptr< Contact > >& CollisionPair::getContacts() const
{
    return contacts;
}

void CollisionPair::printCollisionPairs()
{
    std::cout << "# Contacts: " << this->contacts.size() << std::endl;
    for (size_t i = 0; i < this->contacts.size(); i++)
    {
        std::cout << "Contact no: " << i << std::endl;
        this->contacts[0]->printInfo();
    }

}
