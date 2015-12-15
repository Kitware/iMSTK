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
#include "Core/Vector.h"

// STL includes
#include <memory>
#include <vector>
#include <iostream>
#include <map>

class Model;

///
/// \brief Contact point representation
///
class Contact
{
public:
    Contact (const double penetrationDepth,
             const core::Vec3d& p,
             const int ind,
             const core::Vec3d& contactNornmal) :
                depth(penetrationDepth),
                point(p),
                normal(contactNornmal),
                index(ind),
                model(nullptr){}
    Contact (std::shared_ptr<Model> m,
             const double penetrationDepth,
             const core::Vec3d& p,
             const int ind,
             const core::Vec3d& contactNornmal) :
                depth(penetrationDepth),
                point(p),
                normal(contactNornmal),
                index(ind),
                model(m){}

    void printInfo()
    {
        std::cout << "\tDepth  : " << depth << std::endl;
        std::cout << "\tIndex  : " << depth << std::endl;
        std::cout << "\tNoramal: (" << normal(0) << ", " << normal(1) << ", " << normal(2) << ")\n";
        std::cout << "\tVertex : (" << point(0) << ", " << point(1) << ", " << point(2) << ")\n\n";
    }

    double depth;
    core::Vec3d point;
    core::Vec3d normal;
    int index;
    std::shared_ptr<Model> model;
};

///
/// \brief Class containing the edge-edge collision data
/// \todo implement this class
///
class EdgeEdgeCollisionData : public CollisionDataBase
{

};

///
/// \brief Class containing the vertex-triangle collision data
/// \todo implement this class
///
class VertexTriangleCollisionData : public CollisionDataBase
{

};

///
/// \brief Contains pair of potential collision models
///   This class also stores contacts between those models.
///
class CollisionPair
{
public:
    CollisionPair();
    ~CollisionPair();

    ///
    /// @brief Set the pair of collision models
    ///
    void setModels(std::shared_ptr<Model> first,
                   std::shared_ptr<Model> second );

    ///
    /// @brief Get the pair of collision models
    ///
    const std::pair<std::shared_ptr<Model>, std::shared_ptr<Model>>&
    getModels() const;

    ///
    /// @brief Add contact between the models
    ///
    void addContact( const double& penetrationDepth,
                     const core::Vec3d& vert,
                     const int index,
                     const core::Vec3d& contactNornmal);

    ///
    /// @brief Add contact between the models
    ///
    void addContact( std::shared_ptr<Model> model,
                     const double& penetrationDepth,
                     const core::Vec3d& vert,
                     const int index,
                     const core::Vec3d& contactNornmal);

    ///
    /// @brief Clear contact list
    ///
    void clearContacts();

    ///
    /// @brief Get first model
    ///
    std::shared_ptr<Model> getFirst();

    ///
    /// @brief Get second model
    ///
    std::shared_ptr<Model> getSecond();

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
    std::vector<std::shared_ptr<Contact>> &getContacts();
    const std::vector<std::shared_ptr<Contact>> &getContacts() const;

    ///
    /// @brief Returns contact array for a particular model
    ///
    std::vector<std::shared_ptr<Contact>> &getContacts(const std::shared_ptr<Model> &model);
    const std::vector<std::shared_ptr<Contact>> &getContacts(const std::shared_ptr<Model> &model) const;

    ///
    /// @brief Returns contact array for these two models
    ///
    void printCollisionPairs();

private:
    std::pair<std::shared_ptr<Model>,
        std::shared_ptr<Model>> modelRepresentations; //!< Models
    std::vector<std::shared_ptr<Contact>> contacts; //!< List of contacts
    std::map<std::shared_ptr<Model>,
               std::vector<std::shared_ptr<Contact>>> modelContacts; //!< Contacts per model
};

#endif // SMCOLLISIONPAIR_H
