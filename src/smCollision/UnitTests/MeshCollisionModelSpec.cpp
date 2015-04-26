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

#include "smCollision/smMeshCollisionModel.h"
#include "smUtilities/smMakeUnique.h"
#include "smMesh/smSurfaceMesh.h"

using namespace bandit;

go_bandit([](){
    describe("Mesh collision model used by collision detection algorithms.", []() {
        it("construct ", []() {
            std::unique_ptr<smMeshCollisionModel> meshCollisionModel = make_unique<smMeshCollisionModel>();
        });
        it("loads mesh ", []() {
            std::unique_ptr<smMeshCollisionModel> meshCollisionModel = make_unique<smMeshCollisionModel>();

            std::shared_ptr<smMesh> mesh = std::make_shared<smSurfaceMesh>();

            meshCollisionModel->setMesh(mesh);

            AssertThat(meshCollisionModel->getMesh(), Equals( mesh));

        });
        it("can access positions ", []() {
            std::unique_ptr<smMeshCollisionModel> meshCollisionModel = make_unique<smMeshCollisionModel>();

            std::shared_ptr<smMesh> mesh = std::make_shared<smSurfaceMesh>();
            meshCollisionModel->setMesh(mesh);

            // Add two triangles to the data structure
            mesh->initVertexArrays(4);
            mesh->initTriangleArrays(2);

            smStdVector3f vertices;
            vertices.emplace_back(1.0,2.0,-1.0);
            vertices.emplace_back(2.0,3.0,1.0);
            vertices.emplace_back(2.0,1.0,-1.0);
            vertices.emplace_back(3.0,2.0,1.0);

            mesh->vertices.push_back(vertices[0]);
            mesh->vertices.push_back(vertices[1]);
            mesh->vertices.push_back(vertices[2]);
            mesh->vertices.push_back(vertices[3]);

            mesh->triangles[0].vert[0] = 0;
            mesh->triangles[0].vert[1] = 1;
            mesh->triangles[0].vert[2] = 2;

            mesh->triangles[1].vert[0] = 1;
            mesh->triangles[1].vert[1] = 2;
            mesh->triangles[1].vert[2] = 3;

            mesh->initVertexNeighbors();
            mesh->updateTriangleNormals();
            mesh->updateVertexNormals();

            //edge information
            mesh->calcNeighborsVertices();
            mesh->calcEdges();
            mesh->upadateAABB();
            mesh->allocateAABBTris();

            AssertThat(meshCollisionModel->getTrianglePositions(0)[0], Equals(vertices[0]));
            AssertThat(meshCollisionModel->getTrianglePositions(0)[1], Equals(vertices[1]));
            AssertThat(meshCollisionModel->getTrianglePositions(0)[2], Equals(vertices[2]));

            AssertThat(meshCollisionModel->getTrianglePositions(1)[0], Equals(vertices[1]));
            AssertThat(meshCollisionModel->getTrianglePositions(1)[1], Equals(vertices[2]));
            AssertThat(meshCollisionModel->getTrianglePositions(1)[2], Equals(vertices[3]));
        });
        it("can access normals ", []() {
            std::unique_ptr<smMeshCollisionModel> meshCollisionModel = make_unique<smMeshCollisionModel>();

            std::shared_ptr<smMesh> mesh = std::make_shared<smSurfaceMesh>();
            meshCollisionModel->setMesh(mesh);

            // Add two triangles to the data structure
            mesh->initVertexArrays(4);
            mesh->initTriangleArrays(2);

            smStdVector3f vertices;
            vertices.emplace_back(1.0,2.0,-1.0);
            vertices.emplace_back(2.0,3.0,1.0);
            vertices.emplace_back(2.0,1.0,-1.0);
            vertices.emplace_back(3.0,2.0,1.0);

            mesh->vertices.push_back(vertices[0]);
            mesh->vertices.push_back(vertices[1]);
            mesh->vertices.push_back(vertices[2]);
            mesh->vertices.push_back(vertices[3]);

            mesh->triangles[0].vert[0] = 0;
            mesh->triangles[0].vert[1] = 1;
            mesh->triangles[0].vert[2] = 2;

            mesh->triangles[1].vert[0] = 1;
            mesh->triangles[1].vert[1] = 2;
            mesh->triangles[1].vert[2] = 3;

            mesh->initVertexNeighbors();
            mesh->updateTriangleNormals();
            mesh->updateVertexNormals();

            //edge information
            mesh->calcNeighborsVertices();
            mesh->calcEdges();
            mesh->upadateAABB();
            mesh->allocateAABBTris();


            smVec3f normalA = (vertices[1]-vertices[0]).cross(vertices[2]-vertices[0]).normalized();
            smVec3f normalB = (vertices[2]-vertices[1]).cross(vertices[3]-vertices[1]).normalized();

            AssertThat((meshCollisionModel->getNormal(0)-normalA).squaredNorm(), EqualsWithDelta(0.0,.00001));
            AssertThat((meshCollisionModel->getNormal(1)-normalB).squaredNorm(), EqualsWithDelta(0.0,.00001));
        });
        it("create BVH ", []() {
            std::unique_ptr<smMeshCollisionModel> meshCollisionModel = make_unique<smMeshCollisionModel>();

            std::shared_ptr<smMesh> mesh = std::make_shared<smSurfaceMesh>();
            meshCollisionModel->setMesh(mesh);

            // Add two triangles to the data structure
            mesh->initVertexArrays(4);
            mesh->initTriangleArrays(2);

            smStdVector3f vertices;
            vertices.emplace_back(1.0,2.0,-1.0);
            vertices.emplace_back(2.0,3.0,1.0);
            vertices.emplace_back(2.0,1.0,-1.0);
            vertices.emplace_back(3.0,2.0,1.0);

            mesh->vertices.push_back(vertices[0]);
            mesh->vertices.push_back(vertices[1]);
            mesh->vertices.push_back(vertices[2]);
            mesh->vertices.push_back(vertices[3]);

            mesh->triangles[0].vert[0] = 0;
            mesh->triangles[0].vert[1] = 1;
            mesh->triangles[0].vert[2] = 2;

            mesh->triangles[1].vert[0] = 1;
            mesh->triangles[1].vert[1] = 2;
            mesh->triangles[1].vert[2] = 3;

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
                std::static_pointer_cast<smSurfaceMesh>(mesh),6);
            modelAabbTree->initStructure();

            meshCollisionModel->setAABBTree(modelAabbTree);

            AssertThat(meshCollisionModel->getAABBTree(), Equals(modelAabbTree));
        });

    });

});

