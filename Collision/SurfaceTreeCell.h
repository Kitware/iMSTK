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

#ifndef COLLISION_SURFACETREECELL_H
#define COLLISION_SURFACETREECELL_H

// STD includes
#include<memory>
#include<vector>
#include<set>

// iMSTK includes
#include "Core/Config.h"
#include "Core/Vector.h"

namespace imstk {

/// \brief contains the cell of the surface tree structure
template <typename Derived>
class SurfaceTreeCell
{
public:
    /// \brief constructor
    SurfaceTreeCell()
    {
        empty = true;
        isLeaf = false;
        level = 0;
    }

    /// \brief returns a pointer to the child object
    inline Derived *derived()
    {
        return static_cast<Derived*>(this);
    }

    /// \brief subdivide the cell of surface tree structure
    template<typename ArrayType>
    inline void subDivide(const int divisionPerAxis, ArrayType &cells)
    {
        derived()->subDivide(divisionPerAxis,cells);
    };

    /// \brief !! copy the cell shape
    inline void copyShape(Derived &cell)
    {
        derived()->copyShape(cell);
    }

    /// \brief checks if the cell collided with a triangle primitive
    inline bool isCollidedWithTri(const Vec3d &v0, const Vec3d &v1, const Vec3d &v2)
    {
        return derived()->isCollidedWithTri(v0,v1,v2);
    }

    /// \brief checks if the cell contains the point primitive
    inline bool isCollidedWithPoint(const Vec3d &point)
    {
        return derived()->isCollidedWithPoint(point);
    }

    /// \brief !! expand the cell of the surface tree structure
    inline void expand(const float expansion)
    {
        derived()->expand(expansion);
    }

    /// \brief set the center of the cell of surface tree
    inline void setCenter(const Vec3d &center)
    {
        derived()->setCenter(center);
    }

    /// \brief set the length of the cell of surface tree
    inline void setLength(const float length)
    {
        derived()->setLength(length);
    }

    /// \brief get the center of the cell of surface tree
    inline Vec3d &getCenter()
    {
        return derived()->getCenter();
    }

    /// \brief set the length of the cell of surface tree
    inline float &getLength()
    {
        return derived()->getLength();
    }

    /// \brief get the center of the cell of surface tree
    inline const Vec3d &getCenter()  const
    {
        return derived()->getCenter();
    }

    /// \brief set the length of the cell of surface tree
    inline const float &getLength() const
    {
        return derived()->getLength();
    }

    template<typename AABB>
    inline void addTriangleData(const AABB &aabb, size_t index)
    {
        return derived()->addTriangleData(aabb,index);
    }

    inline const bool &isEmpty() const
    {
        return empty;
    }

    inline void setIsEmpty(const bool &isEmpty)
    {
        empty = isEmpty;
    }

    inline const bool &getIsLeaf() const
    {
        return isLeaf;
    }

    inline void setIsLeaf(const bool &leaf)
    {
        isLeaf = leaf;
    }

    inline const std::set<int> &getVerticesIndices() const
    {
        return verticesIndices;
    }

    inline void setVerticesIndices(const std::set<int> &indices)
    {
        verticesIndices = indices;
    }

    inline void addVertexIndex(const int &index)
    {
        verticesIndices.insert(index);
    }

    inline void setLevel(const int &l)
    {
        level = l;
    }

    inline const int &getLevel() const
    {
        return level;
    }

    inline void setWeights(const std::vector<float> &w)
    {
        weights = w;
    }

    inline const std::vector<float> &getWeights() const
    {
        return weights;
    }

    inline std::vector<float> &getWeights()
    {
        return weights;
    }

    inline void addWeight(const float &w)
    {
        weights.emplace_back(w);
    }

    inline const float &getWeight(const float &w) const
    {
        return weights.at(w);
    }

    inline std::shared_ptr<Derived> getChildNode(size_t i)
    {
        return derived()->getChildNode(i);
    }

    inline void setChildNode(size_t i, std::shared_ptr<Derived> node)
    {
        derived()->setChildNode(node);
    }

    inline std::shared_ptr<Derived> getParentNode()
    {
        derived()->getParentNode();
    }

    inline void setParentNode(std::shared_ptr<Derived> parent)
    {
        derived()->setParentNode(parent);
    }

private:
    bool empty; ///< !!
    bool isLeaf; ///< !!
    int level; ///< level in the tree
    std::vector<float> weights; ///< !!
    std::set<int> verticesIndices; ///< indices of vertices
};

}

#endif
