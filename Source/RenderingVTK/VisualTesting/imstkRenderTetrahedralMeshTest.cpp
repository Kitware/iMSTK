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

#include "imstkRenderTest.h"
#include "imstkTetrahedralMesh.h"
#include "imstkVecDataArray.h"

TEST_F(RenderTest, createTetrahedralMesh)
{
    auto tetMesh = std::make_shared<TetrahedralMesh>();
    geom = tetMesh;

    VecDataArray<double, 3> vertices(4);
    vertices[0] = Vec3d(-0.5, 0.0, -0.5);
    vertices[1] = Vec3d(0.5, 0.0, -0.5);
    vertices[2] = Vec3d(0.0, 0.0, 0.75);
    vertices[3] = Vec3d(0.0, 0.5, 0.0);

    VecDataArray<int, 4> indices(1);
    indices[0] = Vec4i(0, 1, 2, 3);

    tetMesh->initialize(std::make_shared<VecDataArray<double, 3>>(vertices),
        std::make_shared<VecDataArray<int, 4>>(indices));

    createScene();
    runAllMaterials();
}