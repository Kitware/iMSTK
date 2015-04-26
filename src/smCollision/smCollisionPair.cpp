/*
 * // This file is part of the SimMedTK project.
 * // Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
 * //                        Rensselaer Polytechnic Institute
 * //
 * // Licensed under the Apache License, Version 2.0 (the "License");
 * // you may not use this file except in compliance with the License.
 * // You may obtain a copy of the License at
 * //
 * //     http://www.apache.org/licenses/LICENSE-2.0
 * //
 * // Unless required by applicable law or agreed to in writing, software
 * // distributed under the License is distributed on an "AS IS" BASIS,
 * // WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * // See the License for the specific language governing permissions and
 * // limitations under the License.
 * //
 * //---------------------------------------------------------------------------
 * //
 * // Authors:
 * //
 * // Contact:
 * //---------------------------------------------------------------------------
 */

#include "smCollision/smCollisionPair.h"

smCollisionPair::smCollisionPair() {}
smCollisionPair::~smCollisionPair() {}

void smCollisionPair::setModels(const std::shared_ptr< smModelRepresentation >& first,
                                const std::shared_ptr< smModelRepresentation >& second)
{
    this->modelRepresentations.first = first;
    this->modelRepresentations.second = second;
}
void smCollisionPair::addContact(const double& penetrationDepth, const smVec3f& contactPoint, const smVec3f& contactNornmal)
{
    auto contact = std::make_shared<smContact>(penetrationDepth, contactPoint, contactNornmal);
    this->contacts.emplace_back(contact);
}
const std::pair< std::shared_ptr< smModelRepresentation >, std::shared_ptr< smModelRepresentation > >&
smCollisionPair::getModels() const
{
    return modelRepresentations;
}
void smCollisionPair::clearContacts()
{
    contacts.clear();
}
std::shared_ptr< smModelRepresentation > smCollisionPair::getFirst()
{
    return this->modelRepresentations.first;
}
std::shared_ptr< smModelRepresentation > smCollisionPair::getSecond()
{
    return this->modelRepresentations.second;
}
bool smCollisionPair::hasContacts()
{
    return !this->contacts.empty();
}
std::vector< std::shared_ptr< smContact > >& smCollisionPair::getContacts()
{
    return contacts;
}
const std::vector< std::shared_ptr< smContact > >& smCollisionPair::getContacts() const
{
    return contacts;
}
