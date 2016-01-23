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

#ifndef COLLISION_OCTREECELL_H
#define COLLISION_OCTREECELL_H

#include <Eigen/Geometry>
#include <array>
#include <memory>

// iMSTK includes
#include "Collision/SurfaceTreeCell.h"
#include "Core/Geometry.h"

/// \brief cell of an octree
class OctreeCell : public SurfaceTreeCell<OctreeCell>
{
protected:
    using BaseType = SurfaceTreeCell<OctreeCell>;
    using SelfType = OctreeCell;

public:
    OctreeCell();
    ~OctreeCell();

    enum { numberOfSubdivisions = 8 };

    /// \brief get a constant reference to the cell primitive
    const Cube &getCube() const;

    /// \brief get a reference to the cell primitive
    Cube &getCube();

    /// \brief set the primitive
    void setCube(const Cube &other);

    /// \brief get the center of the octree cell
    core::Vec3d &getCenter();

    /// \brief get constant reference to the center of the octree cell
    const core::Vec3d &getCenter() const;

    /// \brief set the center of the octree cell
    void setCenter ( const core::Vec3d &center );

    /// \brief get reference to the side length of the octree cell
    double &getLength ();

    /// \brief get the side length of the octree cell
    const double &getLength() const;

    /// \brief set the octree cell
    void copyShape ( const OctreeCell &cell );

    /// \brief !! expand the cell of the octree structure
    void expand ( const double expandScale );

    /// \brief set the length of the octree cell
    void setLength ( const double length );

    /// \brief check if a triangle is intersecting the octree cell
    bool isCollidedWithTri ( const core::Vec3d &v0, const core::Vec3d &v1, const core::Vec3d &v2 ) const;

    /// \brief check if a point lies inside an octree cell
    bool isCollidedWithPoint ();

    /// \brief subdivide the cells of octree cells
    void subDivide ( const int divisionPerAxis,
                     std::array<OctreeCell,numberOfSubdivisions> &cells );

    const Eigen::AlignedBox3d &getAabb() const
    {
        return aabb;
    }

    void setAabb(const Eigen::AlignedBox3d &newAabb)
    {
        this->aabb = newAabb;
    }

    void getIntersections(const Eigen::AlignedBox3d &aabb, std::vector<size_t> &triangles)
    {
        for(auto &i : data)
        {
            if(!i.first.intersection(aabb).isEmpty())
            {
                triangles.emplace_back(i.second);
            }
        }
    }

    inline void addTriangleData(const Eigen::AlignedBox3d &aabb, size_t index)
    {
        return data.emplace_back(aabb,index);
    }

    std::shared_ptr<OctreeCell> getChildNode(size_t i)
    {
        return childNodes[i];
    }

    const std::array<std::shared_ptr<OctreeCell>,numberOfSubdivisions>
    &getChildNodes() const
    {
        return childNodes;
    }

    void setChildNode(size_t i, std::shared_ptr<OctreeCell> node)
    {
        childNodes[i] = node;
    }

    std::shared_ptr<OctreeCell> getParentNode()
    {
        return parentNode;
    }

    void setParentNode(std::shared_ptr<OctreeCell> parent)
    {
        parentNode = parent;
    }

    void draw() const
    {
//         this->aabb.draw();
//         for(auto &child : childNodes)
//         {
//             if(child)
//             {
//                 child->draw();
//             }
//         }
    }

    void update()
    {
        this->aabb.setEmpty();
        for(auto &box : data)
        {
            this->aabb.extend(box.first);
        }
    }

private:
    Cube cube; ///< cube
    Eigen::AlignedBox3d aabb;

    std::array<std::shared_ptr<OctreeCell>,numberOfSubdivisions> childNodes;
    std::shared_ptr<OctreeCell> parentNode;
    std::vector<std::pair<Eigen::AlignedBox3d,size_t>> data;

};

#endif

