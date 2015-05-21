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

#include <bandit/bandit.h>

#include "smCollision/smMeshToMeshCollision.h"
#include "smCollision/smMeshCollisionModel.h"
#include "smCollision/smCollisionPair.h"
#include "smUtilities/smMakeUnique.h"
#include "smMesh/smSurfaceMesh.h"

using namespace bandit;

std::shared_ptr<smMeshCollisionModel> getModel(const smStdVector3d &vertices)
{
    std::shared_ptr<smMeshCollisionModel> model = std::make_shared<smMeshCollisionModel>();
    std::shared_ptr<smMesh> mesh = std::make_shared<smSurfaceMesh>();
    model->setMesh(mesh);

    // Add two triangles to the data structure
    mesh->initVertexArrays(3);
    mesh->initTriangleArrays(1);

    mesh->vertices.push_back(vertices[0]);
    mesh->vertices.push_back(vertices[1]);
    mesh->vertices.push_back(vertices[2]);

    mesh->triangles[0].vert[0] = 0;
    mesh->triangles[0].vert[1] = 1;
    mesh->triangles[0].vert[2] = 2;

    mesh->initVertexNeighbors();
    mesh->updateTriangleNormals();
    mesh->updateVertexNormals();

    //edge information
    mesh->calcNeighborsVertices();
    mesh->calcEdges();
    mesh->upadateAABB();
    mesh->allocateAABBTris();

    std::shared_ptr<smMeshCollisionModel::AABBTreeType>
    modelAabbTree = std::make_shared<smMeshCollisionModel::AABBTreeType>(
    std::static_pointer_cast<smSurfaceMesh>(mesh),1);
    modelAabbTree->initStructure();

    model->setAABBTree(modelAabbTree);

    return model;
}

go_bandit([](){
    describe("BVH Collision Detection Algorithm", []() {
        it("constructs ", []() {
            std::unique_ptr<smCollisionDetection> meshToMeshCollision = make_unique<smMeshToMeshCollision>();
            AssertThat(meshToMeshCollision == nullptr, IsFalse());
        });
        it("performs collision detection ", []() {
            std::unique_ptr<smCollisionDetection> meshToMeshCollision = make_unique<smMeshToMeshCollision>();

            smStdVector3d verticesA;
            verticesA.emplace_back(1.0,2.0,0);
            verticesA.emplace_back(2.0,3.0,0);
            verticesA.emplace_back(2.0,1.0,0);

            smStdVector3d verticesB;
            verticesB.emplace_back(1.0,2.0,0.5);
            verticesB.emplace_back(2.0,3.0,0);
            verticesB.emplace_back(2.0,1.0,0);

            std::shared_ptr<smModelRepresentation> modelA = getModel(verticesA);

            std::shared_ptr<smModelRepresentation> modelB = getModel(verticesB);

            std::shared_ptr<smCollisionPair> collisionPair = std::make_shared<smCollisionPair>();

            collisionPair->setModels(modelA,modelB);

            meshToMeshCollision->computeCollision(collisionPair);

            AssertThat(collisionPair->hasContacts(), IsTrue());

        });

    });

});

