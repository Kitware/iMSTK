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

#include "Core/MakeUnique.h"
#include "Geometry/MeshModel.h"

using namespace bandit;

std::shared_ptr<MeshModel> getModel(const std::vector<core::Vec3d> &vertices)
{
    std::shared_ptr<SurfaceMesh> mesh = std::make_shared<SurfaceMesh>();

    // Add one triangle to the data structure
    mesh->setVertices(vertices);
    std::array<size_t,3> triangle = {0,1,2};

    mesh->getTriangles().emplace_back(triangle);

    mesh->computeVertexNeighbors();
    mesh->computeTriangleNormals();
    mesh->computeVertexNormals();

    std::shared_ptr<MeshModel> model = std::make_shared<MeshModel>();
    model->setModelMesh(mesh);
    return model;
}

go_bandit([](){
    describe("Mesh model", []() {
        it("constructs", []() {
            auto model = Core::make_unique<MeshModel>();
            AssertThat(model != nullptr, IsTrue());
        });
        it("can access mesh vertices", []() {

            std::vector<core::Vec3d> vertices;
            vertices.emplace_back(1.0,2.0,0);
            vertices.emplace_back(2.0,3.0,0);
            vertices.emplace_back(2.0,1.0,0);

            auto model = getModel(vertices);

            AssertThat(model->getVertices()[0], Equals(vertices[0]));
            AssertThat(model->getVertices()[1], Equals(vertices[1]));
            AssertThat(model->getVertices()[2], Equals(vertices[2]));

        });
        it("can access mesh face normals", []() {

            std::vector<core::Vec3d> vertices;
            vertices.emplace_back(1.0,2.0,0);
            vertices.emplace_back(2.0,3.0,0);
            vertices.emplace_back(2.0,1.0,0);

            auto model = getModel(vertices);

            core::Vec3d normalA = (vertices[1]-vertices[0]).cross(vertices[2]-vertices[0]).normalized();

            auto mesh = std::static_pointer_cast<SurfaceMesh>(model->getMesh());
            AssertThat((mesh->getTriangleNormal(0)-normalA).squaredNorm(), EqualsWithDelta(0.0,.00001));
        });

    });

});

