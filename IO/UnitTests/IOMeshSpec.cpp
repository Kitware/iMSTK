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
#include <memory>

// iMSTK includes
#include "Mesh/VegaVolumetricMesh.h"
#include "Mesh/SurfaceMesh.h"
#include "IO/IOMesh.h"
#include "IO/InitIO.h"
#include "Core/MakeUnique.h"
#include "Testing/ReadPaths.h"

auto paths = imstk::ReadPaths("./IOConfig.paths");

const std::string ds3MeshFileName = std::get<imstk::Path::Source>(paths)+"/cube.3ds";
const std::string objMeshFileName = std::get<imstk::Path::Source>(paths)+"/cube.obj";
const std::string vegMeshFileName = std::get<imstk::Path::Source>(paths)+"/SampleMesh.veg";
const std::string plyMeshFileName = std::get<imstk::Path::Source>(paths)+"/sphere.ply";
const std::string stlMeshFileName = std::get<imstk::Path::Source>(paths)+"/sphere.stl";
const std::string vtkMeshFileName = std::get<imstk::Path::Source>(paths)+"/sphere.vtk";
const std::string vtuMeshFileName = std::get<imstk::Path::Source>(paths)+"/sphere.vtu";
const std::string vtuPropFileName = std::get<imstk::Path::Binary>(paths)+"/TestMesh.vtu";
const std::string vtpMeshFileName = std::get<imstk::Path::Source>(paths)+"/sphere.vtp";

using namespace bandit;

go_bandit([](){

    imstk::InitIODelegates();
    describe("IO Mesh Reader", [&]() {
        auto ioMesh = imstk::make_unique<imstk::IOMesh>();

        it("constructs", [&]() {
            AssertThat(ioMesh != nullptr, IsTrue());
        });
        it("loads 3ds mesh", [&]() {
            ioMesh->read(ds3MeshFileName);
            AssertThat(ioMesh->getFileType() == imstk::IOMesh::MeshFileType::ThreeDS, IsTrue());
            auto mesh = std::dynamic_pointer_cast<imstk::SurfaceMesh>(ioMesh->getMesh());
            AssertThat(mesh != nullptr, IsTrue());
            AssertThat(mesh->getNumberOfVertices() > 0, IsTrue());
            AssertThat(mesh->getNumberOfTriangles() > 0, IsTrue());
        });
        it("loads obj mesh", [&]() {
            ioMesh->read(objMeshFileName);
            AssertThat(ioMesh->getFileType() == imstk::IOMesh::MeshFileType::OBJ, IsTrue());
            auto mesh = std::dynamic_pointer_cast<imstk::SurfaceMesh>(ioMesh->getMesh());
            AssertThat(mesh != nullptr, IsTrue());
            AssertThat(mesh->getNumberOfVertices() > 0, IsTrue());
            AssertThat(mesh->getNumberOfTriangles() > 0, IsTrue());
        });
        it("loads veg mesh", [&]() {
            ioMesh->read(vegMeshFileName);
            AssertThat(ioMesh->getFileType() == imstk::IOMesh::MeshFileType::VEG, IsTrue());
            auto mesh = std::dynamic_pointer_cast<imstk::VegaVolumetricMesh>(ioMesh->getMesh());
            AssertThat(mesh != nullptr, IsTrue());
            AssertThat(mesh->getNumberOfVertices() > 0, IsTrue());
            AssertThat(mesh->getNumberOfElements() > 0, IsTrue());

            auto surface = mesh->getAttachedMesh(0);
            AssertThat(surface->getNumberOfVertices() > 0, IsTrue());
            AssertThat(surface->getNumberOfTriangles() > 0, IsTrue());
        });
        it("loads ply mesh", [&]() {
            ioMesh->read(plyMeshFileName);
            AssertThat(ioMesh->getFileType() == imstk::IOMesh::MeshFileType::PLY, IsTrue());
            auto mesh = std::dynamic_pointer_cast<imstk::SurfaceMesh>(ioMesh->getMesh());
            AssertThat(mesh != nullptr, IsTrue());
            AssertThat(mesh->getNumberOfVertices() > 0, IsTrue());
            AssertThat(mesh->getNumberOfTriangles() > 0, IsTrue());
        });
        it("loads stl mesh", [&]() {
            ioMesh->read(stlMeshFileName);
            AssertThat(ioMesh->getFileType() == imstk::IOMesh::MeshFileType::STL, IsTrue());
            auto mesh = std::dynamic_pointer_cast<imstk::SurfaceMesh>(ioMesh->getMesh());
            AssertThat(mesh != nullptr, IsTrue());
            AssertThat(mesh->getNumberOfVertices() > 0, IsTrue());
            AssertThat(mesh->getNumberOfTriangles() > 0, IsTrue());
        });
        it("loads vtk mesh", [&]() {
            ioMesh->read(vtkMeshFileName);
            AssertThat(ioMesh->getFileType() == imstk::IOMesh::MeshFileType::VTK, IsTrue());
            auto mesh = std::dynamic_pointer_cast<imstk::VegaVolumetricMesh>(ioMesh->getMesh());
            AssertThat(mesh != nullptr, IsTrue());
            AssertThat(mesh->getNumberOfVertices() > 0, IsTrue());
            AssertThat(mesh->getNumberOfElements() > 0, IsTrue());

            auto surface = mesh->getAttachedMesh(0);
            AssertThat(surface->getNumberOfVertices() > 0, IsTrue());
            AssertThat(surface->getNumberOfTriangles() > 0, IsTrue());
        });
        it("loads vtu mesh", [&]() {
            ioMesh->read(vtuMeshFileName);
            AssertThat(ioMesh->getFileType() == imstk::IOMesh::MeshFileType::VTU, IsTrue());
            auto mesh = std::dynamic_pointer_cast<imstk::VegaVolumetricMesh>(ioMesh->getMesh());
            AssertThat(mesh != nullptr, IsTrue());
            AssertThat(mesh->getNumberOfVertices() > 0, IsTrue());
            AssertThat(mesh->getNumberOfElements() > 0, IsTrue());

            auto surface = mesh->getAttachedMesh(0);
            AssertThat(surface->getNumberOfVertices() > 0, IsTrue());
            AssertThat(surface->getNumberOfTriangles() > 0, IsTrue());
        });
        it("loads vtu with properties mesh", [&]() {
            ioMesh->read(vtuPropFileName);
            AssertThat(ioMesh->getFileType() == imstk::IOMesh::MeshFileType::VTU, IsTrue());
            auto mesh = std::dynamic_pointer_cast<imstk::VegaVolumetricMesh>(ioMesh->getMesh());
            AssertThat(mesh != nullptr, IsTrue());
            AssertThat(mesh->getNumberOfVertices() > 0, IsTrue());
            AssertThat(mesh->getNumberOfElements() > 0, IsTrue());

            auto surface = mesh->getAttachedMesh(0);
            AssertThat(surface->getNumberOfVertices() > 0, IsTrue());
            AssertThat(surface->getNumberOfTriangles() > 0, IsTrue());
        });
        it("loads vtp with properties mesh", [&]() {
            ioMesh->read(vtpMeshFileName);
            AssertThat(ioMesh->getFileType() == imstk::IOMesh::MeshFileType::VTP, IsTrue());
            auto mesh = std::dynamic_pointer_cast<imstk::SurfaceMesh>(ioMesh->getMesh());
            AssertThat(mesh != nullptr, IsTrue());
            AssertThat(mesh->getNumberOfVertices() > 0, IsTrue());
            AssertThat(mesh->getNumberOfTriangles() > 0, IsTrue());
        });

    });
});
