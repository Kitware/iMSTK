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

#include "imstkCamera.h"

#include <gtest/gtest.h>

using namespace imstk;

TEST(imstkCameraTest, camera_defaults)
{
    Camera cam;

    // Ensure the camera is initialized to identity
    EXPECT_EQ(cam.getView(), Mat4d::Identity())
        << "Expected: " << Mat4d::Identity()
        << " Actual: " << cam.getView();
    EXPECT_EQ(cam.getInvView(), Mat4d::Identity())
        << "Expected: " << Mat4d::Identity()
        << " Actual: " << cam.getInvView();

    cam.update();

    // Ensure camera is initialized to identity even after applying
    // lookat
    EXPECT_EQ(cam.getView(), Mat4d::Identity())
        << "Expected: " << Mat4d::Identity()
        << " Actual: " << cam.getView();
    EXPECT_EQ(cam.getInvView(), Mat4d::Identity())
        << "Expected: " << Mat4d::Identity()
        << " Actual: " << cam.getInvView();
}