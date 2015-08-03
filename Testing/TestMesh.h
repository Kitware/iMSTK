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

#include <memory>

#include "Mesh/SurfaceMesh.h"

std::shared_ptr<Mesh> makeSurfaceMesh()
{
    std::vector<core::Vec3d> vertices =
    {
        {-0.5,-0.5, -0.5},
        {-0.5, -0.5, 0.5},
        {-0.5, 0.5, -0.5},
        {-0.5, 0.5, 0.5},
        {0.5, -0.5, -0.5},
        {0.5, -0.5, 0.5},
        {0.5, 0.5, -0.5},
        {0.5, 0.5, 0.5},
        {3.5527136788e-09,-1.7763568394e-09, 0}
    };

    std::vector<Triangle> triangles  =
    {
        {0, 1, 2},
        {0, 4, 1},
        {0, 2, 4},
        {3, 2, 1},
        {1, 5, 3},
        {5, 1, 4},
        {2, 3, 6},
        {6, 4, 2},
        {7, 3, 5},
        {7, 6, 3},
        {4, 6, 5},
        {7, 5, 6}
    };

    std::shared_ptr<SurfaceMesh> mesh = std::make_shared<SurfaceMesh>();
    mesh->getVertices() = vertices;
    mesh->triangles = triangles;

    return mesh;
}
