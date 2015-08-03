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

#include "Collision/MeshToMeshCollision.h"
#include "Collision/MeshCollisionModel.h"
#include "Core/CollisionPair.h"
#include "Core/MakeUnique.h"
#include "Mesh/SurfaceMesh.h"

using namespace bandit;

std::shared_ptr<MeshCollisionModel> getModel(const std::vector<core::Vec3d> &vertices)
{
    std::shared_ptr<MeshCollisionModel> model = std::make_shared<MeshCollisionModel>();
    std::shared_ptr<Mesh> mesh = std::make_shared<SurfaceMesh>();

    // Add two triangles to the data structure
    mesh->initVertexArrays(3);
    mesh->initTriangleArrays(1);

    auto vertexArray = mesh->getVertices();

    vertexArray[0] = vertices[0];
    vertexArray[1] = vertices[1];
    vertexArray[2] = vertices[2];

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

    model->setMesh(mesh);

    return model;
}

go_bandit([](){
    describe("BVH Collision Detection Algorithm", []() {
        it("constructs ", []() {
            std::unique_ptr<CollisionDetection> meshToMeshCollision = make_unique<MeshToMeshCollision>();
            AssertThat(meshToMeshCollision == nullptr, IsFalse());
        });
        it("performs collision detection ", []() {
            std::unique_ptr<CollisionDetection> meshToMeshCollision = make_unique<MeshToMeshCollision>();

            std::vector<core::Vec3d> verticesA;
            verticesA.emplace_back(1.0,2.0,0);
            verticesA.emplace_back(2.0,3.0,0);
            verticesA.emplace_back(2.0,1.0,0);

            std::vector<core::Vec3d> verticesB;
            verticesB.emplace_back(1.0,2.0,0.5);
            verticesB.emplace_back(2.0,3.0,0);
            verticesB.emplace_back(2.0,1.0,0);

            std::shared_ptr<ModelRepresentation> modelA = getModel(verticesA);

            std::shared_ptr<ModelRepresentation> modelB = getModel(verticesB);

            std::shared_ptr<CollisionPair> collisionPair = std::make_shared<CollisionPair>();

            collisionPair->setModels(modelA,modelB);

            meshToMeshCollision->computeCollision(collisionPair);

            AssertThat(collisionPair->hasContacts(), IsTrue());

        });

    });

});

