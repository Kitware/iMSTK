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
//----------------------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//----------------------------------------------------------------------------------------

#include "Collision/MeshCollisionModel.h"
#include "Collision/SurfaceTree.h"
#include "Core/CollisionConfig.h"

//----------------------------------------------------------------------------------------
MeshCollisionModel::MeshCollisionModel()
{
    this->collisionGroup = std::make_shared<CollisionGroup>();
}

//----------------------------------------------------------------------------------------
MeshCollisionModel::~MeshCollisionModel()
{

}

//----------------------------------------------------------------------------------------
void MeshCollisionModel::setMesh(std::shared_ptr<SurfaceMesh> modelMesh)
{
    this->setModelMesh(modelMesh);
    this->aabbTree.reset();
    this->initAABBTree(1);
}

//----------------------------------------------------------------------------------------
void MeshCollisionModel::loadTriangleMesh(const std::string& meshName)
{
    this->load(meshName);

    this->initAABBTree(1);
}

//----------------------------------------------------------------------------------------
std::shared_ptr< MeshCollisionModel::AABBTreeType >
MeshCollisionModel::getAABBTree()
{
    return this->aabbTree;
}

//----------------------------------------------------------------------------------------
void MeshCollisionModel::setAABBTree(
    std::shared_ptr<MeshCollisionModel::AABBTreeType> modelAabbTree)
{
    this->aabbTree.reset();
    this->aabbTree = modelAabbTree;
}

//----------------------------------------------------------------------------------------
void MeshCollisionModel::initAABBTree(const int& numLevels)
{
    this->aabbTree = std::make_shared<AABBTreeType>(shared_from_this(),
                                                    numLevels);
    this->aabbTree->initStructure();
}

//----------------------------------------------------------------------------------------
const core::Vec3d& MeshCollisionModel::getSurfaceNormal(size_t i) const
{
    auto surfaceMesh = std::static_pointer_cast<SurfaceMesh>(this->mesh);
    return surfaceMesh->getTriangleNormal(i);
}

//----------------------------------------------------------------------------------------
std::array<core::Vec3d,3>
MeshCollisionModel::getElementPositions(size_t i) const
{
    auto surfaceMesh = std::static_pointer_cast<SurfaceMesh>(this->mesh);
    return std::move(surfaceMesh->getTriangleVertices(i));
}

//----------------------------------------------------------------------------------------
void MeshCollisionModel::setBoundingBox(const Eigen::AlignedBox3d& box)
{
    this->aabb = box;
}

//----------------------------------------------------------------------------------------
const Eigen::AlignedBox3d& MeshCollisionModel::getBoundingBox() const
{
    return this->aabb;
}

//----------------------------------------------------------------------------------------
void MeshCollisionModel::computeBoundingBoxes()
{
    auto const &vertices = mesh->getVertices();
    triangleBoundingBoxArray.reserve(this->mesh->getTriangles().size());

    for(auto &t : this->mesh->getTriangles())
    {
        triangleBoundingBoxArray.push_back(Eigen::AlignedBox3d());
        Eigen::AlignedBox3d &box = triangleBoundingBoxArray.back();
        box.extend(vertices[t[0]]);
        box.extend(vertices[t[1]]);
        box.extend(vertices[t[2]]);
        aabb.extend(box);
    }
}

//----------------------------------------------------------------------------------------
const Eigen::AlignedBox3d& MeshCollisionModel::getAabb(size_t i) const
{
    return this->triangleBoundingBoxArray[i];
}

//----------------------------------------------------------------------------------------
const Eigen::AlignedBox3d& MeshCollisionModel::getAabb() const
{
    return this->aabb;
}

//----------------------------------------------------------------------------------------
std::shared_ptr< CollisionGroup >& MeshCollisionModel::getCollisionGroup()
{
    return this->collisionGroup;
}
