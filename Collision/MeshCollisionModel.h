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

#ifndef COLLISION_MESHCOLLISIONMODEL_H
#define COLLISION_MESHCOLLISIONMODEL_H

// STL includes
#include <memory>

// iMSTK includes
#include "Mesh/SurfaceMesh.h"
#include "Collision/OctreeCell.h"
#include "Geometry/MeshModel.h"


class CollisionGroup;
template<typename T>
class SurfaceTree;

///
/// @brief Mesh representation of a model.
/// Used for collision detection algorithms.
/// This class facilitates collision detection between a mesh and another
/// model.
///
/// @see MeshToMeshCollision
///
class MeshCollisionModel : public MeshModel, public std::enable_shared_from_this<MeshCollisionModel>
{
public:
    using AABBNodeType = OctreeCell;
    using AABBTreeType = SurfaceTree<AABBNodeType>;
    using NodePairType = std::pair<std::shared_ptr<AABBNodeType>,std::shared_ptr<AABBNodeType>>;

public:
    MeshCollisionModel();
    ~MeshCollisionModel();

    ///
    /// @brief Set internal mesh data structure
    ///
    void setMesh(std::shared_ptr<SurfaceMesh> modelMesh);

    ///
    /// @brief Returns pointer to axis aligned bounding box hierarchy
    ///
    std::shared_ptr<AABBTreeType> getAABBTree();

    ///
    /// @brief Loads a triangular mesh and stores it.
    ///
    void loadTriangleMesh(const std::string &meshName);

    ///
    /// @brief Set internal AABB tree
    ///
    void setAABBTree(std::shared_ptr<AABBTreeType> modelAabbTree);

    ///
    /// @brief Initialize AABB tree data structure
    ///
    void initAABBTree(const int &numLevels = 6);

    ///
    /// @brief Returns normal vectors for triangles on mesh surface
    ///
    const core::Vec3d& getSurfaceNormal(size_t i) const;

    ///
    /// @brief Returns array of vertices for triangle on surface
    ///
    std::array<core::Vec3d,3> getElementPositions(size_t i) const;

    ///
    /// \brief Set/get bounding box
    ///
    void setBoundingBox(const Eigen::AlignedBox3d &box);
    const Eigen::AlignedBox3d &getBoundingBox() const;

    ///
    /// \brief Compute bounding boxes for mesh.
    ///
    void computeBoundingBoxes();

    ///
    /// \brief Compute bounding boxes for mesh.
    ///
    void updateBoundingBoxes()
    {
        triangleBoundingBoxArray.clear();
        this->computeBoundingBoxes();
    }

    ///
    /// \brief Bounding boxes accessors
    ///
    const Eigen::AlignedBox3d &getAabb(size_t i) const;
    const Eigen::AlignedBox3d &getAabb() const;

    ///
    /// \brief Return the collision group this mesh belongs to.
    ///
    std::shared_ptr<CollisionGroup> &getCollisionGroup();

private:
    std::shared_ptr<AABBTreeType> aabbTree; // Bounding volume hierarchy
    Eigen::AlignedBox3d aabb; // Axis aligned boundig box for the etire model
    std::vector<Eigen::AlignedBox3d> triangleBoundingBoxArray;

    // Collision group this model belongs to.
    std::shared_ptr<CollisionGroup> collisionGroup;
};

#endif // COLLISION_MESHMODEL_H
