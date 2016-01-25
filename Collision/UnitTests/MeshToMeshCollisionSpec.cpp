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

#include "Collision/MeshToMeshCollision.h"
#include "Collision/MeshCollisionModel.h"
#include "Core/CollisionManager.h"
#include "Core/MakeUnique.h"
#include "Mesh/SurfaceMesh.h"

using namespace bandit;

std::shared_ptr<imstk::MeshModel> getModel(const std::vector<imstk::Vec3d> &vertices)
{
    auto mesh = std::make_shared<imstk::SurfaceMesh>();

    // Add one triangle to the data structure
    mesh->setVertices(vertices);

    std::array<size_t,3> t = {0,1,2};
    mesh->getTriangles().emplace_back(t);

    mesh->computeVertexNeighbors();
    mesh->computeTriangleNormals();
    mesh->computeVertexNormals();

    auto model = std::make_shared<imstk::MeshCollisionModel>();
    model->setMesh(mesh);
    return model;
}

go_bandit([](){
    describe("BVH Collision Detection Algorithm", []() {
        it("constructs ", []() {
            auto meshToMeshCollision = imstk::make_unique<imstk::MeshToMeshCollision>();
            AssertThat(meshToMeshCollision == nullptr, IsFalse());
        });
        it("performs collision detection ", []() {
            auto meshToMeshCollision = imstk::make_unique<imstk::MeshToMeshCollision>();

            std::vector<imstk::Vec3d> verticesA;
            verticesA.emplace_back(1.0,2.0,0);
            verticesA.emplace_back(2.0,3.0,0);
            verticesA.emplace_back(2.0,1.0,0);

            std::vector<imstk::Vec3d> verticesB;
            verticesB.emplace_back(1.0,2.0,0.5);
            verticesB.emplace_back(2.0,3.0,0);
            verticesB.emplace_back(2.0,1.0,0);

            auto modelA = getModel(verticesA);

            auto modelB = getModel(verticesB);

            auto collisionPair = std::make_shared<imstk::CollisionManager>();

            collisionPair->setModels(modelA,modelB);

            meshToMeshCollision->computeCollision(collisionPair);

            AssertThat(collisionPair->hasContacts(), IsTrue());

        });

    });

});

