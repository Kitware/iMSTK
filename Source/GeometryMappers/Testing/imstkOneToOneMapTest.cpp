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

#include "imstkGeometry.h"
#include "imstkOneToOneMap.h"
#include "imstkPointSet.h"
#include "imstkSphere.h"
#include "imstkSurfaceMesh.h"
#include "imstkVecDataArray.h"

#include <gtest/gtest.h>

using namespace imstk;

namespace
{
std::shared_ptr<VecDataArray<double, 3>>
getCubePoints()
{
    // Setup a cube
    //    0-------1
    //   /|      /|
    //  / |     / |
    // 3--|----2  |
    // |  4----|--5    +y +z
    // | /     | /     | /
    // 7-------6       |/__+x
    auto verticesPtr = std::make_shared<VecDataArray<double, 3>>(8);

    VecDataArray<double, 3>& vertices = *verticesPtr;

    vertices[0] = Vec3d(-0.5, 0.5, 0.5);
    vertices[1] = Vec3d(0.5, 0.5, 0.5);
    vertices[2] = Vec3d(0.5, 0.5, -0.5);
    vertices[3] = Vec3d(-0.5, 0.5, -0.5);
    vertices[4] = Vec3d(-0.5, -0.5, 0.5);
    vertices[5] = Vec3d(0.5, -0.5, 0.5);
    vertices[6] = Vec3d(0.5, -0.5, -0.5);
    vertices[7] = Vec3d(-0.5, -0.5, -0.5);

    return verticesPtr;
}
} // namespace

TEST(imstkOneToOneMapTest, SimpleMap)
{
    auto parent = std::make_shared<PointSet>();
    parent->initialize(getCubePoints());

    auto child = std::make_shared<PointSet>();
    child->initialize(getCubePoints());

    OneToOneMap map;
    map.setParentGeometry(parent);
    map.setChildGeometry(child);
    map.compute();

    for (int i = 0; i < child->getNumVertices(); ++i)
    {
        EXPECT_EQ(map.getParentVertexId(i), i);
    }

    parent->translate({ 1.0, 2.0, 3.0 }, Geometry::TransformType::ApplyToData);
    map.update();
    for (int i = 0; i < child->getNumVertices(); ++i)
    {
        EXPECT_TRUE(parent->getVertexPosition(i).isApprox(child->getVertexPosition(i)));
    }
}

TEST(imstkOneToOneMapTest, DeathTests)
{
    auto parent = std::make_shared<PointSet>();
    auto child  = std::make_shared<PointSet>();

    auto sphere = std::make_shared<Sphere>();
    {
        OneToOneMap map;
        ASSERT_DEATH(map.compute(), "without valid geometries");
    }
    {
        OneToOneMap map;
        map.setParentGeometry(parent);
        ASSERT_DEATH(map.compute(), "without valid geometries");
    }
    {
        OneToOneMap map;
        map.setChildGeometry(child);
        ASSERT_DEATH(map.compute(), "without valid geometries");
    }
    {
        OneToOneMap map;
        ASSERT_DEATH(map.setParentGeometry(sphere), "The geometry provided is not a PointSet!");
    }
    {
        OneToOneMap map;
        ASSERT_DEATH(map.setChildGeometry(sphere), "The geometry provided is not a PointSet!");
    }
}

TEST(imstkOneToOneMapTest, OneToManyMap)
{
    auto parent = std::make_shared<PointSet>();
    parent->initialize(getCubePoints());

    auto child  = std::make_shared<PointSet>();
    auto points = getCubePoints();
    points->push_back(Vec3d(0.5, 0.5, -0.5));
    points->push_back(Vec3d(0.5, 0.5, -0.5) * 10);
    child->initialize(points);

    OneToOneMap map;
    map.setParentGeometry(parent);
    map.setChildGeometry(child);
    map.setTolerance(1e-8);
    map.compute();

    for (int i = 0; i < parent->getNumVertices(); ++i)
    {
        EXPECT_EQ(map.getParentVertexId(i), i);
    }

    EXPECT_EQ(map.getParentVertexId(8), 2);

    EXPECT_EQ(map.getParentVertexId(9), -1);

    parent->translate({ 1.0, 2.0, 3.0 }, Geometry::TransformType::ApplyToData);
    map.update();
    for (int i = 0; i < child->getNumVertices() - 1; ++i)
    {
        auto j = map.getParentVertexId(i);
        EXPECT_TRUE(child->getVertexPosition(i).isApprox(parent->getVertexPosition(j))) << "For " << i << ", " << j;
    }
}
