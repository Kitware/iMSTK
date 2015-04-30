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

#ifndef AABBNODE_H
#define AABBNODE_H

// STL includes
#include <memory>
#include <array>

#include <Eigen/Geometry>

class AabbNode
{
public:
    /// Constructor
    AabbNode();

    /// Destructor
    ~AabbNode();

    /// Assigment operator, it overrides this data
    AabbNode& operator=( const AabbNode& other )
    {
        this->aabb = other.getAabb();
        this->parent = other.getParent();
        this->children.clear();

        for(auto &c : other.getChildren())
            this->children.emplace_back(c);
    }

    /// Comparison operator
    bool operator== ( const AabbNode& other ) const
    {
        return this->parent == other.getParent()
            && ;
    }

    void setAabb(const Eigen::AlignedBox<double, 3> &box)
    {
        this->aabb = box;
    }

    const Eigen::AlignedBox<double, 3> &getAabb() const
    {
        return this->aabb;
    }

    void setParent(const std::shared_prt<AabbNode> nodeParent)
    {
        this->parent = nodeParent;
    }

    const std::shared_prt<AabbNode> &getParent()
    {
        return this->parent;
    }

    void setChildren(const std::vector<std::shared_prt<AabbNode>> &nodeChildren)
    {
        this->children = nodeChildren;
    }

    const std::vector<std::shared_prt<AabbNode>> &getChildren()
    {
        return this->children;
    }

    bool intersect(std::shared_ptr<AabbNode> other)
    {
        const Eigen::AlignedBox<double, 3> &otherBox = other->getAabb();
        return (otherBox.min().array() > aabb.max().array()
            || aabb.min().array() > otherBox.max().array()).all();
    }

    void subDivide();

    void updateAabb()
    {
        this->aabb.setNull();
        for(const auto &i : this->data)
        {
            this->aabb.extend(i.first);
        }
    }

private:
    std::shared_prt<AabbNode> parent;                   // parent node
    std::array<std::shared_prt<AabbNode>,2> children;   // children nodes
    Eigen::AlignedBox<double, 3> aabb;                  // axis aligned bounding box

    std::vector<std::pair<AabbNode,size_t>> data;       // triangle data
};

#endif // AABBNODE_H
