/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

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