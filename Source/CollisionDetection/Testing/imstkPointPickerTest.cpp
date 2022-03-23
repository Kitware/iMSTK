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

#include "gtest/gtest.h"

#include "imstkPointPicker.h"
#include "imstkSphere.h"
#include "imstkPlane.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkCapsule.h"
#include "imstkOrientedBox.h"
#include "imstkVecDataArray.h"

using namespace imstk;

///
/// \brief PickImplicit tests the implicit function of a capsule as the capsule
/// does not yet have an analytical ray intersection solution
///
TEST(imstkPointerPickerTest, PickImplicit)
{
    auto capsule = std::make_shared<Capsule>(Vec3d::Zero(), 0.5, 1.0, Quatd::Identity());

    // Ray to left (-x) of capsule, pointing in
    PointPicker picker;
    picker.setPickingRay(Vec3d(-1.0, 0.0, 0.0), Vec3d(1.0, 0.0, 0.0));
    picker.setUseFirstHit(false);
    const std::vector<PickData>& pickData1 = picker.pick(capsule);
    EXPECT_EQ(pickData1.size(), 1);
    EXPECT_TRUE(pickData1[0].pickPoint.isApprox(Vec3d(-0.5, 0.0, 0.0), 0.01)) <<
        "Pick Point: " << pickData1[0].pickPoint.transpose();

    // Ray pointing out
    picker.setPickingRay(Vec3d(-1.0, 0.0, 0.0), Vec3d(-1.0, -1.0, 0.0));
    const std::vector<PickData>& pickData2 = picker.pick(capsule);
    EXPECT_EQ(pickData2.size(), 0);
}

TEST(imstkPointerPickerTest, PickOrientedBox)
{
    auto obb = std::make_shared<OrientedBox>(Vec3d::Zero(), Vec3d(0.5, 0.5, 0.5), Quatd::Identity());

    // Ray to the left of the box pointing in to it
    PointPicker picker;
    picker.setPickingRay(Vec3d(-1.0, 0.0, 0.0), Vec3d(1.0, 0.0, 0.0));
    picker.setUseFirstHit(false);
    const std::vector<PickData>& pickData1 = picker.pick(obb);
    EXPECT_EQ(pickData1.size(), 2);
    EXPECT_TRUE(pickData1[0].pickPoint.isApprox(Vec3d(-0.5, 0.0, 0.0))) <<
        "Pick Point: " << pickData1[0].pickPoint.transpose();
    EXPECT_TRUE(pickData1[1].pickPoint.isApprox(Vec3d(0.5, 0.0, 0.0))) <<
        "Pick Point: " << pickData1[1].pickPoint.transpose();

    // Ray same direction but above the box missing it
    picker.setPickingRay(Vec3d(-1.0, 1.0, 0.0), Vec3d(1.0, 0.0, 0.0));
    const std::vector<PickData>& pickData2 = picker.pick(obb);
    EXPECT_EQ(pickData2.size(), 0);
}

TEST(imstkPointerPickerTest, PickPlane)
{
    auto plane = std::make_shared<Plane>(Vec3d::Zero(), Vec3d(0.0, 1.0, 0.0));

    // Ray below the plane pointing up
    PointPicker picker;
    picker.setPickingRay(Vec3d(0.0, -1.0, 0.0), Vec3d(0.0, 1.0, 0.0));
    picker.setUseFirstHit(false);
    const std::vector<PickData>& pickData1 = picker.pick(plane);
    EXPECT_EQ(pickData1.size(), 1);
    EXPECT_TRUE(pickData1[0].pickPoint.isApprox(Vec3d(0.0, 0.0, 0.0))) <<
        "Pick Point: " << pickData1[0].pickPoint.transpose();

    // Coplanar case
    picker.setPickingRay(Vec3d(0.0, -1.0, 0.0), Vec3d(1.0, 0.0, 0.0));
    const std::vector<PickData>& pickData2 = picker.pick(plane);
    EXPECT_EQ(pickData2.size(), 0);

    // Pointing away from plane
    picker.setPickingRay(Vec3d(0.0, -1.0, 0.0), Vec3d(0.0, -1.0, -1.0));
    const std::vector<PickData>& pickData3 = picker.pick(plane);
    EXPECT_EQ(pickData3.size(), 0);
}

TEST(imstkPointerPickerTest, PickSphere)
{
    auto sphere = std::make_shared<Sphere>(Vec3d::Zero(), 0.5);

    // Ray to the left of the sphere pointing in to it
    PointPicker picker;
    picker.setPickingRay(Vec3d(-1.0, 0.0, 0.0), Vec3d(1.0, 0.0, 0.0));
    picker.setUseFirstHit(false);
    const std::vector<PickData>& pickData1 = picker.pick(sphere);
    EXPECT_EQ(pickData1.size(), 1);
    EXPECT_TRUE(pickData1[0].pickPoint.isApprox(Vec3d(-0.5, 0.0, 0.0))) <<
        "Pick Point: " << pickData1[0].pickPoint.transpose();

    // Ray same direction but above the sphere missing it
    picker.setPickingRay(Vec3d(-1.0, 1.0, 0.0), Vec3d(1.0, 0.0, 0.0));
    const std::vector<PickData>& pickData2 = picker.pick(sphere);
    EXPECT_EQ(pickData2.size(), 0);
}

