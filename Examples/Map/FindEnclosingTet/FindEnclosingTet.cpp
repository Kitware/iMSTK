/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

=========================================================================*/
#include <cmath>
#include <cstdlib>
#include <memory>
#include <iostream>

#include "imstkAPIUtilities.h"
#include "imstkTetraTriangleMap.h"
#include "imstkTetrahedralMesh.h"
#include "imstkVolumetricMesh.h"

using namespace imstk;

int main()
{

    StdVectorOfVec3d coords(4);
    coords[0] = {0.0, 0.0, 0.0};
    coords[1] = {0.0, 1.0, 0.0};
    coords[2] = {1.0, 0.0, 0.0};
    coords[3] = {0.0, 0.0, 1.0};

    std::vector<TetrahedralMesh::TetraArray> vertices = {{0, 1, 2, 3}};

    auto tetMesh = std::make_shared<TetrahedralMesh>();
    tetMesh->initialize(coords, vertices);

    auto map = std::make_shared<TetraTriangleMap>();
    map->setMaster(std::move(tetMesh));
    map->updateBoundingBox();

    Vec3d pnt = {0.1, 0.1, 0.1};
    auto tetId = map->findEnclosingTetrahedron(pnt);
    if (tetId != 0)
    {
        return 1;
    }

    pnt = {-0.1, 0.0, 0.0};
    tetId = map->findEnclosingTetrahedron(pnt);
    if (tetId == 0)
    {
        return 1;
    }

    // std::cout << "Passed" << std::endl;
    return 0;
}
