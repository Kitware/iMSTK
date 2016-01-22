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

#include <memory>

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkPoints.h>

// iMSTK includes
#include "../MeshNodalCoordinates.h"

#include <bandit/bandit.h>
using namespace bandit;

go_bandit([](){

    describe("VTK mapped array.", []() {
        it("constructs", []() {
            vtkNew<MeshNodalCoordinates<double>> meshMapper;
            AssertThat(meshMapper.GetPointer() != nullptr, IsTrue());
        });

        it("initializes", []() {
            vtkNew<MeshNodalCoordinates<double>> meshMapper;
            meshMapper->Initialize();
            AssertThat(meshMapper->GetMaxId() == -1, IsTrue());
            AssertThat(meshMapper->GetSize() == 0, IsTrue());
            AssertThat(meshMapper->GetNumberOfComponents() == 1, IsTrue());
        });

        it("wraps data", []() {
            vtkNew<MeshNodalCoordinates<double>> meshMapper;
            std::vector<core::Vec3d> vertices;
            vertices.emplace_back(0,1,0);
            vertices.emplace_back(1,0,0);
            vertices.emplace_back(0,0,1);
            vertices.emplace_back(0,1,1);

            meshMapper->SetVertexArray(vertices);

            AssertThat(meshMapper->GetMaxId() == 11, IsTrue());
            AssertThat(meshMapper->GetSize() == 12, IsTrue());
            AssertThat(meshMapper->GetNumberOfComponents() == 3, IsTrue());
        });

        it("is used by vtkPoints", []() {
            vtkNew<MeshNodalCoordinates<double>> meshMapper;
            std::vector<core::Vec3d> vertices;
            vertices.emplace_back(0,1,0);
            vertices.emplace_back(1,0,0);
            vertices.emplace_back(0,0,1);
            vertices.emplace_back(0,1,1);

            meshMapper->SetVertexArray(vertices);

            vtkNew<vtkPoints> points;
            points->SetData(meshMapper.GetPointer());

            AssertThat(points->GetNumberOfPoints() == 4, IsTrue());
            AssertThat(points->GetPoint(0)[1] == 1, IsTrue());
            AssertThat(points->GetPoint(1)[0] == 1, IsTrue());
            AssertThat(points->GetPoint(2)[2] == 1, IsTrue());
            AssertThat(points->GetPoint(3)[1] == 1, IsTrue());
        });
    });

});

