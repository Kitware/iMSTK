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
getSphere()
{
    std::vector<double> buffVertices
    {
        0, 0, -0.5,
        0, 0.262865603, -0.425325394,
        0.249999985, 0.0812300518, -0.425325394,
        0, 0.44721368, -0.223606572,
        0.249999985, 0.344095677, -0.262865305,
        0.425325423, 0.138196841, -0.223606601,
        0.154508501, -0.212662712, -0.425325423,
        0.404508621, -0.131432697, -0.262865394,
        0.262865603, -0.361803472, -0.223606631,
        -0.154508501, -0.212662712, -0.425325423,
        0, -0.425325513, -0.262865365,
        -0.262865603, -0.361803472, -0.223606631,
        -0.249999985, 0.0812300518, -0.425325394,
        -0.404508621, -0.131432697, -0.262865394,
        -0.425325423, 0.138196841, -0.223606601,
        -0.249999985, 0.344095677, -0.262865305,
        -0.154508486, 0.4755283, 0,
        -0.404508412, 0.293892711, 0,
        -0.262865603, 0.361803472, 0.223606631,
        -0.5, 0, 0,
        -0.404508412, -0.293892711, 0,
        -0.425325423, -0.138196841, 0.223606601,
        -0.154508486, -0.4755283, 0,
        0.154508486, -0.4755283, 0,
        0, -0.44721368, 0.223606572,
        0.404508412, -0.293892711, 0,
        0.5, 0, 0,
        0.425325423, -0.138196841, 0.223606601,
        0.404508412, 0.293892711, 0,
        0.154508486, 0.4755283, 0,
        0.262865603, 0.361803472, 0.223606631,
        0, 0.425325513, 0.262865365,
        -0.404508621, 0.131432697, 0.262865394,
        -0.249999985, -0.344095677, 0.262865305,
        0.249999985, -0.344095677, 0.262865305,
        0.404508621, 0.131432697, 0.262865394,
        0, 0, 0.5,
        0.154508501, 0.212662712, 0.425325423,
        -0.154508501, 0.212662712, 0.425325423,
        0.249999985, -0.0812300518, 0.425325394,
        0, -0.262865603, 0.425325394,
        -0.249999985, -0.0812300518, 0.425325394
    };

    std::vector<size_t> buffFaces
    {
        3, 1, 2,
        5, 2, 4,
        6, 3, 5,
        3, 2, 5,
        7, 1, 3,
        8, 3, 6,
        9, 7, 8,
        7, 3, 8,
        10, 1, 7,
        11, 7, 9,
        12, 10, 11,
        10, 7, 11,
        13, 1, 10,
        14, 10, 12,
        15, 13, 14,
        13, 10, 14,
        2, 1, 13,
        16, 13, 15,
        4, 2, 16,
        2, 13, 16,
        17, 4, 16,
        18, 16, 15,
        19, 17, 18,
        17, 16, 18,
        20, 15, 14,
        21, 14, 12,
        22, 20, 21,
        20, 14, 21,
        23, 12, 11,
        24, 11, 9,
        25, 23, 24,
        23, 11, 24,
        26, 9, 8,
        27, 8, 6,
        28, 26, 27,
        26, 8, 27,
        29, 6, 5,
        30, 5, 4,
        31, 29, 30,
        29, 5, 30,
        30, 4, 17,
        32, 17, 19,
        31, 30, 32,
        30, 17, 32,
        18, 15, 20,
        33, 20, 22,
        19, 18, 33,
        18, 20, 33,
        21, 12, 23,
        34, 23, 25,
        22, 21, 34,
        21, 23, 34,
        24, 9, 26,
        35, 26, 28,
        25, 24, 35,
        24, 26, 35,
        27, 6, 29,
        36, 29, 31,
        28, 27, 36,
        27, 29, 36,
        39, 37, 38,
        32, 38, 31,
        19, 39, 32,
        39, 38, 32,
        38, 37, 40,
        36, 40, 28,
        31, 38, 36,
        38, 40, 36,
        40, 37, 41,
        35, 41, 25,
        28, 40, 35,
        40, 41, 35,
        41, 37, 42,
        34, 42, 22,
        25, 41, 34,
        41, 42, 34,
        42, 37, 39,
        33, 39, 19,
        22, 42, 33,
        42, 39, 33
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
