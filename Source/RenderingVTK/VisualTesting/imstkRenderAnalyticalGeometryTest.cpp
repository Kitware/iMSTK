/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkRenderTest.h"

#include "imstkCapsule.h"
#include "imstkCylinder.h"
#include "imstkOrientedBox.h"
#include "imstkPlane.h"
#include "imstkSphere.h"

TEST_F(RenderTest, Capsule)
{
    geom = std::make_shared<Capsule>();
    createScene();
    runAllMaterials();
}

TEST_F(RenderTest, Cylinder)
{
    geom = std::make_shared<Cylinder>();
    createScene();
    runAllMaterials();
}

TEST_F(RenderTest, OrientedBox)
{
    geom = std::make_shared<OrientedBox>();
    createScene();
    runAllMaterials();
}

TEST_F(RenderTest, Plane)
{
    geom = std::make_shared<Plane>();
    createScene();
    runAllMaterials();
}

TEST_F(RenderTest, Sphere)
{
    geom = std::make_shared<Sphere>();
    createScene();
    runAllMaterials();
}