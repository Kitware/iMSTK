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

#ifndef TESTING_TEST_MESH_H
#define TESTING_TEST_MESH_H

#include <memory>

#include "Mesh/SurfaceMesh.h"

namespace imstk {

std::shared_ptr<SurfaceMesh> makeSurfaceMesh()
{
    std::vector<imstk::Vec3d> vertices =
    {
        {-0.5,-0.5, -0.5},
        {-0.5, -0.5, 0.5},
        {-0.5, 0.5, -0.5},
        {-0.5, 0.5, 0.5},
        {0.5, -0.5, -0.5},
        {0.5, -0.5, 0.5},
        {0.5, 0.5, -0.5},
        {0.5, 0.5, 0.5},
    };

    std::vector<std::array<size_t,3>> triangles  =
    {
        {{3, 2, 1}},
        {{0, 1, 2}},
        {{0, 2, 4}},
        {{0, 4, 1}},
        {{1, 5, 3}},
        {{2, 3, 6}},
        {{4, 6, 5}},
        {{5, 1, 4}},
        {{6, 4, 2}},
        {{7, 3, 5}},
        {{3, 7, 6}},
        {{7, 5, 6}}
    };
    std::vector<std::array<double,2>> texCoord  =
    {
        {0.0,0.0},
        {0.0,1.0},
        {1.0,0.0},
        {1.0,1.0},
        {1.0,0.0},
        {1.0,1.0},
        {0.0,0.0},
        {0.0,1.0},
    };


    std::shared_ptr<SurfaceMesh> mesh = std::make_shared<SurfaceMesh>();
    mesh->getVertices() = vertices;
    mesh->setTriangles(triangles);
    for(size_t i = 0; i < texCoord.size(); ++i)
    {
        mesh->addTextureCoordinate(texCoord[i][0],texCoord[i][1]);
    }

    return mesh;
}

}

#endif
