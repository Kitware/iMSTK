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

#include "imstkAssimpMeshIO.h"
#include "imstkLineMesh.h"
#include "imstkSurfaceMesh.h"

#include <gtest/gtest.h>

using namespace imstk;

TEST(imstkAssimpMeshIOTest, ReadObjTriangle)
{
    std::shared_ptr<PointSet> mesh =
        AssimpMeshIO::readMeshData(iMSTK_DATA_ROOT "testing/MeshIO/triangle.obj");
    ASSERT_TRUE(mesh);

    // Assure we read a SurfaceMesh
    auto surfMesh = std::dynamic_pointer_cast<SurfaceMesh>(mesh);
    ASSERT_TRUE(surfMesh);
}

TEST(imstkAssimpMeshIOTest, ReadObjLine)
{
    std::shared_ptr<PointSet> mesh =
        AssimpMeshIO::readMeshData(iMSTK_DATA_ROOT "testing/MeshIO/line.obj");
    ASSERT_TRUE(mesh);

    // Assure we read a LineMesh
    auto lineMesh = std::dynamic_pointer_cast<LineMesh>(mesh);
    ASSERT_TRUE(lineMesh);
}