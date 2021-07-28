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

#include "imstkCamera.h"
#include "imstkDirectionalLight.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneObject.h"
#include "imstkVisualModel.h"

// AnalyticalGeometry Classes
#include "imstkCapsule.h"
#include "imstkCylinder.h"
#include "imstkDecal.h"
#include "imstkOrientedBox.h"
#include "imstkPlane.h"
#include "imstkSphere.h"

class AnalyticalGeometryRenderTest : public RenderTest
{
public:
  void addGeometry(std::shared_ptr<AnalyticalGeometry> geom, Color color)
  {
    scene->getActiveCamera()->setPosition(Vec3d(0.0, 3.0, 3.0));
    auto light = std::make_shared<DirectionalLight>();
    light->setDirection(Vec3d(5.0, -8.0, -5.0));
    light->setIntensity(1.0);
    scene->addLight("light", light);

    geom->scale(0.5, Geometry::TransformType::ConcatenateToTransform);
    geom->rotate(Vec3d(0.0, 1.0, 0), PI_2, Geometry::TransformType::ConcatenateToTransform);
    geom->translate(Vec3d(0.0, 0.0, 0.0), Geometry::TransformType::ConcatenateToTransform);

    auto renderMaterial = std::make_shared<RenderMaterial>();
    renderMaterial->setColor(color);
    renderMaterial->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    renderMaterial->setPointSize(6.0);
    renderMaterial->setLineWidth(4.0);

    auto visualModel = std::make_shared<VisualModel>(geom);
    visualModel->setRenderMaterial(renderMaterial);

    auto sceneObj = std::make_shared<SceneObject>("SceneObject");
    sceneObj->addVisualModel(visualModel);
    scene->addSceneObject(sceneObj);
  }
};

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