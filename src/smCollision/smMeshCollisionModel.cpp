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

#include "smCollision/smMeshCollisionModel.h"

smMeshCollisionModel::smMeshCollisionModel()
{

}

smMeshCollisionModel::~smMeshCollisionModel()
{

}
void smMeshCollisionModel::setMesh(std::shared_ptr<smMesh> modelMesh)
{
    this->setModelMesh(modelMesh);
    this->aabbTree.reset();
    this->initAABBTree(6);
}
void smMeshCollisionModel::loadTriangleMesh(const std::string& meshName, const smMeshFileType &type)
{
    this->load(meshName,type);

    this->initAABBTree(6);
}
std::shared_ptr< smMeshCollisionModel::AABBTreeType > smMeshCollisionModel::getAABBTree()
{
    return this->aabbTree;
}

void smMeshCollisionModel::setAABBTree(std::shared_ptr<smMeshCollisionModel::AABBTreeType> modelAabbTree)
{
    this->aabbTree.reset();
    this->aabbTree = modelAabbTree;
}
void smMeshCollisionModel::initAABBTree(const int& numLevels)
{
    this->aabbTree = std::make_shared<AABBTreeType>(std::static_pointer_cast<smSurfaceMesh>(this->mesh), numLevels);
    this->aabbTree->initStructure();
}
