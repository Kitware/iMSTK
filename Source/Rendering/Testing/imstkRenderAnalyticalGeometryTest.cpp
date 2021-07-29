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
#include "imstkDecal.h"
#include "imstkOrientedBox.h"
#include "imstkPlane.h"
#include "imstkSphere.h"

class CapsuleRenderTest : public RenderTest
{
  void createGeometry() override
  {
    geom = std::make_shared<Capsule>();
  }
};
TEST_F(CapsuleRenderTest, createCapsule)
{
    runAllMaterials();
}

class CylinderRenderTest : public RenderTest
{
  void createGeometry() override
  {
    geom = std::make_shared<Cylinder>();
  }
};
TEST_F(CylinderRenderTest, createCylinder)
{
  runAllMaterials();
}

class OrientedBoxRenderTest : public RenderTest
{
  void createGeometry() override
  {
    geom = std::make_shared<OrientedBox>();
  }
};
TEST_F(OrientedBoxRenderTest, createOrientedBox)
{
  runAllMaterials();
}

class PlaneRenderTest : public RenderTest
{
  void createGeometry() override
  {
    geom = std::make_shared<Plane>();
  }
};
TEST_F(PlaneRenderTest, createPlane)
{
  runAllMaterials();
}

class SphereRenderTest : public RenderTest
{
  void createGeometry() override
  {
    geom = std::make_shared<Sphere>();
  }
};
TEST_F(SphereRenderTest, createSphere)
{
  runAllMaterials();
}