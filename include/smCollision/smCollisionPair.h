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


#ifndef SMCOLLISIONPAIR_H
#define SMCOLLISIONPAIR_H

// SimMedTK includes
#include "smUtilities/smVector.h"

// STL includes
#include <memory>
#include <vector>

class smModelRepresentation;

///
/// \brief Contact point representation
///
class smContact
{
public:
    smContact ( const double penetrationDepth,
                const smVec3d& p,
                const int ind,
                const smVec3d& contactNornmal
				) : depth(penetrationDepth), point(p), index(ind), normal(contactNornmal){}

    double depth;
    smVec3d point;
    smVec3d normal;
    int index;
};

///
/// \brief Contains pair of potential collision models
///   This class also stores contacts between those models.
///
class smCollisionPair
{
public:
    smCollisionPair();
    ~smCollisionPair();

    ///
    /// @brief Set the pair of collision models
    ///
    void setModels(const std::shared_ptr<smModelRepresentation>& first,
                   const std::shared_ptr<smModelRepresentation>& second );

    ///
    /// @brief Get the pair of collision models
    ///
    const std::pair<std::shared_ptr<smModelRepresentation>, std::shared_ptr<smModelRepresentation>>&
    getModels() const;

    ///
    /// @brief Add contact between the models
    ///
    void addContact( const double& penetrationDepth,
                     const smVec3d& vert,
                     const int index,
                     const smVec3d& contactNornmal);

    ///
    /// @brief Clear contact list
    ///
    void clearContacts();

    ///
    /// @brief Get first model
    ///
    std::shared_ptr<smModelRepresentation> getFirst();

    ///
    /// @brief Get second model
    ///
    std::shared_ptr<smModelRepresentation> getSecond();

    ///
    /// @brief Returns wether the contact container is empty
    ///
    bool hasContacts();

    ///
    /// @brief Returns the number of contacts (= number of contact pairs)
    ///
    int getNumberOfContacts();

    ///
    /// @brief Returns contact array for these two models
    ///
    std::vector<std::shared_ptr<smContact>> &getContacts();
    const std::vector<std::shared_ptr<smContact>> &getContacts() const;

private:
    std::pair<std::shared_ptr<smModelRepresentation>,
        std::shared_ptr<smModelRepresentation>> modelRepresentations; // Models

    std::vector<std::shared_ptr<smContact>> contacts; // List of contacts
};

#endif // SMCOLLISIONPAIR_H
