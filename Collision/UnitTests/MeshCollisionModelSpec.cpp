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

#include <bandit/bandit.h>

#include "Collision/MeshCollisionModel.h"
#include "Core/MakeUnique.h"
#include "Mesh/SurfaceMesh.h"
#include "Collision/SurfaceTree.h"

using namespace bandit;

go_bandit([](){
    describe("the mesh collision model used by collision detection algorithms.", []() {
        it("constructs", []() {
            auto meshCollisionModel = imstk::make_unique<imstk::MeshCollisionModel>();
            AssertThat(meshCollisionModel != nullptr, IsTrue());
        });
        it("loads the mesh", []() {
            auto meshCollisionModel = imstk::make_unique<imstk::MeshCollisionModel>();
            auto mesh = std::make_shared<imstk::SurfaceMesh>();

            meshCollisionModel->setModelMesh(mesh);

            AssertThat(meshCollisionModel->getMesh(), Equals( mesh));

        });
        it("can access positions", []() {
            auto meshCollisionModel = std::make_shared<imstk::MeshCollisionModel>();

            auto mesh = std::make_shared<imstk::SurfaceMesh>();

            std::vector<imstk::Vec3d> vertices;
            vertices.emplace_back(1.0,2.0,-1.0);
            vertices.emplace_back(2.0,3.0,1.0);
            vertices.emplace_back(2.0,1.0,-1.0);
            vertices.emplace_back(3.0,2.0,1.0);

            mesh->setVertices(vertices);

            std::array<size_t,3> t0 = {0,1,2};
            std::array<size_t,3> t1 = {0,2,3};
            mesh->getTriangles().emplace_back(t0);
            mesh->getTriangles().emplace_back(t1);

            mesh->computeVertexNeighbors();
            mesh->computeTriangleNormals();
            mesh->computeVertexNormals();

            meshCollisionModel->setMesh(mesh);
            auto p0 = meshCollisionModel->getElementPositions(0);
            AssertThat(p0[0], Equals(vertices[0]));
            AssertThat(p0[1], Equals(vertices[1]));
            AssertThat(p0[2], Equals(vertices[2]));

            auto p1 = meshCollisionModel->getElementPositions(1);
            AssertThat(p1[0], Equals(vertices[0]));
            AssertThat(p1[1], Equals(vertices[2]));
            AssertThat(p1[2], Equals(vertices[3]));
        });
        it("can access normals", []() {
            auto meshCollisionModel = std::make_shared<imstk::MeshCollisionModel>();

            auto mesh = std::make_shared<imstk::SurfaceMesh>();

            auto &vertices = mesh->getVertices();
            vertices.emplace_back(1.0,2.0,-1.0);
            vertices.emplace_back(2.0,3.0,1.0);
            vertices.emplace_back(2.0,1.0,-1.0);
            vertices.emplace_back(3.0,2.0,1.0);

            std::array<size_t,3> t0 = {0,1,2};
            std::array<size_t,3> t1 = {1,2,3};
            mesh->getTriangles().emplace_back(t0);
            mesh->getTriangles().emplace_back(t1);

            mesh->computeVertexNeighbors();
            mesh->computeTriangleNormals();
            mesh->computeVertexNormals();

            auto normalA = (vertices[1]-vertices[0]).cross(vertices[2]-vertices[0]).normalized();
            auto normalB = (vertices[2]-vertices[1]).cross(vertices[3]-vertices[1]).normalized();

            meshCollisionModel->setMesh(mesh);
            AssertThat((meshCollisionModel->getSurfaceNormal(0)-normalA).squaredNorm(), EqualsWithDelta(0.0,.00001));
            AssertThat((meshCollisionModel->getSurfaceNormal(1)-normalB).squaredNorm(), EqualsWithDelta(0.0,.00001));
        });
        it("can create BVH", []() {
            auto meshCollisionModel = std::make_shared<imstk::MeshCollisionModel>();

            auto mesh = std::make_shared<imstk::SurfaceMesh>();

            // Add two triangles to the data structure
            auto &vertices = mesh->getVertices();
            vertices.emplace_back(1.0,2.0,-1.0);
            vertices.emplace_back(2.0,3.0,1.0);
            vertices.emplace_back(2.0,1.0,-1.0);
            vertices.emplace_back(3.0,2.0,1.0);

            std::array<size_t,3> t0 = {0,1,2};
            std::array<size_t,3> t1 = {1,2,3};
            mesh->getTriangles().emplace_back(t0);
            mesh->getTriangles().emplace_back(t1);

            mesh->computeVertexNeighbors();
            mesh->computeTriangleNormals();
            mesh->computeVertexNormals();

            meshCollisionModel->setMesh(mesh);
            meshCollisionModel->computeBoundingBoxes();

            auto modelAabbTree = std::make_shared<imstk::MeshCollisionModel::AABBTreeType>(
                    meshCollisionModel,6);
            modelAabbTree->initStructure();

            meshCollisionModel->setAABBTree(modelAabbTree);
            AssertThat(meshCollisionModel->getAABBTree(), Equals(modelAabbTree));
        });

    });

});