TEST(imstkPointerPickerTest, PickSurfaceMesh)
{
    auto surfMesh = std::make_shared<SurfaceMesh>();
    {
        auto verticesPtr = std::make_shared<VecDataArray<double, 3>>(3);
        (*verticesPtr)[0] = Vec3d(0.5, 0.0, -0.5);
        (*verticesPtr)[1] = Vec3d(-0.5, 0.0, -0.5);
        (*verticesPtr)[2] = Vec3d(0.0, 0.0, 0.5);
        auto indicesPtr = std::make_shared<VecDataArray<int, 3>>(1);
        (*indicesPtr)[0] = Vec3i(0, 1, 2);
        surfMesh->initialize(verticesPtr, indicesPtr);
    }

    // Ray above triangle, pointing directly down
    PointPicker picker;
    picker.setPickingRay(Vec3d(0.0, 1.0, 0.0), Vec3d(0.0, -1.0, 0.0));
    picker.setUseFirstHit(false);
    const std::vector<PickData>& pickData1 = picker.pick(surfMesh);
    EXPECT_EQ(pickData1.size(), 1);
    EXPECT_TRUE(pickData1[0].pickPoint.isApprox(Vec3d(0.0, 0.0, 0.0))) <<
        "Pick Point: " << pickData1[0].pickPoint.transpose();

    // Ray pointing away from triangle
    picker.setPickingRay(Vec3d(0.0, 1.0, 0.0), Vec3d(1.0, 0.0, 0.0));
    const std::vector<PickData>& pickData2 = picker.pick(surfMesh);
    EXPECT_EQ(pickData2.size(), 0);

    // Ray pointing toward triangle plane but not on triangle
    picker.setPickingRay(Vec3d(1.0, 1.0, 0.0), Vec3d(0.0, -1.0, 0.0));
    const std::vector<PickData>& pickData3 = picker.pick(surfMesh);
    EXPECT_EQ(pickData3.size(), 0);
}

TEST(imstkPointerPickerTest, PickTetrahedralMesh)
{
    auto tetMesh = std::make_shared<TetrahedralMesh>();
    {
        // We use a regular tetrahedron with edge lengths 2
        // V = (edge length)^3/(6sqrt(2))
        const double edgeLenth = 2.0;

        auto                     verticesPtr = std::make_shared<VecDataArray<double, 3>>(4);
        auto                     indicesPtr  = std::make_shared<VecDataArray<int, 4>>(1);
        VecDataArray<double, 3>& vertices    = *verticesPtr;
        VecDataArray<int, 4>&    indices     = *indicesPtr;

        vertices[0] = Vec3d(1.0, 0.0, -1.0 / std::sqrt(2.0));
        vertices[1] = Vec3d(-1.0, 0.0, -1.0 / std::sqrt(2.0));
        vertices[2] = Vec3d(0.0, 1.0, 1.0 / std::sqrt(2.0));
        vertices[3] = Vec3d(0.0, -1.0, 1.0 / std::sqrt(2.0));

        indices[0] = Vec4i(0, 1, 2, 3);
        tetMesh->initialize(verticesPtr, indicesPtr);
    }

    // Ray above tet, pointing directly down
    PointPicker picker;
    picker.setPickingRay(Vec3d(0.0, 1.0, 0.0), Vec3d(0.0, -1.0, 0.0));
    picker.setUseFirstHit(false);
    const std::vector<PickData>& pickData1 = picker.pick(tetMesh);
    EXPECT_EQ(pickData1.size(), 2);

    // Ray pointing away from tet
    picker.setPickingRay(Vec3d(0.0, 2.0, 0.0), Vec3d(1.0, 0.0, 0.0));
    const std::vector<PickData>& pickData2 = picker.pick(tetMesh);
    EXPECT_EQ(pickData2.size(), 0);
}

///
/// \brief Tests code path for culling all other hits
///
TEST(imstkPointerPickerTest, PickFirstHit)
{
    auto obb = std::make_shared<OrientedBox>(Vec3d::Zero(), Vec3d(0.5, 0.5, 0.5), Quatd::Identity());

    // Ray to the left of the box pointing in to it
    PointPicker picker;
    picker.setPickingRay(Vec3d(-1.0, 0.0, 0.0), Vec3d(1.0, 0.0, 0.0));
    picker.setUseFirstHit(true);
    const std::vector<PickData>& pickData1 = picker.pick(obb);
    EXPECT_EQ(pickData1.size(), 1);
}