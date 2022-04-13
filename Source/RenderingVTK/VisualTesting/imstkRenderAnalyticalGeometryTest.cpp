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