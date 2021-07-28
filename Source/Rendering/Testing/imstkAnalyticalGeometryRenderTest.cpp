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

#include "imstkSetupRenderTest.h"

#include "imstkCapsule.h"
#include "imstkCylinder.h"
#include "imstkDecal.h"
#include "imstkOrientedBox.h"
#include "imstkPlane.h"
#include "imstkSphere.h"

class AnalyticalGeometryRenderTest : public RenderTest { };

TEST_F(AnalyticalGeometryRenderTest, createCapsule)
{
    ASSERT_TRUE(scene != nullptr) << "ERROR: Unable to create scene object";

    auto geom = std::make_shared<Capsule>();

    addGeometry(geom, Color::Pink);

    run_for(driver.get(), 2);
}

TEST_F(AnalyticalGeometryRenderTest, createCylinder)
{
  ASSERT_TRUE(scene != nullptr) << "ERROR: Unable to create scene object";

  auto geom = std::make_shared<Cylinder>();

  addGeometry(geom, Color::Blue);

  run_for(driver.get(), 2);
}

TEST_F(AnalyticalGeometryRenderTest, createOrientedBox)
{
  ASSERT_TRUE(scene != nullptr) << "ERROR: Unable to create scene object";

  auto geom = std::make_shared<OrientedBox>();

  addGeometry(geom, Color::Teal);

  run_for(driver.get(), 2);
}

TEST_F(AnalyticalGeometryRenderTest, createPlane)
{
  ASSERT_TRUE(scene != nullptr) << "ERROR: Unable to create scene object";

  auto geom = std::make_shared<Plane>();

  addGeometry(geom, Color::Orange);

  run_for(driver.get(), 2);
}

TEST_F(AnalyticalGeometryRenderTest, createSphere)
{
  ASSERT_TRUE(scene != nullptr) << "ERROR: Unable to create scene object";

  auto geom = std::make_shared<Sphere>();

  addGeometry(geom, Color::Red);

  run_for(driver.get(), 2);
}