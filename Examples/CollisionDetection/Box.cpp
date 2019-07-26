/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License",
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

=========================================================================*/

#include "imstkMath.h"

#include <array>
using namespace imstk;

std::pair<StdVectorOfVec3d, std::vector<std::array<size_t, 3>>>
getBox()
{
    std::vector<double> buffVertices
    {
        0.353553385, -0.707106769, -0.353553414,
        -0.353553414, -0.707106769, 0.353553385,
        0.853553414, 0, 0.146446615,
        0.146446615, 0, 0.853553414,
        -0.853553414, 0, -0.146446615,
        -0.146446615, 0, -0.853553414,
        -0.353553385, 0.707106769, 0.353553414,
        0.353553414, 0.707106769, -0.353553385
    };

    std::vector<size_t> buffFaces
    {
        1, 4, 2,
        1, 3, 4,
        5, 8, 6,
        5, 7, 8,
        7, 3, 8,
        7, 4, 3,
        6, 2, 5,
        6, 1, 2,
        6, 3, 1,
        6, 8, 3,
        2, 7, 5,
        2, 4, 7
    };

    StdVectorOfVec3d vertices;
    vertices.reserve(buffVertices.size() / 3);
    for (size_t i = 0; i < buffVertices.size() / 3; ++i)
    {
        vertices.emplace_back(Vec3d(buffVertices[i * 3],
                                    buffVertices[i * 3 + 1],
                                    buffVertices[i * 3 + 2]));
    }

    std::vector<std::array<size_t, 3>> faces;
    faces.reserve(buffFaces.size() / 3);
    for (size_t i = 0; i < buffFaces.size() / 3; ++i)
    {
        // Face ID of triangles is 0-based index (data from .obj file is 1-based index)
        std::array<size_t, 3> tmp;
        tmp[0] = buffFaces[i * 3] - 1;
        tmp[1] = buffFaces[i * 3 + 1] - 1;
        tmp[2] = buffFaces[i * 3 + 2] - 1;
        faces.push_back(std::move(tmp));
    }

    return { vertices, faces };
}
